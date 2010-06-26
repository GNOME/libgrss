/*
 * Copyright (C) 2010, Roberto Guido <rguido@src.gnome.org>
 *                     Michele Tameni <michele@amdplanet.it>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#include "feeds-publisher.h"
#include "utils.h"
#include "feed-marshal.h"

#define VERIFICATION_INTERVAL_MINUTES	30
#define DEFAULT_LEASE_INTERVAL		(60 * 60)
#define DEFAULT_SERVER_PORT   		80
#define DEFAULT_REFRESH_CHECK_INTERVAL	60

#define FEEDS_PUBLISHER_GET_PRIVATE(obj)	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), FEEDS_PUBLISHER_TYPE, FeedsPublisherPrivate))

/**
 * SECTION: feeds-publisher
 * @short_description: feed writer and PubSubHubbub publisher
 *
 * #FeedsPublisher may be used to expose contents for any given #FeedChannel,
 * both writing a file to be dispatched by the local webserver or providing
 * himself to distribute it, and implements a server able to receive
 * subscriptions by PubSubHubbub clients and deliver them new contents in
 * real-time.
 */

static void	subscribe_verify_cb	(SoupSession *session, SoupMessage *msg, gpointer user_data);
static void	verify_delivery_cb	(SoupSession *session, SoupMessage *msg, gpointer user_data);

struct _FeedsPublisherPrivate {
	gboolean		running;

	int			port;
	SoupServer		*server;
	GInetAddress		*local_addr;

	SoupSession		*soupsession;

	time_t			current_time;
	GHashTable		*topics;
	guint			refresh_handler;
};

typedef struct {
	FeedChannel		*channel;
	GList			*subscribers;
	GList			*items_delivered;
	guint			resend_handler;
} ValidTopic;

typedef enum {
	REMOTE_SUBSCRIBING,
	REMOTE_UNSUBSCRIBING,
} SUBSCRIBER_STATUS;

typedef struct {
	FeedsPublisher		*parent;
	SUBSCRIBER_STATUS	status;
	gchar			*topic;
	ValidTopic		*topic_struct;
	gchar			*callback;
	gchar			*challenge;
	gint64			lease_interval;
	time_t			first_contact_time;
	time_t			registration_time;
	SoupMessage		*registration_msg;
	gchar			*to_be_resent;
} RemoteSubscriber;

enum {
	SUBSCRIPTION_ADDED,
	SUBSCRIPTION_DELETED,
	LAST_SIGNAL
};

static guint signals [LAST_SIGNAL] = {0};

G_DEFINE_TYPE (FeedsPublisher, feeds_publisher, G_TYPE_OBJECT);

static void
destroy_remote_subscriber (RemoteSubscriber *client)
{
	FREE_STRING (client->topic);
	FREE_STRING (client->callback);
	FREE_STRING (client->challenge);
	FREE_OBJECT (client->registration_msg);
	g_free (client);
}

static void
destroy_topic (gpointer user_data)
{
	GList *iter;
	ValidTopic *topic;

	topic = user_data;
	g_object_unref (topic->channel);

	if (topic->subscribers != NULL) {
		for (iter = topic->subscribers; iter; iter = iter->next)
			destroy_remote_subscriber (iter->data);

		g_list_free (topic->subscribers);
	}

	if (topic->items_delivered != NULL) {
		for (iter = topic->items_delivered; iter; iter = iter->next)
			g_object_unref (iter->data);

		g_list_free (topic->items_delivered);
	}

	g_free (topic);
}

static gboolean
topic_remove_helper (gpointer key, gpointer value, gpointer user_data)
{
	return TRUE;
}

static void
remove_current_topics (FeedsPublisher *pub)
{
	g_hash_table_foreach_remove (pub->priv->topics, topic_remove_helper, NULL);
}

static void
feeds_publisher_finalize (GObject *obj)
{
	FeedsPublisher *pub;

	pub = FEEDS_PUBLISHER (obj);
	feeds_publisher_hub_switch (pub, FALSE);

	remove_current_topics (pub);
	g_hash_table_destroy (pub->priv->topics);
}

