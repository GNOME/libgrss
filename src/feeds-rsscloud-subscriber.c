/*
 * Copyright (C) 2011-2015, Roberto Guido <rguido@src.gnome.org>
 *                          Michele Tameni <michele@amdplanet.it>
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

#include "feeds-rsscloud-subscriber.h"
#include "feeds-subscriber.h"
#include "feeds-subscriber-private.h"
#include "feeds-subscriber-handler.h"
#include "utils.h"
#include "feed-parser.h"

#define FEEDS_SUBSCRIBER_GET_PRIVATE(obj)	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), GRSS_FEEDS_RSSCLOUD_SUBSCRIBER_TYPE, GrssFeedsRsscloudSubscriberPrivate))

struct _GrssFeedsRsscloudSubscriberPrivate {
	GrssFeedsSubscriber	*parent;
};

static void feeds_subscriber_handler_interface_init (GrssFeedsSubscriberHandlerInterface *iface);
G_DEFINE_TYPE_WITH_CODE (GrssFeedsRsscloudSubscriber, grss_feeds_rsscloud_subscriber, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GRSS_FEEDS_SUBSCRIBER_HANDLER_TYPE,
                                                feeds_subscriber_handler_interface_init));

static void
grss_feeds_rsscloud_subscriber_finalize (GObject *obj)
{
	/* dummy */
}

static void
grss_feeds_rsscloud_subscriber_class_init (GrssFeedsRsscloudSubscriberClass *klass)
{
	GObjectClass *gobject_class;

	g_type_class_add_private (klass, sizeof (GrssFeedsRsscloudSubscriberPrivate));

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->finalize = grss_feeds_rsscloud_subscriber_finalize;
}

static void
grss_feeds_rsscloud_subscriber_init (GrssFeedsRsscloudSubscriber *node)
{
	node->priv = FEEDS_SUBSCRIBER_GET_PRIVATE (node);
	memset (node->priv, 0, sizeof (GrssFeedsRsscloudSubscriberPrivate));
}

GrssFeedsRsscloudSubscriber*
grss_feeds_rsscloud_subscriber_new ()
{
	return g_object_new (GRSS_FEEDS_RSSCLOUD_SUBSCRIBER_TYPE, NULL);
}

static void
feeds_rsscloud_subscriber_handler_set_parent (GrssFeedsSubscriberHandler *handler,
                                              GrssFeedsSubscriber *parent)
{
	GrssFeedsRsscloudSubscriber *myself;

	myself = GRSS_FEEDS_RSSCLOUD_SUBSCRIBER (handler);
	myself->priv->parent = parent;
}

static void
subscribe_response_cb (SoupSession *session,
                       SoupMessage *msg,
                       gpointer user_data)
{
	gboolean done;
	xmlChar *success;
	guint status;
	xmlDocPtr doc;
	xmlNodePtr cur;

	done = TRUE;

	g_object_get (msg, "status-code", &status, NULL);

	if (status < 200 || status > 299) {
		done = FALSE;
	}
	else {
		doc = content_to_xml (msg->response_body->data, strlen (msg->response_body->data));
		success = NULL;

		if ((cur = xmlDocGetRootElement (doc)) == NULL ||
				xmlStrcmp (cur->name, BAD_CAST "notifyResult") != 0 ||
				(success = xmlGetProp (cur, BAD_CAST"success")) == NULL ||
				xmlStrcmp (success, BAD_CAST "true") != 0)
			done = FALSE;

		xmlFreeDoc (doc);

		if (success != NULL)
			xmlFree (success);
	}

	if (done == FALSE) {
		g_warning ("Unable to subscribe feed: %s", msg->response_body->data);
	}
}

gboolean
feeds_rsscloud_subscriber_handler_check_format (GrssFeedsSubscriberHandler *handler,
                                                GrssFeedChannel *channel)
{
	return grss_feed_channel_get_rsscloud (channel, NULL, NULL);
}

