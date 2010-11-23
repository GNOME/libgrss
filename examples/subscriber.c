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

static void
item_fetched (GrssFeedsSubscriber *subscriber, GrssFeedChannel *feed, GrssFeedItem *item)
{
	printf ("Received from %s\n", grss_feed_channel_get_title (feed));
	printf ("\t\t%s\n\n\n", grss_feed_item_get_title (item));
}

static gboolean
start_subscribe (gpointer data)
{
	GrssFeedsSubscriber *sub;

	sub = (GrssFeedsSubscriber*) data;
	grss_feeds_subscriber_switch (sub, TRUE);
	printf ("Ready, now waiting for events.\n\n");
	return FALSE;
}

int
main ()
{
	register int i;
	gboolean ok;
	gchar *example_feeds [] = {
		"http://rss.slashdot.org/Slashdot/slashdot",
		"http://feeds.feedburner.com/Techcrunch",
		NULL
	};
	GList *iter;
	GList *list;
	GrssFeedChannel *feed;
	GrssFeedsSubscriber *sub;

	g_type_init ();

	/*
		This is required for correct behaviour in libsoup
	*/
	g_thread_init (NULL);

	list = NULL;
	printf ("Initing, please wait...\n");
	printf ("... fetching feeds...\n");

	for (i = 0; example_feeds [i] != NULL; i++) {
		feed = grss_feed_channel_new ();
		grss_feed_channel_set_source (feed, example_feeds [i]);
		ok = grss_feed_channel_fetch (feed);

		if (ok == FALSE) {
			g_warning ("Unable to fetch feed at %s", example_feeds [i]);
			g_object_unref (feed);
		}
		else {
			list = g_list_prepend (list, feed);
		}
	}

	if (list == NULL)
		exit (0);

	printf ("... feeds analyzed...\n");
	list = g_list_reverse (list);

	sub = grss_feeds_subscriber_new ();

	ok = grss_feeds_subscriber_listen (sub, list);
	g_list_free (list);

	if (ok == FALSE) {
		g_warning ("Unable to listen for notifications\n");
		g_object_unref (sub);
		exit (1);
	}

	g_idle_add (start_subscribe, sub);
	g_signal_connect (sub, "notification-received", G_CALLBACK (item_fetched), NULL);

	g_main_run (g_main_loop_new (NULL, FALSE));

	for (iter = list; iter; iter = g_list_next (iter))
		g_object_unref (iter->data);

	g_object_unref (sub);
	exit (0);
}
