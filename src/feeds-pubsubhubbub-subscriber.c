/*
 * Copyright (C) 2011, Roberto Guido <rguido@src.gnome.org>
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

#include "feeds-pubsubhubbub-subscriber.h"
#include "feeds-subscriber.h"
#include "feeds-subscriber-handler.h"
#include "utils.h"
#include "feed-parser.h"

#define FEEDS_SUBSCRIBER_GET_PRIVATE(obj)	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), GRSS_FEEDS_PUBSUBHUBBUB_SUBSCRIBER_TYPE, GrssFeedsPubsubhubbubSubscriberPrivate))

struct _GrssFeedsPubsubhubbubSubscriberPrivate {
	GrssFeedsSubscriber	*parent;
	GrssFeedParser		*parser;
};

static void feeds_subscriber_handler_interface_init (GrssFeedsSubscriberHandlerInterface *iface);
G_DEFINE_TYPE_WITH_CODE (GrssFeedsPubsubhubbubSubscriber, grss_feeds_pubsubhubbub_subscriber, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GRSS_FEEDS_SUBSCRIBER_HANDLER_TYPE,
                                                feeds_subscriber_handler_interface_init));

static void
grss_feeds_pubsubhubbub_subscriber_finalize (GObject *obj)
{
	GrssFeedsPubsubhubbubSubscriber *sub;

	sub = GRSS_FEEDS_PUBSUBHUBBUB_SUBSCRIBER (obj);
	g_object_unref (sub->priv->parser);
}

static void
grss_feeds_pubsubhubbub_subscriber_class_init (GrssFeedsPubsubhubbubSubscriberClass *klass)
{
	GObjectClass *gobject_class;

	g_type_class_add_private (klass, sizeof (GrssFeedsPubsubhubbubSubscriberPrivate));

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->finalize = grss_feeds_pubsubhubbub_subscriber_finalize;
}

static void
grss_feeds_pubsubhubbub_subscriber_init (GrssFeedsPubsubhubbubSubscriber *node)
{
	node->priv = FEEDS_SUBSCRIBER_GET_PRIVATE (node);
	memset (node->priv, 0, sizeof (GrssFeedsPubsubhubbubSubscriberPrivate));
	node->priv->parser = grss_feed_parser_new ();
}

GrssFeedsPubsubhubbubSubscriber*
grss_feeds_pubsubhubbub_subscriber_new ()
{
	return g_object_new (GRSS_FEEDS_PUBSUBHUBBUB_SUBSCRIBER_TYPE, NULL);
}

static void
feeds_pubsubhubbub_subscriber_handler_set_parent (GrssFeedsSubscriberHandler *handler,
                                                  GrssFeedsSubscriber *parent)
{
	GrssFeedsPubsubhubbubSubscriber *myself;

	myself = GRSS_FEEDS_PUBSUBHUBBUB_SUBSCRIBER (handler);
	myself->priv->parent = parent;
}

static gboolean
feeds_pubsubhubbub_subscriber_handler_check_format (GrssFeedsSubscriberHandler *handler,
                                                    GrssFeedChannel *channel)
{
	return grss_feed_channel_get_pubsubhub (channel, NULL);
}

static void
subscribe_response_cb (SoupSession *session,
                       SoupMessage *msg,
                       gpointer user_data)
{
	guint status;

	g_object_get (msg, "status-code", &status, NULL);
	if (status < 200 || status > 299)
		g_warning ("Unable to subscribe feed: %s", msg->response_body->data);
}

static void
feeds_pubsubhubbub_subscriber_handler_subscribe (GrssFeedsSubscriberHandler *handler,
                                                 GrssFeedChannel *channel,
                                                 gchar *assigned_url)
{
	int local_port;
	gchar *body;
	gchar *pubsubhub;
	const gchar *source;
	GInetAddress *local_addr;
	SoupMessage *msg;
	GrssFeedsPubsubhubbubSubscriber *myself;

	if (grss_feed_channel_get_pubsubhub (channel, &pubsubhub) == FALSE)
		return;

	myself = GRSS_FEEDS_PUBSUBHUBBUB_SUBSCRIBER (handler);

	local_addr = grss_feeds_subscriber_get_address (myself->priv->parent);
	local_port = grss_feeds_subscriber_get_port (myself->priv->parent);
	source = grss_feed_channel_get_source (channel);

	body = g_strdup_printf ("hub.mode=subscribe&hub.callback=http://%s:%d/%s&hub.topic=%s&hub.verify=sync",
	                        g_inet_address_to_string (local_addr), local_port, assigned_url, source);

	msg = soup_message_new ("POST", pubsubhub);
	soup_message_set_request (msg, "application/x-www-form-urlencoded", SOUP_MEMORY_TAKE, body, strlen (body));

	soup_session_queue_message (grss_feeds_subscriber_get_session (myself->priv->parent), msg, subscribe_response_cb, NULL);
}

static GList*
feeds_pubsubhubbub_subscriber_handler_handle_message (GrssFeedsSubscriberHandler *handler,
                                                      GrssFeedChannel *channel,
                                                      FEED_SUBSCRIPTION_STATUS *status,
                                                      SoupServer *server,
                                                      SoupMessage *msg,
                                                      const char *path,
                                                      GHashTable *query,
                                                      SoupClientContext *client)
{
	gchar *mode;
	gchar *challenge;
	GList *items;
	GError *error;
	xmlDocPtr doc;
	GrssFeedsPubsubhubbubSubscriber *myself;

	items = NULL;

	if (query != NULL) {
		mode = (gchar*) g_hash_table_lookup (query, "hub.mode");

		if (*status == FEED_SUBSCRIPTION_SUBSCRIBING && strcmp (mode, "subscribe") == 0) {
			*status = FEED_SUBSCRIPTION_SUBSCRIBED;
			challenge = g_strdup ((gchar*) g_hash_table_lookup (query, "hub.challenge"));
			soup_message_set_response (msg, "application/x-www-form-urlencoded", SOUP_MEMORY_TAKE, challenge, strlen (challenge));
			soup_message_set_status (msg, 200);
		}
		else if (*status == FEED_SUBSCRIPTION_UNSUBSCRIBING && strcmp (mode, "unsubscribe") == 0) {
			*status = FEED_SUBSCRIPTION_IDLE;

			challenge = g_strdup ((gchar*) g_hash_table_lookup (query, "hub.challenge"));
			soup_message_set_response (msg, "application/x-www-form-urlencoded", SOUP_MEMORY_TAKE, challenge, strlen (challenge));

			soup_message_set_status (msg, 200);
		}
	}
	else if (*status == FEED_SUBSCRIPTION_SUBSCRIBED) {
		/*
			TODO	Parsing and notification has to be moved in a
				g_idle_add() function, so to reply to the
				server as soon as possible
		*/

		myself = GRSS_FEEDS_PUBSUBHUBBUB_SUBSCRIBER (handler);
		doc = content_to_xml (msg->request_body->data, strlen (msg->request_body->data));
		error = NULL;
		items = grss_feed_parser_parse (myself->priv->parser, channel, doc, &error);

		if (items == NULL) {
			g_warning ("Unable to parse notification from %s: %s", grss_feed_channel_get_source (channel), error->message);
			g_error_free (error);
		}

		xmlFreeDoc (doc);
		soup_message_set_status (msg, 202);
	}
	else {
		soup_message_set_status (msg, 404);
	}

	return items;
}

