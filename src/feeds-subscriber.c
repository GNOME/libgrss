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

#include "feeds-subscriber.h"
#include "feeds-subscriber-handler.h"
#include "feeds-pubsubhubbub-subscriber.h"
#include "feeds-rsscloud-subscriber.h"
#include "utils.h"
#include "feed-parser.h"
#include "feed-marshal.h"

#define DEFAULT_SERVER_PORT   8444

#define FEEDS_SUBSCRIBER_GET_PRIVATE(obj)	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), GRSS_FEEDS_SUBSCRIBER_TYPE, GrssFeedsSubscriberPrivate))

/**
 * SECTION: feeds-subscriber
 * @short_description: feeds subscriber
 *
 * #GrssFeedsSubscriber is an alternative for #GrssFeedsPool, able to receive
 * real-time notifications by feeds managed by one of the supported protocols.
 * When the subscriber is executed (with grss_feeds_subscriber_switch()) it opens
 * a server on a local port (configurable with grss_feeds_subscriber_set_port()),
 * engage a subscription for each #GrssFeedChannel passed with
 * grss_feeds_subscriber_listen(), and waits for direct notifications by the
 * remote server.
 */

/*
	TODO	There were an error in refreshing subscriptions, since is the
		hub which must send refresh requests, not subscriber.
		That has been removed, but it would be better to provide a
		control mechanism able to refresh subscriptions from the
		client side in case of problems by the server side
*/

static void	subscribe_feeds			(GrssFeedsSubscriber *sub);

struct _GrssFeedsSubscriberPrivate {
	gboolean		running;

	int			port;
	SoupServer		*server;
	GInetAddress		*local_addr;
	GInetAddress		*exposed_addr;

	SoupSession		*soupsession;

	GrssFeedParser		*parser;
	GList			*handlers;
	GList			*feeds_list;
};

typedef struct {
	GrssFeedChannel			*channel;

	FEED_SUBSCRIPTION_STATUS	status;
	gchar				*identifier;
	gchar				*path;

	GrssFeedsSubscriber		*sub;
	GrssFeedsSubscriberHandler	*handler;

	GList				*items_cache;
} GrssFeedChannelWrap;

enum {
	NOTIFICATION_RECEIVED,
	LAST_SIGNAL
};

static guint signals [LAST_SIGNAL] = {0};

G_DEFINE_TYPE (GrssFeedsSubscriber, grss_feeds_subscriber, G_TYPE_OBJECT);

static void
remove_currently_listened (GrssFeedsSubscriber *sub)
{
	GList *iter;
	GList *cache_iter;
	GrssFeedChannelWrap *wrap;

	if (sub->priv->feeds_list != NULL) {
		for (iter = sub->priv->feeds_list; iter; iter = g_list_next (iter)) {
			wrap = (GrssFeedChannelWrap*) iter->data;

			if (wrap->items_cache != NULL) {
				for (cache_iter = wrap->items_cache; cache_iter; cache_iter = g_list_next (cache_iter))
					g_object_unref (cache_iter->data);
				g_list_free (wrap->items_cache);
			}

			g_free (wrap->path);
			g_object_unref (wrap->channel);
			g_free (wrap);
		}

		g_list_free (sub->priv->feeds_list);
	}
}

static void
notification_handled_cb (GrssFeedsSubscriber *sub, GrssFeedChannel *feed, GrssFeedItem *item)
{
	g_object_unref (item);
}

static void
grss_feeds_subscriber_finalize (GObject *obj)
{
	GrssFeedsSubscriber *sub;

	sub = GRSS_FEEDS_SUBSCRIBER (obj);
	grss_feeds_subscriber_switch (sub, FALSE);
	remove_currently_listened (sub);
	g_object_unref (sub->priv->parser);
}