static void
feeds_publisher_class_init (FeedsPublisherClass *klass)
{
	GObjectClass *gobject_class;

	g_type_class_add_private (klass, sizeof (FeedsPublisherPrivate));

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->finalize = feeds_publisher_finalize;

	/**
	 * FeedsSubscriber::new_subscription:
	 * @pub: the #FeedsPublisher emitting the signal
	 * @topic: #FeedChannel for which subscription has been added
	 * @callback: callback required for new subscriber
	 *
	 * Emitted when a new remote client subscribes to this publisher
	 */
	signals [SUBSCRIPTION_ADDED] = g_signal_new ("new-subscription", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, 0,
	                                             NULL, NULL, feed_marshal_VOID__OBJECT_STRING,
	                                             G_TYPE_NONE, 2, FEED_CHANNEL_TYPE, G_TYPE_STRING);

	/**
	 * FeedsSubscriber::new_subscription:
	 * @pub: the #FeedsPublisher emitting the signal
	 * @topic: #FeedChannel for which subscription has been removed
	 * @callback: callback revoked by the subscriber
	 *
	 * Emitted when a new remote client unsubscribes to this publisher
	 */
	signals [SUBSCRIPTION_DELETED] = g_signal_new ("delete-subscription", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, 0,
	                                               NULL, NULL, feed_marshal_VOID__OBJECT_STRING,
	                                               G_TYPE_NONE, 2, FEED_CHANNEL_TYPE, G_TYPE_STRING);
}

static void
feeds_publisher_init (FeedsPublisher *node)
{
	node->priv = FEEDS_PUBLISHER_GET_PRIVATE (node);
	memset (node->priv, 0, sizeof (FeedsPublisherPrivate));
	node->priv->port = DEFAULT_SERVER_PORT;
	node->priv->topics = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, destroy_topic);
}

/**
 * feeds_publisher_new:
 *
 * Allocates a new #FeedsPublisher
 *
 * Return value: a new #FeedsPublisher
 */
FeedsPublisher*
feeds_publisher_new ()
{
	return g_object_new (FEEDS_PUBLISHER_TYPE, NULL);
}

static gchar*
format_feed_text (FeedsPublisher *pub, FeedChannel *channel, GList *items)
{
	const gchar *str;
	gchar *formatted;
	time_t date;
	GList *iter;
	const GList *list;
	GString *text;
	FeedItem *item;

	text = g_string_new ("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<feed xmlns=\"http://www.w3.org/2005/Atom\">\n");

	str = feed_channel_get_title (channel);
	if (str != NULL)
		g_string_append_printf (text, "<title>%s</title>\n", str);

	str = feed_channel_get_description (channel);
	if (str != NULL)
		g_string_append_printf (text, "<subtitle>%s</subtitle>\n", str);

	str = feed_channel_get_homepage (channel);
	if (str != NULL)
		g_string_append_printf (text, "<link href=\"%s\" />\n", str);

	str = feed_channel_get_copyright (channel);
	if (str != NULL)
		g_string_append_printf (text, "<author>%s</author>\n", str);

	str = feed_channel_get_editor (channel);
	if (str != NULL)
		g_string_append_printf (text, "<rights>%s</rights>\n", str);

	str = feed_channel_get_generator (channel);
	if (str != NULL)
		g_string_append_printf (text, "<generator>%s</generator>\n", str);

	list = feed_channel_get_contributors (channel);
	while (list != NULL) {
		g_string_append_printf (text, "<contributor>%s</contributor>\n", (gchar*) list->data);
		list = list->next;
	}

	date = feed_channel_get_update_time (channel);
	formatted = date_to_ISO8601 (date);
	g_string_append_printf (text, "<updated>%s</updated>\n", formatted);
	g_free (formatted);

	str = feed_channel_get_icon (channel);
	if (str != NULL)
		g_string_append_printf (text, "<icon>%s</icon>\n", str);

	str = feed_channel_get_image (channel);
	if (str != NULL)
		g_string_append_printf (text, "<logo>%s</logo>\n", str);

	for (iter = items; iter; iter = iter->next) {
		item = iter->data;

		g_string_append (text, "\t<entry>\n");

		str = feed_item_get_title (item);
		if (str != NULL)
			g_string_append_printf (text, "\t<title>%s</title>\n", str);

		str = feed_item_get_id (item);
		if (str != NULL)
			g_string_append_printf (text, "\t<id>%s</id>\n", str);

		str = feed_item_get_source (item);
		if (str != NULL)
			g_string_append_printf (text, "<link href=\"%s\" />\n", str);

		str = feed_item_get_description (item);
		if (str != NULL)
			g_string_append_printf (text, "\t<summary>%s</summary>\n", str);

		str = feed_item_get_author (item);
		if (str != NULL)
			g_string_append_printf (text, "\t<author>%s</author>\n", str);

		str = feed_item_get_copyright (item);
		if (str != NULL)
			g_string_append_printf (text, "\t<rights>%s</rights>\n", str);

		list = feed_item_get_contributors (item);
		while (list != NULL) {
			g_string_append_printf (text, "\t<contributor>%s</contributor>\n", (gchar*) list->data);
			list = list->next;
		}

		date = feed_item_get_publish_time (item);
		formatted = date_to_ISO8601 (date);
		g_string_append_printf (text, "<published>%s</published>\n", formatted);
		g_free (formatted);

		g_string_append (text, "\t</entry>\n");
	}

	g_string_append (text, "</feed>");
	return g_string_free (text, FALSE);
}

