/*
 * Copyright (C) 2014, Roberto Guido <rguido@src.gnome.org>
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

#define TEST_CHANNEL_TITLE		"Test Feed"
#define TEST_CHANNEL_COPYRIGHT		"Copyright 2014 John Doe"
#define TEST_CHANNEL_DESCRIPTION	"A test fake feed"

static void
do_the_job (GrssFeedFormatter *formatter)
{
	gchar *xml;
	GrssFeedChannel *channel;
	GError *error;

	channel = grss_feed_channel_new ();
	grss_feed_channel_set_title (channel, TEST_CHANNEL_TITLE);
	grss_feed_channel_set_copyright (channel, TEST_CHANNEL_COPYRIGHT);
	grss_feed_channel_set_description (channel, TEST_CHANNEL_DESCRIPTION);

	grss_feed_formatter_set_channel (formatter, channel);
	g_object_unref (channel);

	xml = grss_feed_formatter_format (formatter);
	g_assert (xml != NULL);

	error = NULL;
	channel = grss_feed_channel_new_from_memory (xml, &error);
	g_free (xml);

	if (error != NULL) {
		printf ("%s\n", error->message);
		g_error_free (error);
		g_test_fail ();
	}

	g_assert (channel != NULL);
	g_assert_cmpstr (grss_feed_channel_get_title (channel), ==, TEST_CHANNEL_TITLE);
	g_assert_cmpstr (grss_feed_channel_get_copyright (channel), ==, TEST_CHANNEL_COPYRIGHT);
	g_assert_cmpstr (grss_feed_channel_get_description (channel), ==, TEST_CHANNEL_DESCRIPTION);

	g_object_unref (channel);
}

static void
test_format_atom ()
{
	GrssFeedAtomFormatter *formatter;

	formatter = grss_feed_atom_formatter_new ();
	do_the_job (GRSS_FEED_FORMATTER (formatter));

  grss_feed_formatter_reset (GRSS_FEED_FORMATTER (formatter));
  g_object_unref (formatter);
}

static void
test_format_rss ()
{
	GrssFeedRssFormatter *formatter;

	formatter = grss_feed_rss_formatter_new ();
	do_the_job (GRSS_FEED_FORMATTER (formatter));

  grss_feed_formatter_reset (GRSS_FEED_FORMATTER (formatter));
  g_object_unref (formatter);
}

int
main (int argc, char *argv[])
{
	g_test_init (&argc, &argv, NULL);

	g_test_add_func ("/formatter/format_atom", test_format_atom);
	g_test_add_func ("/formatter/format_rss", test_format_rss);

	return g_test_run ();
}

