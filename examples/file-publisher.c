/*
 * Copyright (C) 2012, Roberto Guido <rguido@src.gnome.org>
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

int main ()
{
	GError *error;
	GList *iter;
	GList *list;
	GrssFeedChannel *feed;
	GrssFeedItem *item;
	GrssFeedsPublisher *publisher;

	list = NULL;

	feed = grss_feed_channel_new ();
	grss_feed_channel_set_title (feed, "Test Feed");
	grss_feed_channel_set_homepage (feed, "http://example.com");
	grss_feed_channel_set_description (feed, "A test feed");
	grss_feed_channel_set_publish_time (feed, time (NULL));

	item = grss_feed_item_new (feed);
	grss_feed_item_set_title (item, "A big news!");
	grss_feed_item_set_description (item, "Something incredible happening!");
	grss_feed_item_set_source (item, "http://slashdot.org");
	list = g_list_prepend (list, item);

	item = grss_feed_item_new (feed);
	grss_feed_item_set_title (item, "libgrss released");
	grss_feed_item_set_description (item, "A new version of the best Glib-based RSS management library has been released");
	grss_feed_item_set_source (item, "http://live.gnome.org/Libgrss");
	list = g_list_prepend (list, item);

	item = grss_feed_item_new (feed);
	grss_feed_item_set_title (item, "A new commit in libgrss");
	grss_feed_item_set_description (item, "Some commit happens on libgrss git repository");
	grss_feed_item_set_source (item, "http://git.gnome.org/browse/libgrss");
	list = g_list_prepend (list, item);

	error = NULL;
	publisher = grss_feeds_publisher_new ();
	if (grss_feeds_publisher_publish_file (publisher, feed, list, "file:///tmp/test.xml", &error) == FALSE) {
		printf ("Unable to write file: %s\n", error->message);
		g_error_free (error);
	}

	for (iter = list; iter; iter = g_list_next (iter))
		g_object_unref (iter->data);

	g_list_free (list);
	g_object_unref (feed);
	exit (0);
}