static void
unsubscribe_response_cb (SoupSession *session,
                         SoupMessage *msg,
                         gpointer user_data)
{
	/* dummy */
}

static void
feeds_pubsubhubbub_subscriber_handler_unsubscribe (GrssFeedsSubscriberHandler *handler,
                                                   GrssFeedChannel *channel,
                                                   gchar *assigned_url)
{
	int local_port;
	gchar *body;
	gchar *pubsubhub;
	const gchar *source;
	GInetAddress *local_addr;
	SoupMessage *msg;
	GrssFeedsPubsubhubbubSubscriber *myself;

	if (grss_feed_channel_get_pubsubhub (channel, &pubsubhub) == FALSE)
		return;

	myself = GRSS_FEEDS_PUBSUBHUBBUB_SUBSCRIBER (handler);

	local_addr = grss_feeds_subscriber_get_address (myself->priv->parent);
	local_port = grss_feeds_subscriber_get_port (myself->priv->parent);
	source = grss_feed_channel_get_source (channel);

	body = g_strdup_printf ("hub.mode=unsubscribe&hub.callback=http://%s:%d/%s&hub.topic=%s&hub.verify=sync",
	                        g_inet_address_to_string (local_addr), local_port, assigned_url, source);

	msg = soup_message_new ("POST", pubsubhub);
	soup_message_set_request (msg, "application/x-www-form-urlencoded", SOUP_MEMORY_TAKE, body, strlen (body));

	soup_session_queue_message (grss_feeds_subscriber_get_session (myself->priv->parent), msg, unsubscribe_response_cb, NULL);
}

static void
feeds_subscriber_handler_interface_init (GrssFeedsSubscriberHandlerInterface *iface)
{
	iface->set_parent = feeds_pubsubhubbub_subscriber_handler_set_parent;
	iface->check_format = feeds_pubsubhubbub_subscriber_handler_check_format;
	iface->subscribe = feeds_pubsubhubbub_subscriber_handler_subscribe;
	iface->handle_message = feeds_pubsubhubbub_subscriber_handler_handle_message;
	iface->unsubscribe = feeds_pubsubhubbub_subscriber_handler_unsubscribe;
}