static gboolean
resend_deliver_to_subscribers (gpointer user_data)
{
	int found;
	GList *iter;
	SoupMessage *msg;
	RemoteSubscriber *client;
	ValidTopic *topic;

	topic = user_data;
	found = 0;

	for (iter = topic->subscribers; iter; iter = iter->next) {
		client = iter->data;

		if (client->to_be_resent != NULL) {
			found++;

			msg = soup_message_new ("POST", client->callback);
			soup_message_set_request (msg, "application/x-www-form-urlencoded", SOUP_MEMORY_STATIC, client->to_be_resent, strlen (client->to_be_resent));
			soup_session_queue_message (client->parent->priv->soupsession, msg, verify_delivery_cb, client);
		}
	}

	if (found == 0) {
		topic->resend_handler = -1;
		return FALSE;
	}
	else {
		return TRUE;
	}
}

static void
verify_delivery_cb (SoupSession *session, SoupMessage *msg, gpointer user_data)
{
	guint status;
	RemoteSubscriber *client;

	client = user_data;
	g_object_get (msg, "status-code", &status, NULL);

	if (status < 200 || status > 299) {
		if (client->to_be_resent == NULL)
			client->to_be_resent = g_strdup (msg->request_body->data);

		if (client->topic_struct->resend_handler == -1)
			client->topic_struct->resend_handler = g_timeout_add_seconds (60 * VERIFICATION_INTERVAL_MINUTES,
										      resend_deliver_to_subscribers, client->topic_struct);
	}
	else {
		if (client->to_be_resent != NULL) {
			g_free (client->to_be_resent);
			client->to_be_resent = NULL;
		}
	}
}

static void
deliver_to_subscribers (FeedsPublisher *pub, FeedChannel *channel, GList *items)
{
	gboolean found;
	gchar *text;
	const gchar *ilink;
	const gchar *olink;
	GList *iiter;
	GList *oiter;
	GList *to_deliver;
	SoupMessage *msg;
	RemoteSubscriber *client;
	ValidTopic *topic;

	topic = g_hash_table_lookup (pub->priv->topics, feed_channel_get_source (channel));
	if (topic == NULL || topic->subscribers == NULL)
		return;

	to_deliver = NULL;

	for (oiter = items; oiter; oiter = oiter->next) {
		olink = feed_item_get_source (oiter->data);
		if (olink == NULL)
			continue;

		found = FALSE;

		for (iiter = topic->items_delivered; iiter; iiter = iiter->next) {
			ilink = feed_item_get_source (iiter->data);

			if (strcmp (ilink, olink) == 0) {
				found = TRUE;
				break;
			}
		}

		if (found == FALSE)
			to_deliver = g_list_prepend (to_deliver, oiter->data);
	}

	if (to_deliver != NULL) {
		if (topic->resend_handler != -1) {
			g_source_remove (topic->resend_handler);
			topic->resend_handler = -1;
		}

		to_deliver = g_list_reverse (to_deliver);
		text = format_feed_text (pub, channel, to_deliver);

		for (oiter = topic->subscribers; oiter; oiter = oiter->next) {
			client = oiter->data;

			if (client->to_be_resent != NULL) {
				g_free (client->to_be_resent);
				client->to_be_resent = NULL;
			}

			msg = soup_message_new ("POST", client->callback);
			soup_message_set_request (msg, "application/x-www-form-urlencoded", SOUP_MEMORY_TAKE, g_strdup (text), strlen (text));
			soup_session_queue_message (client->parent->priv->soupsession, msg, verify_delivery_cb, client);
		}

		topic->items_delivered = g_list_concat (to_deliver, topic->items_delivered);
	}
}

