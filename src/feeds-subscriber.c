/*
 * Copyright (C) 2009, Roberto Guido <rguido@src.gnome.org>
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

#include "feeds-subscriber.h"
#include "utils.h"
#include "feed-parser.h"
#include "feed-marshal.h"

#define DEFAULT_SERVER_PORT   8444

#define FEEDS_SUBSCRIBER_GET_PRIVATE(obj)	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), FEEDS_SUBSCRIBER_TYPE, FeedsSubscriberPrivate))

/**
 * SECTION: feeds-subscriber
 * @short_description: PubSubHubbub subscriber
 *
 * #FeedsSubscriber is an alternative for #FeedsPool, able to receive
 * real-time notifications by feeds managed by a PubSubHubbub hub.
 * When the subscriber is executed (with feeds_subscriber_switch()) it opens
 * a server on a local port (configurable with feeds_subscriber_set_port()),
 * engage a subscription for each #FeedChannel passed with
 * feeds_subscriber_listen(), and waits for direct notifications by the
 * remote hub.
 * For more information look at http://code.google.com/p/pubsubhubbub/
 */

/*
	TODO	There were an error in refreshing subscriptions, since is the
		hub which must send refresh requests, not subscriber.
		That has been removed, but it would be better to provide a
		control mechanism able to refresh subscriptions from the
		client side in case of problems by the server side
*/

typedef enum {
	SUBSCRIBER_IS_IDLE,
	SUBSCRIBER_TRYING_LOCAL_IP,
	SUBSCRIBER_CHECKING_PUBLIC_IP
} SUBSCRIBER_INIT_STATUS;

static void	subscribe_feeds			(FeedsSubscriber *sub);
static void	try_another_subscription_policy	(FeedsSubscriber *sub);

typedef void (*SubscriberJobCallback) (FeedsSubscriber *subscriber);

struct _FeedsSubscriberPrivate {
	gboolean		running;

	SUBSCRIBER_INIT_STATUS	initing_status;
	gboolean		has_errors_in_subscription;
	int			port;
	SoupServer		*server;
	GInetAddress		*local_addr;
	GInetAddress		*external_addr;

	gchar			*hub;
	SoupSession		*soupsession;

	FeedParser		*parser;
	GList			*feeds_list;

	guint			refresh_scheduler;
};

typedef enum {
	FEED_SUBSCRIPTION_IDLE,
	FEED_SUBSCRIPTION_SUBSCRIBING,
	FEED_SUBSCRIPTION_SUBSCRIBED,
	FEED_SUBSCRIPTION_UNSUBSCRIBING,
} FEED_SUBSCRIPTION_STATUS;

typedef struct {
	FeedChannel			*channel;

	FEED_SUBSCRIPTION_STATUS	status;
	int				identifier;
	gchar				*path;

	FeedsSubscriber			*sub;
} FeedChannelWrap;

typedef struct {
	int			counter;
	SubscriberJobCallback	callback;
	FeedsSubscriber		*subscriber;
} SubscriberJob;

enum {
	NOTIFICATION_RECEIVED,
	LAST_SIGNAL
};

static guint signals [LAST_SIGNAL] = {0};

G_DEFINE_TYPE (FeedsSubscriber, feeds_subscriber, G_TYPE_OBJECT);

static void
remove_currently_listened (FeedsSubscriber *sub)
{
	GList *iter;
	FeedChannelWrap *wrap;

	if (sub->priv->feeds_list != NULL) {
		for (iter = sub->priv->feeds_list; iter; iter = g_list_next (iter)) {
			wrap = (FeedChannelWrap*) iter->data;
			g_free (wrap->path);
			g_object_unref (wrap->channel);
			g_free (wrap);
		}

		g_list_free (sub->priv->feeds_list);
	}
}

static void
notification_handled_cb (FeedsSubscriber *sub, FeedChannel *feed, FeedItem *item)
{
	g_object_unref (item);
}

static void
feeds_subscriber_finalize (GObject *obj)
{
	FeedsSubscriber *sub;

	sub = FEEDS_SUBSCRIBER (obj);
	feeds_subscriber_switch (sub, FALSE);
	remove_currently_listened (sub);
	FREE_STRING (sub->priv->hub);
	g_object_unref (sub->priv->parser);
}

