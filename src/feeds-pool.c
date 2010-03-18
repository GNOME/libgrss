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

#include "feeds-pool.h"
#include "utils.h"
#include "feed-parser.h"
#include "feed-marshal.h"

#define FEEDS_POOL_GET_PRIVATE(obj)     (G_TYPE_INSTANCE_GET_PRIVATE ((obj), FEEDS_POOL_TYPE, FeedsPoolPrivate))

/**
 * SECTION: feeds-pool
 * @short_description: feeds auto-fetcher
 *
 * The #FeedsPool permits to automatically "listen" for more feeds: it
 * provides to fetch them on regular intervals (as defined by
 * feed_channel_get_update_interval() for each channel), parse them with
 * #FeedParser, and emits signals when feeds are ready
 */

struct _FeedsPoolPrivate {
	gboolean	running;
	GList		*feeds_list;
	SoupSession	*soupsession;
	FeedParser	*parser;
	guint		scheduler;
};

typedef struct {
	time_t		next_fetch;
	FeedChannel	*channel;
	FeedsPool	*pool;
} FeedChannelWrap;

enum {
	FEED_FETCHING,
	FEED_READY,
	LAST_SIGNAL
};

static guint signals [LAST_SIGNAL] = {0};

G_DEFINE_TYPE (FeedsPool, feeds_pool, G_TYPE_OBJECT);

static void
remove_currently_listened (FeedsPool *pool)
{
	GList *iter;
	FeedChannelWrap *wrap;

	if (pool->priv->feeds_list != NULL) {
		for (iter = pool->priv->feeds_list; iter; iter = g_list_next (iter)) {
			wrap = (FeedChannelWrap*) iter->data;
			g_object_unref (wrap->channel);
			g_free (wrap);
		}

		g_list_free (pool->priv->feeds_list);
	}
}

static void
feed_handled_cb (FeedsPool *pool, FeedChannel *feed, GList *items)
{
	GList *iter;

	if (items != NULL) {
		for (iter = items; iter; iter = g_list_next (iter))
			g_object_unref (iter->data);
		g_list_free (items);
	}
}

static void
feeds_pool_finalize (GObject *obj)
{
	FeedsPool *pool;

	pool = FEEDS_POOL (obj);
	feeds_pool_switch (pool, FALSE);
	remove_currently_listened (pool);
	g_object_unref (pool->priv->parser);
	g_object_unref (pool->priv->soupsession);
}

static void
feeds_pool_class_init (FeedsPoolClass *klass)
{
	GObjectClass *gobject_class;

	g_type_class_add_private (klass, sizeof (FeedsPoolPrivate));

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->finalize = feeds_pool_finalize;

	klass->feed_ready = feed_handled_cb;

	/**
	 * FeedsPool::feed-fetching:
	 * @pool: the #FeedsPool emitting the signal
	 * @feed: the #FeedChannel which is going to be fetched
	 *
	 * Emitted when the @pool starts fetching a new #FeedChannel. To be
	 * used to know the internal status of the component
	 */
	signals [FEED_FETCHING] = g_signal_new ("feed-fetching", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, 0,
	                                        NULL, NULL, g_cclosure_marshal_VOID__OBJECT,
	                                        G_TYPE_NONE, 1, G_TYPE_OBJECT);

	/**
	 * FeedsPool::feed-ready:
	 * @pool: the #FeedsPool emitting the signal
	 * @feed: the #FeedChannel which has been fetched and parsed
	 * @items: list of #FeedItem obtained parsing the feed
	 *
	 * Emitted when a #FeedChannel assigned to the @pool has been fetched
	 * and parsed. If @items may be NULL, if an error occourred while
	 * fetching and/or parsing. List of @items is freed, and his elements
	 * are unref'd, when signal ends
	 */
	signals [FEED_READY] = g_signal_new ("feed-ready", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, 0,
	                                     NULL, NULL, feed_marshal_VOID__OBJECT_POINTER,
	                                     G_TYPE_NONE, 2, G_TYPE_OBJECT, G_TYPE_POINTER);
}

static void
feeds_pool_init (FeedsPool *node)
{
	node->priv = FEEDS_POOL_GET_PRIVATE (node);
	memset (node->priv, 0, sizeof (FeedsPoolPrivate));
	node->priv->parser = feed_parser_new ();
	node->priv->soupsession = soup_session_async_new ();
}

/**
 * feeds_pool_new:
 *
 * Allocates a new #FeedsPool
 *
 * Return value: a new #FeedsPool
 */
FeedsPool*
feeds_pool_new ()
{
	return g_object_new (FEEDS_POOL_TYPE, NULL);
}

static void
create_listened (FeedsPool *pool, GList *feeds)
{
	GList *list;
	GList *iter;
	FeedChannel *feed;
	FeedChannelWrap *wrap;

	list = NULL;

	for (iter = feeds; iter; iter = g_list_next (iter)) {
		feed = FEED_CHANNEL (iter->data);

		wrap = g_new0 (FeedChannelWrap, 1);
		g_object_ref (feed);
		wrap->channel = feed;
		wrap->pool = pool;
		list = g_list_prepend (list, wrap);
	}

	pool->priv->feeds_list = g_list_reverse (list);
}

/**
 * feeds_pool_listen:
 * @pool: a #FeedsPool
 * @feeds: a list of #FeedChannel
 *
 * To set the list of feeds to be managed by the pool. The previous list, if
 * any, is invalidated. After invokation to the function, feeds_pool_switch()
 * must be call to run the auto-fetching (always, also if previous state was
 * "running").
 * The list in @feeds can be freed after calling this; linked #FeedChannel
 * are g_object_ref'd here
 */
