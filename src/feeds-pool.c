/*
 * Copyright (C) 2009-2015, Roberto Guido <rguido@src.gnome.org>
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

#include "feeds-pool.h"
#include "utils.h"
#include "feed-parser.h"
#include "feed-marshal.h"

#define FEEDS_POOL_GET_PRIVATE(obj)     (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GRSS_FEEDS_POOL_TYPE, GrssFeedsPoolPrivate))

/**
 * SECTION: feeds-pool
 * @short_description: feeds auto-fetcher
 *
 * The #GrssFeedsPool permits to automatically "listen" for more feeds: it
 * provides to fetch them on regular intervals (as defined by
 * grss_feed_channel_get_update_interval() for each channel), parse them with
 * #GrssFeedParser, and emits signals when feeds are ready.
 */

struct _GrssFeedsPoolPrivate {
	gboolean	running;
	GList		*feeds_list;
	SoupSession	*soupsession;
	GrssFeedParser	*parser;
	guint		scheduler;
};

typedef struct {
	time_t		next_fetch;
	GrssFeedChannel	*channel;
	GrssFeedsPool	*pool;
} GrssFeedChannelWrap;

enum {
	FEED_FETCHING,
	FEED_READY,
	FEED_FAIL,
	LAST_SIGNAL
};

static guint signals [LAST_SIGNAL] = {0};

G_DEFINE_TYPE (GrssFeedsPool, grss_feeds_pool, G_TYPE_OBJECT);

static void
cancel_all_pending (GrssFeedsPool *pool)
{
	GList *iter;
	GrssFeedChannelWrap *wrap;

	if (pool->priv->feeds_list != NULL) {
		for (iter = pool->priv->feeds_list; iter; iter = g_list_next (iter)) {
			wrap = (GrssFeedChannelWrap*) iter->data;
			grss_feed_channel_fetch_cancel (wrap->channel);
		}
	}
}

static void
remove_currently_listened (GrssFeedsPool *pool)
{
	GList *iter;
	GrssFeedChannelWrap *wrap;

	soup_session_abort (pool->priv->soupsession);
	cancel_all_pending (pool);

	if (pool->priv->feeds_list != NULL) {
		for (iter = pool->priv->feeds_list; iter; iter = g_list_next (iter)) {
			wrap = (GrssFeedChannelWrap*) iter->data;
			g_object_unref (wrap->channel);
			g_free (wrap);
		}

		g_list_free (pool->priv->feeds_list);
	}
}

static void
feed_handled_cb (GrssFeedsPool *pool, GrssFeedChannel *feed, GList *items)
{
	GList *iter;

	if (items != NULL) {
		for (iter = items; iter; iter = g_list_next (iter))
			g_object_unref (iter->data);
		g_list_free (items);
	}
}

static void
grss_feeds_pool_finalize (GObject *obj)
{
	GrssFeedsPool *pool;

	pool = GRSS_FEEDS_POOL (obj);
	grss_feeds_pool_switch (pool, FALSE);
	remove_currently_listened (pool);
	g_object_unref (pool->priv->parser);
	g_object_unref (pool->priv->soupsession);
}