static void
grss_feeds_subscriber_class_init (GrssFeedsSubscriberClass *klass)
{
	GObjectClass *gobject_class;

	g_type_class_add_private (klass, sizeof (GrssFeedsSubscriberPrivate));

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->finalize = grss_feeds_subscriber_finalize;

	klass->notification_received = notification_handled_cb;

	/**
	 * GrssFeedsSubscriber::notification-received:
	 * @pool: the #GrssFeedsSubscriber emitting the signal
	 * @feed: the #GrssFeedChannel which has been updated
	 * @item: the #GrssFeedItem received
	 *
	 * Emitted when a notification has been received and parsed. The
	 * @item is cached and unref'd when the #GrssFeedsSubscriber is
	 * destroyed or a new set of feeds is provided
	 */
	signals [NOTIFICATION_RECEIVED] = g_signal_new ("notification-received", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, 0,
	                                                NULL, NULL, feed_marshal_VOID__OBJECT_OBJECT,
	                                                G_TYPE_NONE, 2, G_TYPE_OBJECT, G_TYPE_OBJECT);
}

static void
grss_feeds_subscriber_init (GrssFeedsSubscriber *node)
{
	GrssFeedsSubscriberHandler *handler;

	node->priv = FEEDS_SUBSCRIBER_GET_PRIVATE (node);
	memset (node->priv, 0, sizeof (GrssFeedsSubscriberPrivate));
	node->priv->parser = grss_feed_parser_new ();
	node->priv->port = DEFAULT_SERVER_PORT;

	node->priv->handlers = NULL;

	handler = GRSS_FEEDS_SUBSCRIBER_HANDLER (grss_feeds_pubsubhubbub_subscriber_new ());
	node->priv->handlers = g_list_prepend (node->priv->handlers, handler);
	grss_feeds_subscriber_handler_set_parent (handler, node);

	handler = GRSS_FEEDS_SUBSCRIBER_HANDLER (grss_feeds_rsscloud_subscriber_new ());
	node->priv->handlers = g_list_prepend (node->priv->handlers, handler);
	grss_feeds_subscriber_handler_set_parent (handler, node);
}

/**
 * grss_feeds_subscriber_new:
 *
 * Allocates a new #GrssFeedsSubscriber
 *
 * Return value: a new #GrssFeedsSubscriber
 */
GrssFeedsSubscriber*
grss_feeds_subscriber_new ()
{
	return g_object_new (GRSS_FEEDS_SUBSCRIBER_TYPE, NULL);
}

static GrssFeedsSubscriberHandler*
retrieve_handler (GrssFeedsSubscriber *sub, GrssFeedChannel *feed)
{
	GList *iter;
	GrssFeedsSubscriberHandler *handler;

	for (iter = sub->priv->handlers; iter; iter = g_list_next (iter)) {
		handler = (GrssFeedsSubscriberHandler*) iter->data;
		if (grss_feeds_subscriber_handler_check_format (handler, feed) == TRUE)
			return handler;
	}

	return NULL;
}

static gboolean
create_listened (GrssFeedsSubscriber *sub, GList *feeds)
{
	GList *list;
	GList *iter;
	GrssFeedChannel *feed;
	GrssFeedChannelWrap *wrap;
	GrssFeedsSubscriberHandler *handler;

	for (iter = feeds; iter; iter = g_list_next (iter)) {
		feed = (GrssFeedChannel*) iter->data;

		handler = retrieve_handler (sub, feed);
		if (handler == NULL)
			return FALSE;
	}

	list = NULL;

	for (iter = feeds; iter; iter = g_list_next (iter)) {
		feed = (GrssFeedChannel*) iter->data;

		wrap = g_new0 (GrssFeedChannelWrap, 1);
		g_object_ref (feed);
		wrap->status = FEED_SUBSCRIPTION_IDLE;
		wrap->path = NULL;
		wrap->channel = feed;
		wrap->sub = sub;
		wrap->handler = retrieve_handler (sub, feed);
		list = g_list_prepend (list, wrap);
	}

	sub->priv->feeds_list = g_list_reverse (list);
	return TRUE;
}

/**
 * grss_feeds_subscriber_listen:
 * @sub: a #GrssFeedsSubscriber
 * @feeds: a list of #GrssFeedChannel
 *
 * To set the list of feeds to be managed by @sub. The previous list, if any,
 * is invalidated. After invokation to the function, grss_feeds_subscriber_switch()
 * must be call to run the subscription.
 * The list in @feeds can be freed after calling this; linked #GrssFeedChannel
 * are g_object_ref'd here
 *
 * Return value: %TRUE if all #GrssFeedChannels involved in @feeds are valid
 * and can be listened with one of the implemented procotols, %FALSE otherwise
 */