static void
feed_required_by_web_cb (SoupServer *server, SoupMessage *msg, const char *path,
                         GHashTable *query, SoupClientContext *context, gpointer user_data)
{
	gchar *uri;
	gchar *text;
	gint64 size;
	gsize read;
	GError *error;
	GFileInfo *info;
	GFileInputStream *stream;

	error = NULL;
	stream = g_file_read (user_data, NULL, &error);

	if (stream == NULL) {
		uri = g_file_get_uri (user_data);
		g_warning ("Unable to open required feed in %s: %s.", uri, error->message);
		g_free (uri);
		g_error_free (error);
		soup_message_set_status (msg, 404);
		return;
	}

	info = g_file_input_stream_query_info (stream, G_FILE_ATTRIBUTE_STANDARD_SIZE, NULL, NULL);
	size = g_file_info_get_attribute_uint64 (info, G_FILE_ATTRIBUTE_STANDARD_SIZE);
	text = g_new0 (gchar, size);
	g_input_stream_read_all (G_INPUT_STREAM (stream), text, size, &read, NULL, NULL);

	soup_message_set_response (msg, "application/atom+xml", SOUP_MEMORY_TAKE, text, read);
	soup_message_set_status (msg, 200);

	g_object_unref (info);
	g_object_unref (stream);
}

/**
 * feeds_publisher_publish:
 * @pub: a #FeedsPublisher
 * @channel: the #FeedChannel to dump in the file
 * @items: list of #FeedItems to be added in the feed
 * @id: name used in the external URL of the feed
 *
 * If the local web server has been executed (with
 * feeds_publisher_hub_switch()) this function exposes the given @channel as
 * an Atom formatted file avalable to http://[LOCAL_IP:DEFINED_PORT]/@id
 */
void
feeds_publisher_publish (FeedsPublisher *pub, FeedChannel *channel, GList *items, const gchar *id)
{
	gchar *path;
	GFile *file;

	if (pub->priv->server == NULL) {
		g_warning ("Local web server is not running, unable to expose required contents");
		return;
	}

	soup_server_remove_handler (pub->priv->server, id);

	path = g_strdup_printf ("file://%s/libgrss/activefeeds/%s", g_get_tmp_dir (), id);

	/*
		PubSubHubbub notifies are already delivered by feeds_publisher_publish_file()
	*/
	feeds_publisher_publish_file (pub, channel, items, (const gchar*) path);

	file = g_file_new_for_uri (path);
	soup_server_add_handler (pub->priv->server, id, feed_required_by_web_cb, file, g_object_unref);

	g_free (path);
}

/**
 * feeds_publisher_publish_file:
 * @pub: a #FeedsPublisher
 * @channel: the #FeedChannel to dump in the file
 * @items: list of #FeedItems to be added in the feed
 * @path: path of the file to write
 *
 * Dump the given @channel in an Atom formatted file in @path. If the local
 * PubSubHubbub hub has been activated (with feeds_publisher_hub_switch())
 * notifies remote subscribers about the new items which has been added since
 * previous invocation of this function for the same #FeedChannel
 */
void
feeds_publisher_publish_file (FeedsPublisher *pub, FeedChannel *channel, GList *items, const gchar *uri)
{
	gchar *text;
	GFile *file;
	GFileOutputStream *stream;

	file = g_file_new_for_uri (uri);
	text = format_feed_text (pub, channel, items);
	stream = g_file_append_to (file, G_FILE_CREATE_NONE, NULL, NULL);
	g_output_stream_write_all (G_OUTPUT_STREAM (stream), text, strlen (text), NULL, NULL, NULL);

	if (pub->priv->server != NULL)
		deliver_to_subscribers (pub, channel, items);

	g_free (text);
	g_object_unref (stream);
	g_object_unref (file);
}

