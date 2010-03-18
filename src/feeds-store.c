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

#define FEEDS_STORE_GET_PRIVATE(obj)	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), FEEDS_STORE_TYPE, FeedsStorePrivate))

/**
 * SECTION: feeds-store
 * @short_description: abstract class for feeds permanent storage
 *
 * #FeedsStore is a class which abstracts storage of feeds, implementing some
 * behaviours valid for all. Extensions of this must provide implementation
 * of different callbacks so to permit permanent saving of channels and feeds.
 */

G_DEFINE_ABSTRACT_TYPE (FeedsStore, feeds_store, G_TYPE_OBJECT);

struct _FeedsStorePrivate {
	gboolean	running;
	FeedsPool	*pool;
};

static void
feeds_store_class_init (FeedsStoreClass *klass)
{
	g_type_class_add_private (klass, sizeof (FeedsStorePrivate));
}

static void
feeds_store_init (FeedsStore *node)
{
	node->priv = FEEDS_STORE_GET_PRIVATE (node);
	node->priv->running = FALSE;
	node->priv->pool = NULL;
}

/**
 * feeds_store_get_channels:
 * @store: a #FeedsStore
 *
 * To retrieve list of feeds permanently saved into the store
 *
 * Return value: list of #FeedChannel found in the @store. Do not modify or
 * free it
 */
GList*
feeds_store_get_channels (FeedsStore *store)
{
	return FEEDS_STORE_GET_CLASS (store)->get_channels (store);
}

/**
 * feeds_store_get_items_by_channel:
 * @store: a #FeedsStore
 * @channel: parent feed containing required items
 *
 * To retrieve list of items saved into the store, assigned to the given
 * @channel
 *
 * Return value: list of #FeedItem found in the @store. Do not modify or free
 * it
 */
GList*
feeds_store_get_items_by_channel (FeedsStore *store, FeedChannel *channel)
{
	return FEEDS_STORE_GET_CLASS (store)->get_items_by_channel (store, channel);
}

/**
 * feeds_store_has_item:
 * @store: a #FeedsStore
 * @channel: parent feed containing required item
 * @id: unique ID to look for
 *
 * To retrieve an item into a feed, given his unique ID
 *
 * Return value: %TRUE if the specified item exists, %FALSE otherwise
 */
gboolean
feeds_store_has_item (FeedsStore *store, FeedChannel *channel, const gchar *id)
{
	return FEEDS_STORE_GET_CLASS (store)->has_item (store, channel, id);
}

/**
 * feeds_store_add_item_in_channel:
 * @store: a #FeedsStore
 * @channel: parent feed for the new item
 * @item: new item to permanently save
 *
 * To save a new #FeedItem into the @store. It performs a check to grant
 * @item is not already saved
 */
void
feeds_store_add_item_in_channel (FeedsStore *store, FeedChannel *channel, FeedItem *item)
{
	if (feeds_store_has_item (store, channel, feed_item_get_id (item)) == FALSE)
		FEEDS_STORE_GET_CLASS (store)->add_item_in_channel (store, channel, item);
}

static void
feed_fetched (FeedsPool *pool, FeedChannel *feed, GList *items, gpointer user_data)
{
	GList *iter;
	FeedItem *item;
	FeedsStore *store;

	store = (FeedsStore*) user_data;

	for (iter = items; iter; iter = g_list_next (iter)) {
		item = (FeedItem*) iter->data;
		feeds_store_add_item_in_channel (store, feed, item);
	}
}

/**
 * feeds_store_switch:
 * @store: a #FeedsStore
 * @run: %TRUE to run the @store, %FALSE to stop
 *
 * This is to permit the @store to auto-update itself: it creates an internal
 * #FeedsPool and listens for his signals, so to implement the whole loop
 * fetch-parse-save trasparently
 */
void
feeds_store_switch (FeedsStore *store, gboolean run)
{
	GList *channels;

	if (store->priv->running == run)
		return;

	if (run == TRUE) {
		if (store->priv->pool == NULL) {
			store->priv->pool = feeds_pool_new ();
			g_signal_connect (store->priv->pool, "feed-ready", G_CALLBACK (feed_fetched), store);
		}

		channels = feeds_store_get_channels (store);
		feeds_pool_listen (store->priv->pool, channels);
		feeds_pool_switch (store->priv->pool, TRUE);
	}
	else {
		if (store->priv->pool != NULL)
			feeds_pool_switch (store->priv->pool, FALSE);
	}

	store->priv->running = run;
}
