/*
 * Copyright (C) 2010, Roberto Guido <rguido@src.gnome.org>
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

static void print_usage (gchar *program_name)
{
	printf ("%s - sample OPML and XOXO parser\n", program_name);
	printf ("Usage: %s <file>\n", program_name);
	printf ("\t<file>\tan OPML or XOXO file\n");
	printf ("\n");
}

int
main (int argc, char **argv)
{
	GList *iter;
	GList *list;
	GError *error;
	GrssFeedChannel *channel;
	GrssFeedsGroup *parser;

	if (argc < 2) {
		print_usage (argv [0]);
		exit (1);
	}

	g_type_init ();
	g_thread_init (NULL);

	parser = grss_feeds_group_new ();

	error = NULL;
	list = grss_feeds_group_parse_file (parser, argv [1], &error);

	if (list == NULL && error != NULL) {
		printf ("Unable to parse file %s: %s\n", argv [1], error->message);
		g_error_free (error);
		exit (1);
	}

	for (iter = list; iter; iter = iter->next) {
		channel = iter->data;
		printf ("\t%s - %s\n", grss_feed_channel_get_title (channel), grss_feed_channel_get_source (channel));
		g_object_unref (channel);
	}

	g_list_free (list);

	g_object_unref (parser);
	exit (0);
}