static void
add_client_to_topic (FeedsPublisher *pub, RemoteSubscriber *client)
{
	ValidTopic *topic;

	topic = g_hash_table_lookup (pub->priv->topics, client->topic);
	if (topic != NULL) {
		topic->subscribers = g_list_prepend (topic->subscribers, client);
		client->registration_time = time (NULL);

		if (client->registration_msg != NULL) {
			g_object_unref (client->registration_msg);
			client->registration_msg = NULL;
		}

		g_signal_emit (pub, signals [SUBSCRIPTION_ADDED], 0, topic->channel, client->callback);
	}
}

static void
remove_client_to_topic (FeedsPublisher *pub, RemoteSubscriber *client)
{
	GList *iter;
	ValidTopic *topic;

	topic = g_hash_table_lookup (pub->priv->topics, client->topic);
	if (topic != NULL) {
		for (iter = topic->subscribers; iter; iter = iter->next) {
			if (iter->data == client) {
				topic->subscribers = g_list_delete_link (topic->subscribers, iter);
				g_signal_emit (pub, signals [SUBSCRIPTION_DELETED], 0, topic->channel, client->callback);
				destroy_remote_subscriber (client);
			}
		}
	}
}

static gchar*
random_string ()
{
	register int i;
	gchar str [50];
	gchar *chars = "qwertyuiopasdfghjklzxcvbnm1234567890QWERTYUIOPASDFGHJKLZXCVBNM";

	srand (time (NULL));

	for (i = 0; i < 49; i++)
		str [i] = chars [rand () % 62];
	str [i] = '\0';

	return g_strdup (str);
}

static SoupMessage*
verification_message_for_client (RemoteSubscriber *client)
{
	gchar *mode;
	gchar *body;
	SoupMessage *ret;

	FREE_STRING (client->challenge);
	client->challenge = random_string ();

	switch (client->status) {
		case REMOTE_SUBSCRIBING:
			mode = "subscribe";
			break;

		case REMOTE_UNSUBSCRIBING:
			mode = "unsubscribe";
			break;

		default:
			return NULL;
			break;
	}

	body = g_strdup_printf ("%s?hub.mode=%s&hub.topic=%s&hub.challenge=%s&hub.lease_seconds=%ld",
				client->callback, mode, client->topic, client->challenge, client->lease_interval);

	ret = soup_message_new ("GET", body);
	g_free (body);

	return ret;
}

static gboolean
retry_subscriber_verification (gpointer user_data)
{
	SoupMessage *msg;
	RemoteSubscriber *client;

	client = user_data;
	msg = verification_message_for_client (client);
	soup_session_queue_message (client->parent->priv->soupsession, msg, subscribe_verify_cb, client);
	return FALSE;
}

static void
verification_failed (RemoteSubscriber *client)
{
	if (client->registration_msg != NULL) {
		soup_message_set_status (client->registration_msg, 404);
		soup_server_unpause_message (client->parent->priv->server, client->registration_msg);
		destroy_remote_subscriber (client);
	}
	else {
		/*
			If the verification fails for 6 hours, the procedure is dropped.
			Otherwise it is re-scheduled each VERIFICATION_INTERVAL_MINUTES
			minutes
			Cfr. PubSubHubbub Core 0.3 Section 6.2.1
		*/
		if (time (NULL) - client->first_contact_time > (60 * 60 * 6)) {
			if (client->status == REMOTE_SUBSCRIBING)
				destroy_remote_subscriber (client);
		}
		else {
			g_timeout_add_seconds (60 * VERIFICATION_INTERVAL_MINUTES, retry_subscriber_verification, client);
		}
	}
}

static void
subscribe_verify_cb (SoupSession *session, SoupMessage *msg, gpointer user_data)
{
	guint status;
	RemoteSubscriber *client;

	client = user_data;

	g_object_get (msg, "status-code", &status, NULL);

	if (status < 200 || status > 299) {
		verification_failed (client);
	}
	else {
		if (msg->response_body->data == NULL || strcmp (msg->response_body->data, client->challenge) != 0) {
			verification_failed (client);
		}
		else {
			if (client->registration_msg != NULL) {
				soup_message_set_status (client->registration_msg, 204);
				soup_server_unpause_message (client->parent->priv->server, client->registration_msg);
			}

			if (client->status == REMOTE_SUBSCRIBING)
				add_client_to_topic (client->parent, client);
			else
				remove_client_to_topic (client->parent, client);
		}
	}
}