static void
grss_feeds_pool_class_init (GrssFeedsPoolClass *klass)
{
	GObjectClass *gobject_class;

	g_type_class_add_private (klass, sizeof (GrssFeedsPoolPrivate));

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->finalize = grss_feeds_pool_finalize;

	klass->feed_ready = feed_handled_cb;

	/**
	 * GrssFeedsPool::feed-fetching:
	 * @pool: the #GrssFeedsPool emitting the signal.
	 * @feed: the #GrssFeedChannel which is going to be fetched.
	 *
	 * Emitted when the @pool starts fetching a new #GrssFeedChannel. To be
	 * used to know the internal status of the component.
	 */
	signals [FEED_FETCHING] = g_signal_new ("feed-fetching", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, 0,
	                                        NULL, NULL, g_cclosure_marshal_VOID__OBJECT,
	                                        G_TYPE_NONE, 1, G_TYPE_OBJECT);

	/**
	 * GrssFeedsPool::feed-ready:
	 * @pool: the #GrssFeedsPool emitting the signal.
	 * @feed: the #GrssFeedChannel which has been fetched and parsed.
	 * @items: (type GLib.List) (element-type GrssFeedItem) (transfer none): list of #GrssFeedItem obtained parsing the feed.
	 *
	 * Emitted when a #GrssFeedChannel assigned to the @pool has been fetched
	 * and parsed. All parsed items are exposed in the array, with no
	 * regards about previously existing elements. @items may be NULL, if
	 * an error occurred while fetching and/or parsing. List of @items
	 * is freed, and his elements are unref'd, when signal ends.
	 */
	signals [FEED_READY] = g_signal_new ("feed-ready", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, 0,
	                                     NULL, NULL, feed_marshal_VOID__OBJECT_POINTER,
	                                     G_TYPE_NONE, 2, G_TYPE_OBJECT, G_TYPE_POINTER);

	/**
	 * GrssFeedsPool::feed-fail:
	 * @pool: the #GrssFeedsPool emitting the signal.
	 * @feed: the #GrssFeedChannel which failed to fetch or parse.
	 *
	 * Emitted when an error raises in fetching or parsing a #GrssFeedChannel
	 * assigned to the @pool.
	 */
	signals [FEED_FAIL] = g_signal_new ("feed-fail", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, 0,
	                                    NULL, NULL, feed_marshal_VOID__OBJECT,
	                                    G_TYPE_NONE, 1, G_TYPE_OBJECT);
}

static void
grss_feeds_pool_init (GrssFeedsPool *node)
{
	node->priv = FEEDS_POOL_GET_PRIVATE (node);
	memset (node->priv, 0, sizeof (GrssFeedsPoolPrivate));
	node->priv->parser = grss_feed_parser_new ();
	node->priv->soupsession = soup_session_async_new ();
}

/**
 * grss_feeds_pool_new:
 *
 * Allocates a new #GrssFeedsPool.
 *
 * Return value: a new #GrssFeedsPool.
 */
GrssFeedsPool*
grss_feeds_pool_new ()
{
	return g_object_new (GRSS_FEEDS_POOL_TYPE, NULL);
}

static void
create_listened (GrssFeedsPool *pool, GList *feeds)
{
	GList *list;
	GList *iter;
	GrssFeedChannel *feed;
	GrssFeedChannelWrap *wrap;

	list = NULL;

	for (iter = feeds; iter; iter = g_list_next (iter)) {
		feed = GRSS_FEED_CHANNEL (iter->data);

		wrap = g_new0 (GrssFeedChannelWrap, 1);
		g_object_ref (feed);
		wrap->channel = feed;
		wrap->pool = pool;
		list = g_list_prepend (list, wrap);
	}

	pool->priv->feeds_list = g_list_reverse (list);
}

/**
 * grss_feeds_pool_listen:
 * @pool: a #GrssFeedsPool.
 * @feeds: (element-type GrssFeedChannel): a list of #GrssFeedChannel.
 *
 * To set the list of feeds to be managed by the pool. The previous list, if
 * any, is invalidated. After invokation to the function, grss_feeds_pool_switch()
 * must be call to run the auto-fetching (always, also if previous state was
 * "running").
 * The list in @feeds can be freed after calling this; linked #GrssFeedChannel
 * are g_object_ref'd here.
 */
void
grss_feeds_pool_listen (GrssFeedsPool *pool, GList *feeds)
{
	gboolean original_status;

	original_status = pool->priv->running;
	grss_feeds_pool_switch (pool, FALSE);
	remove_currently_listened (pool);
	create_listened (pool, feeds);
	grss_feeds_pool_switch (pool, original_status);
}

/**
 * grss_feeds_pool_get_listened:
 * @pool: a #GrssFeedsPool.
 *
 * Returns the list of feeds currently managed by the @pool. Please consider
 * this function has to build the list that returns, and of course this is a
 * time and resources consuming task: if you only need to know how many feeds
 * are currently handled, check grss_feeds_pool_get_listened_num().
 *
 * Return value: (element-type GrssFeedChannel) (transfer container): a
 * list of #GrssFeedChannel, to be freed with g_list_free() when no longer in
 * use. Do not modify elements found in this list.
 */