static void
feeds_subscriber_class_init (FeedsSubscriberClass *klass)
{
	GObjectClass *gobject_class;

	g_type_class_add_private (klass, sizeof (FeedsSubscriberPrivate));

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->finalize = feeds_subscriber_finalize;

	klass->notification_received = notification_handled_cb;

	/**
	 * FeedsSubscriber::notification-received:
	 * @pool: the #FeedsSubscriber emitting the signal
	 * @feed: the #FeedChannel which has been updated
	 * @item: the #FeedItem received
	 *
	 * Emitted when a notification has been received and parsed. The
	 * @item is unref'd at the end of signal
	 */
	signals [NOTIFICATION_RECEIVED] = g_signal_new ("notification-received", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, 0,
	                                                NULL, NULL, feed_marshal_VOID__OBJECT_OBJECT,
	                                                G_TYPE_NONE, 2, G_TYPE_OBJECT, G_TYPE_OBJECT);
}

static void
feeds_subscriber_init (FeedsSubscriber *node)
{
	node->priv = FEEDS_SUBSCRIBER_GET_PRIVATE (node);
	memset (node->priv, 0, sizeof (FeedsSubscriberPrivate));
	node->priv->parser = feed_parser_new ();
	node->priv->port = DEFAULT_SERVER_PORT;
}

/**
 * feeds_subscriber_new:
 *
 * Allocates a new #FeedsSubscriber
 *
 * Return value: a new #FeedsSubscriber
 */
FeedsSubscriber*
feeds_subscriber_new ()
{
	return g_object_new (FEEDS_SUBSCRIBER_TYPE, NULL);
}

static gboolean
create_listened (FeedsSubscriber *sub, GList *feeds)
{
	GList *list;
	GList *iter;
	FeedChannel *feed;
	FeedChannelWrap *wrap;

	for (iter = feeds; iter; iter = g_list_next (iter)) {
		feed = (FeedChannel*) iter->data;

		if (feed_channel_get_pubsubhub (feed, NULL, NULL) == FALSE) {
			g_warning ("Feed at %s has not PubSubHubbub capability", feed_channel_get_source (feed));
			return FALSE;
		}
	}

	list = NULL;

	for (iter = feeds; iter; iter = g_list_next (iter)) {
		feed = (FeedChannel*) iter->data;

		wrap = g_new0 (FeedChannelWrap, 1);
		g_object_ref (feed);
		wrap->status = FEED_SUBSCRIPTION_IDLE;
		wrap->path = NULL;
		wrap->channel = feed;
		wrap->sub = sub;
		list = g_list_prepend (list, wrap);
	}

	sub->priv->feeds_list = g_list_reverse (list);
	return TRUE;
}

/**
 * feeds_subscriber_listen:
 * @sub: a #FeedsSubscriber
 * @feeds: a list of #FeedChannel
 *
 * To set the list of feeds to be managed by @sub. The previous list, if any,
 * is invalidated. After invokation to the function, feeds_subscriber_switch()
 * must be call to run the subscription.
 * The list in @feeds can be freed after calling this; linked #FeedChannel
 * are g_object_ref'd here
 *
 * Return value: %TRUE if all #FeedChannels involved in @feeds are valid
 * (feed_channel_get_pubsubhub() returns %TRUE), %FALSE otherwise
 */
gboolean
feeds_subscriber_listen (FeedsSubscriber *sub, GList *feeds)
{
	remove_currently_listened (sub);
	return create_listened (sub, feeds);
}

/**
 * feeds_subscriber_get_listened:
 * @sub: a #FeedsSubscriber
 *
 * Returns the list of feeds currently managed by @sub
 *
 * Return value: a list of #FeedChannel, to be freed with g_list_free() when
 * no longer in use. Do not modify elements found in this list
 */
GList*
feeds_subscriber_get_listened (FeedsSubscriber *sub)
{
	GList *ret;
	GList *iter;

	ret = NULL;

	for (iter = sub->priv->feeds_list; iter; iter = g_list_next (iter))
		ret = g_list_prepend (ret, ((FeedChannelWrap*)iter->data)->channel);

	return g_list_reverse (ret);
}

static void
check_complete_job (SubscriberJob *job)
{
	job->counter--;

	if (job->counter <= 0) {
		job->callback (job->subscriber);
		g_free (job);
	}
}