void
feeds_rsscloud_subscriber_handler_subscribe (GrssFeedsSubscriberHandler *handler,
                                             GrssFeedChannel *channel,
                                             gchar *assigned_url)
{
	int local_port;
	gchar *body;
	gchar *path;
	gchar *protocol;
	const gchar *source;
	SoupMessage *msg;
	GInetAddress *local_addr;
	GrssFeedsRsscloudSubscriber *myself;

	if (grss_feed_channel_get_rsscloud (channel, &path, &protocol) == FALSE)
		return;

	myself = GRSS_FEEDS_RSSCLOUD_SUBSCRIBER (handler);

	local_addr = grss_feeds_subscriber_get_address (myself->priv->parent);
	local_port = grss_feeds_subscriber_get_port (myself->priv->parent);
	source = grss_feed_channel_get_source (channel);

	body = g_strdup_printf ("notifyProcedure=\"\"&protocol=%s&domain=%s&path=%s&port=%d&url1=%s",
				protocol, g_inet_address_to_string (local_addr), assigned_url, local_port, source);

	msg = soup_message_new ("POST", path);
	soup_message_set_request (msg, "application/x-www-form-urlencoded", SOUP_MEMORY_TAKE, body, strlen (body));
	soup_session_queue_message (grss_feeds_subscriber_get_session (myself->priv->parent), msg, subscribe_response_cb, NULL);
}

static void
fetched_items_cb (GObject *source_object,
                  GAsyncResult *res,
                  gpointer user_data)
{
	GList *items;
	GrssFeedChannel *channel;
	GrssFeedsRsscloudSubscriber *handler;

	items = g_async_result_get_user_data (res);
	channel = GRSS_FEED_CHANNEL (g_async_result_get_source_object (res));
	handler = user_data;

	grss_feeds_subscriber_dispatch (handler->priv->parent, channel, items);
}

GList*
feeds_rsscloud_subscriber_handler_handle_message (GrssFeedsSubscriberHandler *handler,
                                                  GrssFeedChannel *channel,
                                                  FEED_SUBSCRIPTION_STATUS *status,
                                                  SoupServer *server,
                                                  SoupMessage *msg,
                                                  const char *path,
                                                  GHashTable *query,
                                                  SoupClientContext *client)
{
	GList *ret;
	gchar *challenge;

	ret = NULL;

	if (query != NULL) {
		challenge = (gchar*) g_hash_table_lookup (query, "challenge");

		if (*status == FEED_SUBSCRIPTION_SUBSCRIBING && challenge != NULL) {
			*status = FEED_SUBSCRIPTION_SUBSCRIBED;
			challenge = g_strdup (challenge);
			soup_message_set_response (msg, "application/x-www-form-urlencoded", SOUP_MEMORY_TAKE, challenge, strlen (challenge));
			soup_message_set_status (msg, 200);
		}
	}
	else if (*status == FEED_SUBSCRIPTION_SUBSCRIBED) {
		grss_feed_channel_fetch_all_async (channel, fetched_items_cb, handler);
		soup_message_set_status (msg, 202);
	}
	else {
		soup_message_set_status (msg, 404);
	}

	return ret;
}

void
feeds_rsscloud_subscriber_handler_unsubscribe (GrssFeedsSubscriberHandler *handler,
                                               GrssFeedChannel *channel,
                                               gchar *assigned_url)
{
	/* dummy */
}

static void
feeds_subscriber_handler_interface_init (GrssFeedsSubscriberHandlerInterface *iface)
{
	iface->set_parent = feeds_rsscloud_subscriber_handler_set_parent;
	iface->check_format = feeds_rsscloud_subscriber_handler_check_format;
	iface->subscribe = feeds_rsscloud_subscriber_handler_subscribe;
	iface->handle_message = feeds_rsscloud_subscriber_handler_handle_message;
	iface->unsubscribe = feeds_rsscloud_subscriber_handler_unsubscribe;
}