GList*
grss_feeds_pool_get_listened (GrssFeedsPool *pool)
{
	GList *ret;
	GList *iter;

	ret = NULL;

	for (iter = pool->priv->feeds_list; iter; iter = g_list_next (iter))
		ret = g_list_prepend (ret, ((GrssFeedChannelWrap*)iter->data)->channel);

	return g_list_reverse (ret);
}

/**
 * grss_feeds_pool_get_listened_num:
 * @pool: a #GrssFeedsPool
 *
 * Returns number of feeds under the @pool control, as provided by
 * grss_feeds_pool_listen(). To get the complete list of those feeds, check
 * grss_feeds_pool_get_listened().
 *
 * Return value: number of feeds currently managed by the #GrssFeedsPool.
 */
int
grss_feeds_pool_get_listened_num (GrssFeedsPool *pool)
{
	if (pool->priv->feeds_list == NULL)
		return 0;
	else
		return g_list_length (pool->priv->feeds_list);
}

static void
feed_downloaded (GObject *source, GAsyncResult *res, gpointer user_data)
{
	GList *items;
	GrssFeedChannelWrap *feed;

	feed = (GrssFeedChannelWrap*) user_data;
	if (feed->pool->priv->running == FALSE)
		return;

	items = grss_feed_channel_fetch_all_finish (GRSS_FEED_CHANNEL (source), res, NULL);

	if (items != NULL)
		g_signal_emit (feed->pool, signals [FEED_READY], 0, feed->channel, items, NULL);
	else
		g_signal_emit (feed->pool, signals [FEED_FAIL], 0, feed->channel, NULL);

	feed->next_fetch = time (NULL) + (grss_feed_channel_get_update_interval (feed->channel) * 60);
}

static gboolean
fetch_feeds (gpointer data)
{
	time_t now;
	GList *iter;
	GrssFeedsPool *pool;
	GrssFeedChannelWrap *feed;

	pool = (GrssFeedsPool*) data;

	if (pool->priv->running == FALSE) {
		return FALSE;
	}

	now = time (NULL);

	for (iter = pool->priv->feeds_list; iter; iter = g_list_next (iter)) {
		feed = (GrssFeedChannelWrap*) iter->data;
		if (feed->next_fetch <= now)
			grss_feed_channel_fetch_all_async (feed->channel, feed_downloaded, feed);
	}

	return TRUE;
}

static void
run_scheduler (GrssFeedsPool *pool)
{
	int interval;
	int min_interval;
	GList *iter;
	GrssFeedChannelWrap *feed;

	if (pool->priv->feeds_list == NULL)
		return;

	min_interval = G_MAXINT;

	for (iter = pool->priv->feeds_list; iter; iter = g_list_next (iter)) {
		feed = (GrssFeedChannelWrap*) iter->data;
		interval = grss_feed_channel_get_update_interval (feed->channel);

		if (interval == 0) {
			interval = 30;
			grss_feed_channel_set_update_interval (feed->channel, interval);
		}

		if (min_interval > interval)
			min_interval = interval;

		feed->next_fetch = 0;
	}

	pool->priv->scheduler = g_timeout_add_seconds (min_interval * 60, fetch_feeds, pool);
	fetch_feeds (pool);
}

/**
 * grss_feeds_pool_switch:
 * @pool: a #GrssFeedsPool.
 * @run: %TRUE to run the pool, %FALSE to pause it.
 *
 * Permits to pause or resume the @pool fetching feeds. If @run is %TRUE, the
 * @pool starts immediately.
 */
void
grss_feeds_pool_switch (GrssFeedsPool *pool, gboolean run)
{
	if (pool->priv->running != run) {
		pool->priv->running = run;

		if (run == TRUE) {
			run_scheduler (pool);
		}
		else {
			if (pool->priv->scheduler != 0)
				g_source_remove (pool->priv->scheduler);
			cancel_all_pending (pool);
		}
	}
}

/**
 * grss_feeds_pool_get_session:
 * @pool: a #GrssFeedsPool.
 *
 * To access the internal #SoupSession used by the @pool to fetch items.
 *
 * Return value: (transfer none): instance of #SoupSession. Do not free it.
 */
SoupSession*
grss_feeds_pool_get_session (GrssFeedsPool *pool)
{
	return pool->priv->soupsession;
}
