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

static void
test_parse_invalid ()
{
	gchar *test_xml;
	GrssFeedChannel *channel;

	test_xml = "This is an invalid document";
	channel = grss_feed_channel_new_from_memory (test_xml, NULL);

	g_assert (channel == NULL);
}

static void
test_parse_valid_rss ()
{
	GrssFeedChannel *channel;

	channel = grss_feed_channel_new_from_file ("test.rss.xml", NULL);

	g_assert (channel != NULL);
	g_assert_cmpstr (grss_feed_channel_get_homepage (channel), ==, "http://slashdot.org/");
	g_assert_cmpstr (grss_feed_channel_get_title (channel), ==, "Slashdot");
	g_assert_cmpstr (grss_feed_channel_get_language (channel), ==, "en-us");
	g_assert_cmpstr (grss_person_get_name (grss_feed_channel_get_editor (channel)), ==, "help@slashdot.org");

	g_object_unref (channel);
}

static void
test_parse_valid_atom ()
{
	GrssFeedChannel *channel;

	channel = grss_feed_channel_new_from_file ("test.atom.xml", NULL);

	g_assert (channel != NULL);
	g_assert_cmpstr (grss_feed_channel_get_title (channel), ==, "Planet GNU");

	g_object_unref (channel);
}

int
main (int argc, char *argv[])
{
	g_test_init (&argc, &argv, NULL);

	g_test_add_func ("/channel/parse_invalid", test_parse_invalid);
	g_test_add_func ("/channel/parse_valid_rss", test_parse_valid_rss);
	g_test_add_func ("/channel/parse_valid_atom", test_parse_valid_atom);

	return g_test_run ();
}

