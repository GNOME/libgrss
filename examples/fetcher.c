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

#include <libgrss.h>

static void feed_fetched (GrssFeedsPool *pool, GrssFeedChannel *feed, GList *items)
{
	GList *iter;
	GrssFeedItem *item;

	if (items == NULL) {
		printf ("Error while fetching %s\n", grss_feed_channel_get_title (feed));
		return;
	}

	printf ("Fetched from %s\n", grss_feed_channel_get_title (feed));

	for (iter = items; iter; iter = g_list_next (iter)) {
		item = (GrssFeedItem*) iter->data;
		printf ("\t\t%s\n", grss_feed_item_get_title (item));
	}

	printf ("\n\n");
}

int main ()
{
	register int i;
	gchar *example_feeds [] = {
		"http://rss.slashdot.org/Slashdot/slashdot",
		"http://planet.gnome.org/atom.xml",
		"http://news.gnome.org/atom.xml",
		"http://lwn.net/headlines/rss",
		NULL
	};
	GList *iter;
	GList *list;
	GrssFeedChannel *feed;
	GrssFeedsPool *pool;

	g_type_init ();
	g_thread_init (NULL);

	list = NULL;

	for (i = 0; example_feeds [i] != NULL; i++) {
		feed = grss_feed_channel_new ();
		grss_feed_channel_set_source (feed, example_feeds [i]);
		grss_feed_channel_set_update_interval (feed, i + 1);
		list = g_list_prepend (list, feed);
	}

	list = g_list_reverse (list);

	pool = grss_feeds_pool_new ();
	grss_feeds_pool_listen (pool, list);
	grss_feeds_pool_switch (pool, TRUE);

	g_signal_connect (pool, "feed-ready", G_CALLBACK (feed_fetched), NULL);

	g_main_run (g_main_loop_new (NULL, FALSE));

	for (iter = list; iter; iter = g_list_next (iter))
		g_object_unref (iter->data);

	g_object_unref (pool);
	exit (0);
}