gboolean
grss_feeds_subscriber_listen (GrssFeedsSubscriber *sub, GList *feeds)
{
	remove_currently_listened (sub);
	return create_listened (sub, feeds);
}

/**
 * grss_feeds_subscriber_get_listened:
 * @sub: a #GrssFeedsSubscriber
 *
 * Returns the list of feeds currently managed by @sub
 *
 * Return value: a list of #GrssFeedChannel, to be freed with g_list_free() when
 * no longer in use. Do not modify elements found in this list
 */
GList*
grss_feeds_subscriber_get_listened (GrssFeedsSubscriber *sub)
{
	GList *ret;
	GList *iter;

	ret = NULL;

	for (iter = sub->priv->feeds_list; iter; iter = g_list_next (iter))
		ret = g_list_prepend (ret, ((GrssFeedChannelWrap*)iter->data)->channel);

	return g_list_reverse (ret);
}

static void
dispatch_items (GrssFeedChannelWrap *feed, GList *items)
{
	gboolean exists;
	const gchar* item_id;
	GList *iter;
	GList *cache_iter;
	GrssFeedItem *item;
	GrssFeedItem *cache_item;

	for (iter = items; iter; iter = g_list_next (iter)) {
		item = (GrssFeedItem*) iter->data;
		item_id = grss_feed_item_get_id (item);

		exists = FALSE;

		for (cache_iter = feed->items_cache; cache_iter; cache_iter = g_list_next (cache_iter)) {
			cache_item = (GrssFeedItem*) cache_iter->data;
			if (strcmp (item_id, grss_feed_item_get_id (cache_item)) == 0)
				exists = TRUE;
		}

		if (exists == FALSE) {
			g_signal_emit (feed->sub, signals [NOTIFICATION_RECEIVED], 0, feed->channel, item, NULL);
			feed->items_cache = g_list_prepend (feed->items_cache, item);
		}
	}
}

static void
handle_incoming_notification_cb (SoupServer *server, SoupMessage *msg, const char *path,
                                 GHashTable *query, SoupClientContext *client, gpointer user_data)
{
	GList *items;
	GrssFeedChannelWrap *feed;

	feed = (GrssFeedChannelWrap*) user_data;
	items = grss_feeds_subscriber_handler_handle_incoming_message (feed->handler, feed->channel, &(feed->status), server, msg, path, query, client);

	if (items != NULL) {
		dispatch_items (feed, items);
		g_list_free (items);
	}
}

void
grss_feeds_subscriber_dispatch (GrssFeedsSubscriber *sub, GrssFeedChannel *channel, GList *items)
{
	GList *iter;
	GrssFeedChannelWrap *wrap;

	for (iter = sub->priv->feeds_list; iter; iter = g_list_next (iter)) {
		wrap = (GrssFeedChannelWrap*) iter->data;

		if (wrap->channel == channel) {
			dispatch_items (wrap, items);
			break;
		}
	}
}

static void
unregister_handlers (GrssFeedsSubscriber *sub)
{
	GList *iter;
	GrssFeedChannelWrap *feed;

	for (iter = sub->priv->feeds_list; iter; iter = g_list_next (iter)) {
		feed = (GrssFeedChannelWrap*) iter->data;
		soup_server_remove_handler (sub->priv->server, feed->path);
	}
}

static void
register_handlers (GrssFeedsSubscriber *sub)
{
	register int i;
	GList *iter;
	GrssFeedChannelWrap *feed;

	for (i = 1, iter = sub->priv->feeds_list; iter; iter = g_list_next (iter), i++) {
		feed = (GrssFeedChannelWrap*) iter->data;
		feed->identifier = g_strdup_printf ("%d", i);
		feed->status = FEED_SUBSCRIPTION_SUBSCRIBING;

		FREE_STRING (feed->path);
		feed->path = g_strdup_printf ("/%s", feed->identifier);
		soup_server_add_handler (sub->priv->server, feed->path, handle_incoming_notification_cb, feed, NULL);
	}
}