static RemoteSubscriber*
search_subscriber_by_topic_and_callback (FeedsPublisher *pub, gchar *topic, gchar *callback)
{
	GList *iter;
	ValidTopic *topic_test;
	RemoteSubscriber *client_test;
	RemoteSubscriber *ret;

	ret = NULL;

	topic_test = g_hash_table_lookup (pub->priv->topics, topic);
	if (topic_test != NULL) {
		for (iter = topic_test->subscribers; iter; iter = iter->next) {
			client_test = iter->data;
			if (strcmp (callback, client_test->callback) == 0) {
				ret = iter->data;
				break;
			}
		}
	}

	return ret;
}

static void
handle_incoming_requests_cb (SoupServer *server, SoupMessage *msg, const char *path,
                             GHashTable *query, SoupClientContext *context, gpointer user_data)
{
	int i;
	int len;
	gchar **contents;
	gchar *mode;
	gchar *topic;
	gchar *lease;
	gchar *callback;
	gchar *verify;
	SoupMessage *verify_msg;
	FeedsPublisher *pub;
	RemoteSubscriber *client;
	ValidTopic *topic_struct;

	pub = user_data;

	contents = g_strsplit (msg->request_body->data, "hub.", -1);
	if (contents == NULL) {
		soup_message_set_status (msg, 404);
		return;
	}

	client = NULL;
	mode = NULL;
	topic = NULL;
	lease = NULL;
	callback = NULL;
	verify = NULL;

	for (i = 0; contents [i] != NULL; i++) {
		len = strlen (contents [i]) - 1;
		if (contents [i] [len] == '&')
			contents [i] [len] = '\0';

		if (strncmp (contents [i], "mode", 4) == 0)
			mode = contents [i] + 5;
		else if (strncmp (contents [i], "topic", 5) == 0)
			topic = contents [i] + 6;
		else if (strncmp (contents [i], "lease_seconds", 13) == 0)
			lease = contents [i] + 14;
		else if (strncmp (contents [i], "callback", 8) == 0)
			callback = contents [i] + 9;
		else if (strncmp (contents [i], "verify", 6) == 0)
			verify = contents [i] + 7;
	}

	if (mode == NULL) {
		soup_message_set_status (msg, 404);
	}
	else {
		if (strcmp (mode, "subscribe") == 0) {
			topic_struct = g_hash_table_lookup (pub->priv->topics, topic);

			if (topic_struct == NULL) {
				soup_message_set_status (msg, 404);
			}
			else {
				client = g_new0 (RemoteSubscriber, 1);
				client->parent = pub;
				client->first_contact_time = time (NULL);
				client->topic = g_strdup (topic);
				client->topic_struct = topic_struct;
				client->callback = g_strdup (callback);

				if (lease != NULL)
					client->lease_interval = strtoll (lease, NULL, 10);
				else
					client->lease_interval = DEFAULT_LEASE_INTERVAL;

				client->status = REMOTE_SUBSCRIBING;
			}
		}
		else if (strcmp (mode, "unsubscribe") == 0) {
			client = search_subscriber_by_topic_and_callback (pub, topic, callback);
			if (client != NULL)
				client->status = REMOTE_UNSUBSCRIBING;
		}

		if (client != NULL) {
			verify_msg = verification_message_for_client (client);

			if (strcmp (verify, "sync") == 0) {
				g_object_ref (msg);
				client->registration_msg = msg;
				soup_server_pause_message (server, msg);
				soup_session_queue_message (pub->priv->soupsession, verify_msg, subscribe_verify_cb, client);
			}
			else {
				soup_session_queue_message (pub->priv->soupsession, verify_msg, subscribe_verify_cb, client);
				soup_message_set_status (msg, 202);
			}
		}
	}

	g_strfreev (contents);
}

static void
close_server (FeedsPublisher *pub)
{
	if (pub->priv->server != NULL) {
		soup_server_remove_handler (pub->priv->server, NULL);
		soup_server_quit (pub->priv->server);
		g_object_unref (pub->priv->server);
		pub->priv->server = NULL;
	}
}