static void
handle_incoming_notification_cb (SoupServer *server, SoupMessage *msg, const char *path,
                                 GHashTable *query, SoupClientContext *client, gpointer user_data)
{
	gchar *mode;
	gchar *challenge;
	GList *iter;
	GList *items;
	GError *error;
	xmlDocPtr doc;
	FeedChannelWrap *feed;

	feed = (FeedChannelWrap*) user_data;

	if (query != NULL) {
		mode = (gchar*) g_hash_table_lookup (query, "hub.mode");

		if (feed->status == FEED_SUBSCRIPTION_SUBSCRIBING && strcmp (mode, "subscribe") == 0) {
			challenge = g_strdup ((gchar*) g_hash_table_lookup (query, "hub.challenge"));
			soup_message_set_response (msg, "application/x-www-form-urlencoded", SOUP_MEMORY_TAKE, challenge, strlen (challenge));

			soup_message_set_status (msg, 200);
		}
		else if (feed->status == FEED_SUBSCRIPTION_UNSUBSCRIBING && strcmp (mode, "unsubscribe") == 0) {
			feed->status = FEED_SUBSCRIPTION_IDLE;

			challenge = g_strdup ((gchar*) g_hash_table_lookup (query, "hub.challenge"));
			soup_message_set_response (msg, "application/x-www-form-urlencoded", SOUP_MEMORY_TAKE, challenge, strlen (challenge));

			soup_message_set_status (msg, 200);
		}
	}
	else if (feed->status == FEED_SUBSCRIPTION_SUBSCRIBED) {
		/*
			TODO	Parsing and notification has to be moved in a
				g_idle_add() function, so to reply to the
				server as soon as possible
		*/

		doc = content_to_xml (msg->request_body->data, strlen (msg->request_body->data));
		error = NULL;
		items = feed_parser_parse (feed->sub->priv->parser, feed->channel, doc, &error);

		if (items == NULL) {
			g_warning ("Unable to parse notification from %s: %s", feed_channel_get_source (feed->channel), error->message);
			g_error_free (error);
		}
		else {
			for (iter = items; iter; iter = g_list_next (iter))
				g_signal_emit (feed->sub, signals [NOTIFICATION_RECEIVED], 0, feed->channel, (FeedItem*) iter->data, NULL);
			g_list_free (items);
		}

		xmlFreeDoc (doc);
		soup_message_set_status (msg, 202);
	}
	else {
		soup_message_set_status (msg, 404);
	}
}

static void
unregister_handlers (FeedsSubscriber *sub)
{
	GList *iter;
	FeedChannelWrap *feed;

	for (iter = sub->priv->feeds_list; iter; iter = g_list_next (iter)) {
		feed = (FeedChannelWrap*) iter->data;
		soup_server_remove_handler (sub->priv->server, feed->path);
	}
}

static void
register_handlers (FeedsSubscriber *sub)
{
	register int i;
	GList *iter;
	FeedChannelWrap *feed;

	for (i = 1, iter = sub->priv->feeds_list; iter; iter = g_list_next (iter), i++) {
		feed = (FeedChannelWrap*) iter->data;
		feed->identifier = i;
		feed->status = FEED_SUBSCRIPTION_SUBSCRIBING;

		FREE_STRING (feed->path);
		feed->path = g_strdup_printf ("/%d", feed->identifier);
		soup_server_add_handler (sub->priv->server, feed->path, handle_incoming_notification_cb, feed, NULL);
	}
}

static void
close_server (FeedsSubscriber *sub)
{
	if (sub->priv->server != NULL) {
		unregister_handlers (sub);
		soup_server_quit (sub->priv->server);
		g_object_unref (sub->priv->server);
		sub->priv->server = NULL;
	}
}

static void
feeds_subscribed_cb (FeedsSubscriber *sub)
{
	if (sub->priv->has_errors_in_subscription == TRUE)
		try_another_subscription_policy (sub);
}

static void
subscribe_response_cb (SoupSession *session, SoupMessage *msg, gpointer user_data)
{
	guint status;
	SubscriberJob *job;

	job = (SubscriberJob*) user_data;

	g_object_get (msg, "status-code", &status, NULL);
	if (status < 200 || status > 299) {
		g_warning ("Unable to subscribe feed: %s", msg->response_body->data);
		job->subscriber->priv->has_errors_in_subscription = TRUE;
	}

	check_complete_job (job);
}

static void
subscribe_feed (FeedsSubscriber *sub, FeedChannelWrap *feed, SubscriberJob *job)
{
	gchar *body;
	gchar *pubsubhub;
	gchar *feed_reference;
	SoupMessage *msg;

	if (feed_channel_get_pubsubhub (feed->channel, &pubsubhub, &feed_reference) == FALSE)
		return;

	if (sub->priv->hub != NULL)
		pubsubhub = sub->priv->hub;

	body = g_strdup_printf ("hub.mode=subscribe&hub.callback=http://%s:%d/%d&hub.topic=%s&hub.verify=sync",
	                        g_inet_address_to_string (sub->priv->external_addr), sub->priv->port, feed->identifier, feed_reference);

	msg = soup_message_new ("POST", pubsubhub);
	soup_message_set_request (msg, "application/x-www-form-urlencoded", SOUP_MEMORY_TAKE, body, strlen (body));

	soup_session_queue_message (sub->priv->soupsession, msg, subscribe_response_cb, job);
}