static void
close_server (GrssFeedsSubscriber *sub)
{
	if (sub->priv->server != NULL) {
		unregister_handlers (sub);
		soup_server_quit (sub->priv->server);
		g_object_unref (sub->priv->server);
		sub->priv->server = NULL;
	}
}

static void
subscribe_feeds (GrssFeedsSubscriber *sub)
{
	GList *iter;
	GrssFeedChannelWrap *feed;

	if (sub->priv->feeds_list == NULL)
		return;

	for (iter = sub->priv->feeds_list; iter; iter = g_list_next (iter)) {
		feed = (GrssFeedChannelWrap*) iter->data;
		grss_feeds_subscriber_handler_subscribe (feed->handler, feed->channel, feed->identifier);
	}
}

static GInetAddress*
my_detect_internet_address (GrssFeedsSubscriber *sub)
{
	if (sub->priv->local_addr == NULL)
		sub->priv->local_addr = detect_internet_address ();

	return sub->priv->local_addr;
}

static void
create_and_run_server (GrssFeedsSubscriber *sub)
{
	gchar *ip;
	struct sockaddr_in low_addr;
	SoupAddress *soup_addr;

	low_addr.sin_family = AF_INET;
	low_addr.sin_port = htons (sub->priv->port);
	ip = g_inet_address_to_string (sub->priv->local_addr);

	inet_pton (AF_INET, ip, &low_addr.sin_addr);
	g_free (ip);

	soup_addr = soup_address_new_from_sockaddr ((struct sockaddr*) &low_addr, sizeof (low_addr));
	if (soup_addr == NULL) {
		g_warning ("Unable to use detected exposed IP");
		return;
	}

	sub->priv->server = soup_server_new (SOUP_SERVER_INTERFACE, soup_addr, NULL);
	if (sub->priv->server == NULL) {
		g_warning ("Unable to open server on detected exposed IP");
		return;
	}

	g_object_unref (soup_addr);

	register_handlers (sub);
	soup_server_run_async (sub->priv->server);

	subscribe_feeds (sub);
}

static void
external_ip_received_cb (SoupSession *session, SoupMessage *msg, gpointer data)
{
	int i;
	int len;
	gchar *tmp;
	GrssFeedsSubscriber *sub;

	if (msg->status_code == SOUP_STATUS_OK) {
		sub = (GrssFeedsSubscriber*) data;

		/*
			Typical response from checkip.dyndns.org:

			<html><head><title>Current IP Check</title></head><body>Current IP Address: X.X.X.X</body></html>
			|                                                                          |
			+----------------------------------- 76 -----------------------------------+
		*/
		tmp = g_strdup (msg->response_body->data + 76);
		len = strlen (tmp);
		for (i = 0; tmp [i] != '<' && i < len; i++);

		if (i == len) {
			g_warning ("Unable to determine public IP: %s", msg->response_body->data);
		}
		else {
			tmp [i] = '\0';

			sub->priv->exposed_addr = g_inet_address_new_from_string (tmp);
			if (sub->priv->exposed_addr == NULL)
				g_warning ("Unable to determine public IP: %s", tmp);
			else
				create_and_run_server (sub);
		}

		g_free (tmp);
	}
	else {
		g_warning ("Unable to determine public IP: %s", soup_status_get_phrase (msg->status_code));
	}
}

static void
subscribe_with_external_ip (GrssFeedsSubscriber *sub)
{
	SoupMessage *msg;

	/*
		This method to determine public IP is quite odd, but no
		better has been suggested by StackOverflow.com
	*/
	msg = soup_message_new ("GET", "http://checkip.dyndns.org/");
	soup_session_queue_message (sub->priv->soupsession, msg, external_ip_received_cb, sub);
}

