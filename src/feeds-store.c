/*
 * Copyright (C) 2009/2010, Roberto Guido <rguido@src.gnome.org>
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

#include "feeds-store.h"
#include "utils.h"
#include "feeds-pool.h"

#define FEEDS_STORE_GET_PRIVATE(obj)	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), GRSS_FEEDS_STORE_TYPE, GrssFeedsStorePrivate))

/**
 * SECTION: feeds-store
 * @short_description: abstract class for feeds permanent storage
 *
 * #GrssFeedsStore is a class which abstracts storage of feeds, implementing some
 * behaviours valid for all. Extensions of this must provide implementation
 * of different callbacks so to permit permanent saving of channels and feeds.
 */

G_DEFINE_ABSTRACT_TYPE (GrssFeedsStore, grss_feeds_store, G_TYPE_OBJECT);

struct _GrssFeedsStorePrivate {
	gboolean	running;
	GrssFeedsPool	*pool;
};

static void
grss_feeds_store_class_init (GrssFeedsStoreClass *klass)
{
	g_type_class_add_private (klass, sizeof (GrssFeedsStorePrivate));
}

static void
grss_feeds_store_init (GrssFeedsStore *node)
{
	node->priv = FEEDS_STORE_GET_PRIVATE (node);
	node->priv->running = FALSE;
	node->priv->pool = NULL;
}

/**
 * grss_feeds_store_get_channels:
 * @store: a #GrssFeedsStore.
 *
 * To retrieve list of feeds permanently saved into the store.
 *
 * Returns: (element-type GrssFeedChannel) (transfer none): list
 * of #GrssFeedChannel found in the @store. Do not modify or free it.
 */
GList*
grss_feeds_store_get_channels (GrssFeedsStore *store)
{
	return GRSS_FEEDS_STORE_GET_CLASS (store)->get_channels (store);
}

/**
 * grss_feeds_store_get_items_by_channel:
 * @store: a #GrssFeedsStore.
 * @channel: parent feed containing required items.
 *
 * To retrieve list of items saved into the store, assigned to the given
 * @channel.
 *
 * Returns: (element-type GrssFeedItem) (transfer none): list of
 * #GrssFeedItem found in the @store. Do not modify or free it.
 */
GList*
grss_feeds_store_get_items_by_channel (GrssFeedsStore *store, GrssFeedChannel *channel)
{
	return GRSS_FEEDS_STORE_GET_CLASS (store)->get_items_by_channel (store, channel);
}

/**
 * grss_feeds_store_has_item:
 * @store: a #GrssFeedsStore.
 * @channel: parent feed containing required item.
 * @id: unique ID to look for.
 *
 * To retrieve an item into a feed, given his unique ID.
 *
 * Returns: %TRUE if the specified item exists, %FALSE otherwise.
 */
gboolean
grss_feeds_store_has_item (GrssFeedsStore *store, GrssFeedChannel *channel, const gchar *id)
{
	return GRSS_FEEDS_STORE_GET_CLASS (store)->has_item (store, channel, id);
}

/**
 * grss_feeds_store_add_item_in_channel:
 * @store: a #GrssFeedsStore.
 * @channel: parent feed for the new item.
 * @item: new item to permanently save.
 *
 * To save a new #GrssFeedItem into the @store. It performs a check to grant
 * @item is not already saved.
 */
void
grss_feeds_store_add_item_in_channel (GrssFeedsStore *store, GrssFeedChannel *channel, GrssFeedItem *item)
{
	if (grss_feeds_store_has_item (store, channel, grss_feed_item_get_id (item)) == FALSE)
		GRSS_FEEDS_STORE_GET_CLASS (store)->add_item_in_channel (store, channel, item);
}

static void
feed_fetched (GrssFeedsPool *pool, GrssFeedChannel *feed, GList *items, gpointer user_data)
{
	GList *iter;
	GrssFeedItem *item;
	GrssFeedsStore *store;

	store = (GrssFeedsStore*) user_data;

	for (iter = items; iter; iter = g_list_next (iter)) {
		item = (GrssFeedItem*) iter->data;
		grss_feeds_store_add_item_in_channel (store, feed, item);
	}
}

/**
 * grss_feeds_store_switch:
 * @store: a #GrssFeedsStore.
 * @run: %TRUE to run the @store, %FALSE to stop.
 *
 * This is to permit the @store to auto-update itself: it creates an internal
 * #GrssFeedsPool and listens for his signals, so to implement the whole loop
 * fetch-parse-save trasparently.
 */
void
grss_feeds_store_switch (GrssFeedsStore *store, gboolean run)
{
	GList *channels;

	if (store->priv->running == run)
		return;

	if (run == TRUE) {
		if (store->priv->pool == NULL) {
			store->priv->pool = grss_feeds_pool_new ();
			g_signal_connect (store->priv->pool, "feed-ready", G_CALLBACK (feed_fetched), store);
		}

		channels = grss_feeds_store_get_channels (store);
		grss_feeds_pool_listen (store->priv->pool, channels);
		grss_feeds_pool_switch (store->priv->pool, TRUE);
	}
	else {
		if (store->priv->pool != NULL)
			grss_feeds_pool_switch (store->priv->pool, FALSE);
	}

	store->priv->running = run;
}