void
feeds_pool_listen (FeedsPool *pool, GList *feeds)
{
	gboolean original_status;

	original_status = pool->priv->running;
	feeds_pool_switch (pool, FALSE);
	remove_currently_listened (pool);
	create_listened (pool, feeds);
	feeds_pool_switch (pool, original_status);
}

/**
 * feeds_pool_get_listened:
 * @pool: a #FeedsPool
 *
 * Returns the list of feeds currently managed by the @pool. Please consider
 * this function has to build the list that returns, and of course this is a
 * time and resources consuming task: if you only need to know how many feeds
 * are currently handled, check feeds_pool_get_listened_num()
 *
 * Return value: a list of #FeedChannel, to be freed with g_list_free() when
 * no longer in use. Do not modify elements found in this list
 */
GList*
feeds_pool_get_listened (FeedsPool *pool)
{
	GList *ret;
	GList *iter;

	ret = NULL;

	for (iter = pool->priv->feeds_list; iter; iter = g_list_next (iter))
		ret = g_list_prepend (ret, ((FeedChannelWrap*)iter->data)->channel);

	return g_list_reverse (ret);
}

/**
 * feeds_pool_get_listened:
 * @pool: a #FeedsPool
 *
 * Returns number of feeds under the @pool control, as provided by
 * feeds_pool_listen(). To get the complete list of those feeds, check
 * feeds_pool_get_listened()
 *
 * Return value: number of feeds currently managed by the #FeedsPool
 */
int
feeds_pool_get_listened_num (FeedsPool *pool)
{
	if (pool->priv->feeds_list == NULL)
		return 0;
	else
		return g_list_length (pool->priv->feeds_list);
}

static void
feed_downloaded (SoupSession *session, SoupMessage *msg, gpointer user_data)
{
	guint status;
	GList *items;
	GError *error;
	xmlDocPtr doc;
	FeedChannelWrap *feed;

	feed = (FeedChannelWrap*) user_data;
	if (feed->pool->priv->running == FALSE)
		return;

	items = NULL;
	feed->next_fetch = time (NULL) + (feed_channel_get_update_interval (feed->channel) * 60);
	g_object_get (msg, "status-code", &status, NULL);

	if (status < 200 || status > 299) {
		g_warning ("Unable to download from %s", feed_channel_get_source (feed->channel));
	}
	else {
		doc = feed_content_to_xml ((const gchar*) msg->response_body->data, msg->response_body->length);

		if (doc != NULL) {
			error = NULL;
			items = feed_parser_parse (feed->pool->priv->parser, feed->channel, doc, &error);
			xmlFreeDoc (doc);

			if (items == NULL && error) {
				g_warning ("Unable to parse feed at %s: %s", feed_channel_get_source (feed->channel), error->message);
				g_error_free (error);
			}
		}
	}

	g_signal_emit (feed->pool, signals [FEED_READY], 0, feed->channel, items, NULL);
}

static void
fetch_feed (FeedChannelWrap *feed)
{
	SoupMessage *msg;

	g_signal_emit (feed->pool, signals [FEED_FETCHING], 0, feed->channel, NULL);
	msg = soup_message_new ("GET", feed_channel_get_source (feed->channel));
	soup_session_queue_message (feed->pool->priv->soupsession, msg, feed_downloaded, feed);
}

static gboolean
fetch_feeds (gpointer data)
{
	time_t now;
	GList *iter;
	FeedsPool *pool;
	FeedChannelWrap *feed;

	pool = (FeedsPool*) data;

	if (pool->priv->running == FALSE) {
		return FALSE;
	}

	now = time (NULL);

	for (iter = pool->priv->feeds_list; iter; iter = g_list_next (iter)) {
		feed = (FeedChannelWrap*) iter->data;
		if (feed->next_fetch <= now)
			fetch_feed (feed);
	}

	return TRUE;
}

static void
run_scheduler (FeedsPool *pool)
{
	int interval;
	int min_interval;
	GList *iter;
	FeedChannelWrap *feed;

	if (pool->priv->feeds_list == NULL)
		return;

	min_interval = G_MAXINT;

	for (iter = pool->priv->feeds_list; iter; iter = g_list_next (iter)) {
		feed = (FeedChannelWrap*) iter->data;
		interval = feed_channel_get_update_interval (feed->channel);

		if (interval == 0) {
			interval = 30;
			feed_channel_set_update_interval (feed->channel, interval);
		}

		if (min_interval > interval)
			min_interval = interval;

		feed->next_fetch = 0;
	}

	pool->priv->scheduler = g_timeout_add_seconds (min_interval * 60, fetch_feeds, pool);
	fetch_feeds (pool);
}

/**
 * feeds_pool_switch:
 * @pool: a #FeedsPool
 * @run: TRUE to run the pool, FALSE to pause it
 *
 * Permits to pause or resume the @pool fetching feeds. If @run is #TRUE, the
 * @pool starts immediately
 */
void
feeds_pool_switch (FeedsPool *pool, gboolean run)
{
	if (pool->priv->running != run) {
		pool->priv->running = run;

		if (run == TRUE) {
			run_scheduler (pool);
		}
		else {
			if (pool->priv->scheduler != 0)
				g_source_remove (pool->priv->scheduler);
		}
	}
}

/**
 * feeds_pool_get_session:
 * @pool: a #FeedsPool
 *
 * To access the internal #SoupSession used by the @pool to fetch items
 *
 * Return value: istance of #SoupSession. Do not free it
 */
SoupSession*
feeds_pool_get_session (FeedsPool *pool)
{
	return pool->priv->soupsession;
}