static void
create_and_run_server (FeedsPublisher *pub)
{
	SoupAddress *soup_addr;

	if (pub->priv->soupsession == NULL)
		pub->priv->soupsession = soup_session_async_new ();

	soup_addr = soup_address_new_any (SOUP_ADDRESS_FAMILY_IPV4, pub->priv->port);
	pub->priv->server = soup_server_new ("port", pub->priv->port, "interface", soup_addr, NULL);
	g_object_unref (soup_addr);

	soup_server_add_handler (pub->priv->server, NULL, handle_incoming_requests_cb, pub, NULL);
	soup_server_run_async (pub->priv->server);
}

/**
 * feeds_publisher_hub_set_port:
 * @pub: a #FeedsPublisher
 * @port: new listening port for the server
 *
 * To customize the port opened by the local server to deliver feeds and
 * catch incoming subscriptions. By default this is 80. Changing the port
 * while the hub is running imply restart the local server
 */
void
feeds_publisher_hub_set_port (FeedsPublisher *pub, int port)
{
	if (port != pub->priv->port) {
		pub->priv->port = port;

		if (pub->priv->running == TRUE) {
			feeds_publisher_hub_switch (pub, FALSE);
			feeds_publisher_hub_switch (pub, TRUE);
		}
	}
}

/**
 * feeds_publisher_hub_set_topics:
 * @pub: a #FeedsPublisher
 * @topics: a list of #FeedChannels
 *
 * To define a list of valid "topics" for which the #FeedsPublisher will
 * deliver contents. Sources of those channels, as retrieved by
 * feed_channel_get_source(), are accepted as "hub.topic" parameter in
 * PubSubHubbub registration requests from remote subscribers.
 * Pay attention to the fact subscriptions requests for different topic are
 * now rejected
 */
void
feeds_publisher_hub_set_topics (FeedsPublisher *pub, GList *topics)
{
	GList *iter;
	ValidTopic *topic;

	remove_current_topics (pub);

	for (iter = topics; iter; iter = iter->next) {
		topic = g_new0 (ValidTopic, 1);
		topic->channel = g_object_ref (iter->data);
		topic->resend_handler = -1;
		g_hash_table_insert (pub->priv->topics, (gpointer) feed_channel_get_source (iter->data), topic);
	}
}

static void
refresh_subscribers_in_topic (gpointer key, gpointer value, gpointer user_data)
{
	GList *iter;
	SoupMessage *verify_msg;
	FeedsPublisher *pub;
	ValidTopic *topic;
	RemoteSubscriber *client;

	pub = user_data;
	topic = value;

	for (iter = topic->subscribers; iter; iter = iter->next) {
		client = iter->data;

		if (client->registration_time + client->lease_interval + DEFAULT_REFRESH_CHECK_INTERVAL >= pub->priv->current_time) {
			verify_msg = verification_message_for_client (client);
			soup_session_queue_message (pub->priv->soupsession, verify_msg, subscribe_verify_cb, client);
		}
	}
}

static gboolean
refresh_subscribers (gpointer user_data)
{
	FeedsPublisher *pub;

	pub = user_data;
	pub->priv->current_time = time (NULL);
	g_hash_table_foreach (pub->priv->topics, refresh_subscribers_in_topic, pub);

	return TRUE;
}

static void
install_refresh_handler (FeedsPublisher *pub)
{
	pub->priv->refresh_handler = g_timeout_add_seconds (DEFAULT_REFRESH_CHECK_INTERVAL, refresh_subscribers, pub);
}

static void
remove_refresh_handler (FeedsPublisher *pub)
{
	g_source_remove (pub->priv->refresh_handler);
}

/**
 * feeds_publisher_hub_switch:
 * @pub: a #FeedsPublisher
 * @run: TRUE to run the local server, FALSE to stop it
 *
 * Permits to start and stop the webserver implemented by this object
 */
void
feeds_publisher_hub_switch (FeedsPublisher *pub, gboolean run)
{
	if (pub->priv->running != run) {
		pub->priv->running = run;

		if (run == TRUE) {
			create_and_run_server (pub);
			install_refresh_handler (pub);
		}
		else {
			remove_refresh_handler (pub);
			close_server (pub);
		}
	}
}
