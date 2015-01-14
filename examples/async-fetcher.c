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

#include <libgrss.h>

static int i;
static GMainLoop *loop;

static gboolean mark_time (gpointer useless)
{
	printf ("%d\n", i++);

	if (i >= 10)
		g_main_loop_quit (loop);

	return TRUE;
}

static void
print_items (GObject *source, GAsyncResult *res, gpointer useless)
{
	GList *items;
	GList *iter;
	GError *error;
	GrssFeedChannel *channel;
	GrssFeedItem *it;

	error = NULL;
	channel = GRSS_FEED_CHANNEL (source);
	items = grss_feed_channel_fetch_all_finish (channel, res, &error);

	if (items == NULL) {
		printf ("%s\n", error->message);
	}
	else {
		for (iter = items; iter; iter = g_list_next (iter)) {
			it = (GrssFeedItem*) iter->data;
			printf ("%s\n", grss_feed_item_get_title (it));
		}
	}
}

static gboolean do_work (gpointer useless)
{
	GrssFeedChannel *feed;

	i = 0;
	g_timeout_add (100, mark_time, NULL);

	feed = grss_feed_channel_new_with_source ("http://rss.slashdot.org/Slashdot/slashdot");
	grss_feed_channel_fetch_all_async (feed, print_items, NULL);

	return FALSE;
}

int main ()
{
	loop = g_main_loop_new (NULL, FALSE);
	g_idle_add (do_work, NULL);

	g_main_run (loop);
	exit (0);
}