static void
subscribe_feeds (FeedsSubscriber *sub)
{
	GList *iter;
	FeedChannelWrap *feed;
	SubscriberJob *job;

	if (sub->priv->feeds_list == NULL)
		return;

	job = g_new0 (SubscriberJob, 1);
	job->counter = g_list_length (sub->priv->feeds_list);
	job->callback = feeds_subscribed_cb;
	job->subscriber = sub;

	sub->priv->has_errors_in_subscription = FALSE;

	for (iter = sub->priv->feeds_list; iter; iter = g_list_next (iter)) {
		feed = (FeedChannelWrap*) iter->data;
		subscribe_feed (sub, feed, job);
	}
}

static GInetAddress*
my_detect_internet_address (FeedsSubscriber *sub)
{
	if (sub->priv->local_addr == NULL)
		sub->priv->local_addr = detect_internet_address ();

	return sub->priv->local_addr;
}

static void
create_and_run_server (FeedsSubscriber *sub)
{
	gchar *ip;
	struct sockaddr_in low_addr;
	SoupAddress *soup_addr;
	GInetAddress *my_addr;

	my_addr = my_detect_internet_address (sub);
	if (my_addr == NULL)
		return;

	low_addr.sin_family = AF_INET;
	low_addr.sin_port = htons (sub->priv->port);
	ip = g_inet_address_to_string (my_addr);
	inet_pton (AF_INET, ip, &low_addr.sin_addr);
	g_free (ip);

	soup_addr = soup_address_new_from_sockaddr ((struct sockaddr*) &low_addr, sizeof (low_addr));
	sub->priv->server = soup_server_new ("port", sub->priv->port, "interface", soup_addr, NULL);
	g_object_unref (soup_addr);

	register_handlers (sub);
	soup_server_run_async (sub->priv->server);

	subscribe_feeds (sub);
}

static void
external_ip_received_cb (SoupSession *session, SoupMessage *msg, gpointer data)
{
	FeedsSubscriber *sub;

	sub = (FeedsSubscriber*) data;

	sub->priv->external_addr = g_inet_address_new_from_string (msg->response_body->data);
	if (sub->priv->external_addr == NULL) {
		g_warning ("Unable to determine public IP");
		return;
	}

	create_and_run_server (sub);
}

static void
subscribe_with_external_ip (FeedsSubscriber *sub)
{
	SoupMessage *msg;

	sub->priv->initing_status = SUBSCRIBER_CHECKING_PUBLIC_IP;

	/*
		This method to determine public IP is quite odd, but no
		better has been suggested by StackOverflow.com
	*/
	msg = soup_message_new ("GET", "http://whatismyip.org");
	soup_session_queue_message (sub->priv->soupsession, msg, external_ip_received_cb, sub);
}

static void
try_another_subscription_policy (FeedsSubscriber *sub)
{
	switch (sub->priv->initing_status) {
		case SUBSCRIBER_TRYING_LOCAL_IP:
			close_server (sub);
			subscribe_with_external_ip (sub);
			break;

		default:
			close_server (sub);
			g_warning ("No way: subscription is failed");
			break;
	}
}

static void
init_run_server (FeedsSubscriber *sub)
{
	gboolean done;
	GInetAddress *addr;

	done = FALSE;

	if (sub->priv->external_addr != NULL) {
		g_object_unref (sub->priv->external_addr);
		sub->priv->external_addr = NULL;
	}

	if (sub->priv->soupsession == NULL)
		sub->priv->soupsession = soup_session_async_new ();

	/*
		Flow:

		        BEGIN
		          |
		  +---------------+               +--------------+
		  | has fixed hub | ---- YES ---> | is hub local | ----- YES ---+
		  +---------------+               +--------------+              |
		          |                               |                     |
		          NO <----------------------------+                     |
		          |                                                     |
		+-------------------+           +-----------------+             |
		| host seems public | -- YES -> | subscribe works | ---- YES ---+
		+-------------------+           +-----------------+             |
		          |                               |                     |
		          NO -----------------------------+                     |
		          |                                                     |
		 +-----------------+                                            |
		 | check public IP |                                            |
		 +-----------------+                                            |
		          |                                                     |
		 +-----------------+                   +------+                 |
		 | subscribe works | --- YES --------> | DONE | <---------------+
		 +-----------------+                   +------+
		          |
		          NO
		          |
		      +--------+
		      | NO WAY |
		      +--------+
	*/

	sub->priv->initing_status = SUBSCRIBER_IS_IDLE;

	if (sub->priv->hub != NULL) {
		addr = g_inet_address_new_from_string (sub->priv->hub);
		if (g_inet_address_get_is_link_local (addr) == TRUE) {
			sub->priv->external_addr = my_detect_internet_address (sub);
			done = TRUE;
			create_and_run_server (sub);
		}
	}

	if (done == FALSE) {
		addr = my_detect_internet_address (sub);
		if (address_seems_public (addr) == TRUE) {
			sub->priv->external_addr = addr;
			done = TRUE;
			sub->priv->initing_status = SUBSCRIBER_TRYING_LOCAL_IP;
			create_and_run_server (sub);
		}
	}

	if (done == FALSE)
		subscribe_with_external_ip (sub);
}