static void
init_run_server (GrssFeedsSubscriber *sub)
{
	GInetAddress *addr;

	if (sub->priv->soupsession == NULL)
		sub->priv->soupsession = soup_session_async_new ();

	/*
		Flow:

		        BEGIN
		          |
		          |
		+-------------------+           +-----------------+
		| host seems public | -- YES -> | subscribe works | ---- YES ---+
		+-------------------+           +-----------------+             |
		          |                               |                     |
		          NO -----------------------------+                     |
		          |                                                     |
		 +-----------------+            +-----------------+             |
		 | check public IP | --- YES -> | subscribe works | ---- YES ---+
		 +-----------------+            +-----------------+             |
		          |                               |                     |
		          NO -----------------------------+                     |
		          |                                                     |
		          |                                                     |
		        NO WAY                                                 DONE
	*/

	addr = my_detect_internet_address (sub);
	if (address_seems_public (addr) == TRUE) {
		sub->priv->exposed_addr = sub->priv->local_addr;
		create_and_run_server (sub);
	}
	else {
		subscribe_with_external_ip (sub);
	}
}

static void
unsubscribe_feeds (GrssFeedsSubscriber *sub)
{
	GList *iter;
	GrssFeedChannelWrap *wrap;

	for (iter = sub->priv->feeds_list; iter; iter = g_list_next (iter)) {
		wrap = (GrssFeedChannelWrap*) iter->data;
		grss_feeds_subscriber_handler_unsubscribe (wrap->handler, wrap->channel, wrap->identifier);
		wrap->status = FEED_SUBSCRIPTION_UNSUBSCRIBING;
	}

	sub->priv->feeds_list = NULL;
}

static void
stop_server (GrssFeedsSubscriber *sub)
{
	unsubscribe_feeds (sub);
	close_server (sub);
}

/**
 * grss_feeds_subscriber_set_port:
 * @sub: a #GrssFeedsSubscriber
 * @port: new listening port for the server
 *
 * To customize the port opened by the local server to catch incoming
 * publishers' events. By default this is 8444. Changing the port while the
 * subscriber is running imply restart the local server.
 * Pay attention to the fact many publishers' implementations accept only
 * certain ports
 */
void
grss_feeds_subscriber_set_port (GrssFeedsSubscriber *sub, int port)
{
	if (port != sub->priv->port) {
		sub->priv->port = port;

		if (sub->priv->running == TRUE) {
			grss_feeds_subscriber_switch (sub, FALSE);
			grss_feeds_subscriber_switch (sub, TRUE);
		}
	}
}

/**
 * grss_feeds_subscriber_switch:
 * @sub: a #GrssFeedsSubscriber
 * @run: TRUE to run the subscriber, FALSE to pause it
 *
 * Permits to pause or resume @sub listening for events
 */
void
grss_feeds_subscriber_switch (GrssFeedsSubscriber *sub, gboolean run)
{
	if (sub->priv->running != run) {
		sub->priv->running = run;

		if (run == TRUE)
			init_run_server (sub);
		else
			stop_server (sub);
	}
}

/**
 * grss_feeds_subscriber_get_address:
 * @sub: a #GrssFeedsSubscriber
 *
 * This function returns the Internet address where @sub is listening for
 * external events. It is often required by #GrssFeedsSubscriberHandlers while
 * subscribing contents to specify the local endpoint for communications
 *
 * Return value: the #GInetAddress used by @sub, or %NULL if the
 * #GrssFeedsSubscriber is switched off
 */
GInetAddress*
grss_feeds_subscriber_get_address (GrssFeedsSubscriber *sub)
{
	return sub->priv->exposed_addr;
}

/**
 * grss_feeds_subscriber_get_port:
 * @sub: a #GrssFeedsSubscriber
 *
 * This function returns the Internet port where @sub is listening for
 * external events. It is often required by #GrssFeedsSubscriberHandlers while
 * subscribing contents to specify the local endpoint for communications
 * 
 * Return value: the port of the socket locally opened by @sub
 */
int
grss_feeds_subscriber_get_port (GrssFeedsSubscriber *sub)
{
	return sub->priv->port;
}

/**
 * grss_feeds_subscriber_get_session:
 * @sub: a #GrssFeedsSubscriber
 *
 * To obtain the internal #SoupSession of a #GrssFeedsSubscriber, so to re-use
 * it in #GrssFeedsSubscriberHandlers or similar tasks
 * 
 * Return value: the #SoupSession used by the provided #GrssFeedsSubscriber
 */
SoupSession*
grss_feeds_subscriber_get_session (GrssFeedsSubscriber *sub)
{
	return sub->priv->soupsession;
}
