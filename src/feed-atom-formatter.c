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

#include "feed-atom-formatter.h"
#include "utils.h"

G_DEFINE_TYPE (GrssFeedAtomFormatter, grss_feed_atom_formatter, GRSS_FEED_FORMATTER_TYPE);

static gchar*
feed_atom_formatter_format (GrssFeedFormatter *formatter)
{
	const gchar *str;
	gchar *formatted;
	time_t date;
	GList *iter;
	GList *items;
	const GList *list;
	GString *text;
	GrssFeedChannel *channel;
	GrssFeedItem *item;

	text = g_string_new ("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<feed xmlns=\"http://www.w3.org/2005/Atom\">\n");

	channel = grss_feed_formatter_get_channel (formatter);
	items = grss_feed_formatter_get_items (formatter);

	if (channel != NULL) {
		str = grss_feed_channel_get_title (channel);
		if (str != NULL)
			g_string_append_printf (text, "\t<title>%s</title>\n", str);

		str = grss_feed_channel_get_description (channel);
		if (str != NULL)
			g_string_append_printf (text, "\t<subtitle>%s</subtitle>\n", str);

		str = grss_feed_channel_get_homepage (channel);
		if (str != NULL)
			g_string_append_printf (text, "\t<link href=\"%s\" />\n", str);

		str = grss_feed_channel_get_copyright (channel);
		if (str != NULL)
			g_string_append_printf (text, "\t<rights>%s</rights>\n", str);

		str = grss_feed_channel_get_editor (channel);
		if (str != NULL)
			g_string_append_printf (text, "\t<author>%s</author>\n", str);

		str = grss_feed_channel_get_generator (channel);
		if (str != NULL)
			g_string_append_printf (text, "\t<generator>%s</generator>\n", str);

		list = grss_feed_channel_get_contributors (channel);
		while (list != NULL) {
			g_string_append_printf (text, "\t<contributor>%s</contributor>\n", (gchar*) list->data);
			list = list->next;
		}

		date = grss_feed_channel_get_update_time (channel);
		if (date == 0)
			date = grss_feed_channel_get_publish_time (channel);
		formatted = date_to_ISO8601 (date);
		g_string_append_printf (text, "\t<updated>%s</updated>\n", formatted);
		g_free (formatted);

		str = grss_feed_channel_get_icon (channel);
		if (str != NULL)
			g_string_append_printf (text, "\t<icon>%s</icon>\n", str);

		str = grss_feed_channel_get_image (channel);
		if (str != NULL)
			g_string_append_printf (text, "\t<logo>%s</logo>\n", str);

		for (iter = items; iter; iter = iter->next) {
			item = iter->data;

			g_string_append (text, "\t<entry>\n");

			str = grss_feed_item_get_title (item);
			if (str != NULL)
				g_string_append_printf (text, "\t\t<title>%s</title>\n", str);

			str = grss_feed_item_get_id (item);
			if (str != NULL)
				g_string_append_printf (text, "\t\t<id>%s</id>\n", str);

			str = grss_feed_item_get_source (item);
			if (str != NULL)
				g_string_append_printf (text, "\t\t<link href=\"%s\" />\n", str);

			str = grss_feed_item_get_description (item);
			if (str != NULL)
				g_string_append_printf (text, "\t\t<summary>%s</summary>\n", str);

			str = grss_feed_item_get_author (item);
			if (str != NULL)
				g_string_append_printf (text, "\t\t<author>%s</author>\n", str);

			str = grss_feed_item_get_copyright (item);
			if (str != NULL)
				g_string_append_printf (text, "\t\t<rights>%s</rights>\n", str);

			list = grss_feed_item_get_contributors (item);
			while (list != NULL) {
				g_string_append_printf (text, "\t\t<contributor>%s</contributor>\n", (gchar*) list->data);
				list = list->next;
			}

			date = grss_feed_item_get_publish_time (item);
			formatted = date_to_ISO8601 (date);
			g_string_append_printf (text, "\t\t<published>%s</published>\n", formatted);
			g_free (formatted);

			g_string_append (text, "\t</entry>\n");
		}
	}

	g_string_append (text, "</feed>");
	return g_string_free (text, FALSE);
}

static void
grss_feed_atom_formatter_class_init (GrssFeedAtomFormatterClass *klass)
{
	GrssFeedFormatterClass *formater_class = GRSS_FEED_FORMATTER_CLASS (klass);

	formater_class->format = feed_atom_formatter_format;
}

static void
grss_feed_atom_formatter_init (GrssFeedAtomFormatter *object)
{
}

GrssFeedAtomFormatter*
grss_feed_atom_formatter_new ()
{
	GrssFeedAtomFormatter *formatter;

	formatter = g_object_new (GRSS_FEED_ATOM_FORMATTER_TYPE, NULL);
	return formatter;
}