static void
feeds_unsubscribed_cb (FeedsSubscriber *sub)
{
	close_server (sub);
	g_object_unref (sub->priv->soupsession);
	sub->priv->soupsession = NULL;
}

static void
unsubscribe_response_cb (SoupSession *session, SoupMessage *msg, gpointer user_data)
{
	SubscriberJob *job;

	job = (SubscriberJob*) user_data;
	check_complete_job (job);
}

static void
unsubscribe_feed (FeedsSubscriber *sub, FeedChannelWrap *feed, SubscriberJob *job)
{
	gchar *body;
	gchar *pubsubhub;
	gchar *feed_reference;
	SoupMessage *msg;

	if (feed_channel_get_pubsubhub (feed->channel, &pubsubhub, &feed_reference) == FALSE) {
		check_complete_job (job);
		return;
	}

	feed->status = FEED_SUBSCRIPTION_UNSUBSCRIBING;

	if (sub->priv->hub != NULL)
		pubsubhub = sub->priv->hub;

	body = g_strdup_printf ("hub.mode=unsubscribe&hub.callback=http://%s:%d/%d&hub.topic=%s&hub.verify=sync",
	                        g_inet_address_to_string (sub->priv->external_addr), sub->priv->port, feed->identifier, feed_reference);

	msg = soup_message_new ("POST", pubsubhub);
	soup_message_set_request (msg, "application/x-www-form-urlencoded", SOUP_MEMORY_TAKE, body, strlen (body));

	soup_session_queue_message (sub->priv->soupsession, msg, unsubscribe_response_cb, job);
}

static void
unsubscribe_feeds (FeedsSubscriber *sub)
{
	GList *iter;
	FeedChannelWrap *wrap;
	SubscriberJob *job;

	job = g_new0 (SubscriberJob, 1);
	job->counter = g_list_length (sub->priv->feeds_list);
	job->callback = feeds_unsubscribed_cb;
	job->subscriber = sub;

	for (iter = sub->priv->feeds_list; iter; iter = g_list_next (iter)) {
		wrap = (FeedChannelWrap*) iter->data;
		unsubscribe_feed (sub, wrap, job);
	}

	sub->priv->feeds_list = NULL;
}

static void
stop_server (FeedsSubscriber *sub)
{
	unsubscribe_feeds (sub);
}

/**
 * feeds_subscriber_set_port:
 * @sub: a #FeedsSubscriber
 * @port: new listening port for the server
 *
 * To customize the port opened by the local server to catch incoming
 * publishers' events. By default this is 8444. Changing the port while the
 * subscriber is running imply restart the local server.
 * Pay attention to the fact many publishers' implementations accept only
 * certain ports
 */
void
feeds_subscriber_set_port (FeedsSubscriber *sub, int port)
{
	if (port != sub->priv->port) {
		sub->priv->port = port;

		if (sub->priv->running == TRUE) {
			feeds_subscriber_switch (sub, FALSE);
			feeds_subscriber_switch (sub, TRUE);
		}
	}
}

/**
 * feeds_subscriber_set_hub:
 * @sub: a #FeedsSubscriber
 * @hub: URL of the custom hub
 *
 * To customize the default hub to which send subscriptions. If this value is
 * set, hubs from specific feeds are ignored
 */
void
feeds_subscriber_set_hub (FeedsSubscriber *sub, gchar *hub)
{
	FREE_STRING (sub->priv->hub);
	if (hub != NULL)
		sub->priv->hub = g_strdup (hub);
}

/**
 * feeds_subscriber_switch:
 * @sub: a #FeedsSubscriber
 * @run: TRUE to run the subscriber, FALSE to pause it
 *
 * Permits to pause or resume @sub listening for events
 */
void
feeds_subscriber_switch (FeedsSubscriber *sub, gboolean run)
{
	if (sub->priv->running != run) {
		sub->priv->running = run;

		if (run == TRUE)
			init_run_server (sub);
		else
			stop_server (sub);
	}
}
