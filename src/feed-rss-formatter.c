/*
 * Copyright (C) 2014, Roberto Guido <rguido@src.gnome.org>
 * Copyright (C) 2015 Igor Gnatenko <ignatenko@src.gnome.org>
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

#include "feed-rss-formatter.h"
#include "utils.h"

/**
 * SECTION: feed-rss-formatter
 * @short_description: able to format a feed in RSS
 *
 * #GrssFeedRssFormatter is an implementation on #GrssFeedFormatter able to
 * handle RSS format
 */

G_DEFINE_TYPE (GrssFeedRssFormatter, grss_feed_rss_formatter, GRSS_FEED_FORMATTER_TYPE);

static gchar*
feed_rss_formatter_format (GrssFeedFormatter *formatter)
{
	const gchar *str;
	const gchar *title;
	const gchar *link;
	gchar *formatted;
	time_t date;
	GList *iter;
	GList *items;
        GrssPerson *person;
	const GList *list;
	GString *text;
	GrssFeedChannel *channel;
	GrssFeedItem *item;

	/*
		Based on http://cyber.law.harvard.edu/rss/examples/rss2sample.xml
	*/

	text = g_string_new ("<?xml version=\"1.0\"?>\n<rss version=\"2.0\">\n");

	channel = grss_feed_formatter_get_channel (formatter);
	items = grss_feed_formatter_get_items (formatter);

	if (channel != NULL) {
		g_string_append_printf (text, "<channel>\n");

		title = grss_feed_channel_get_title (channel);
		if (title != NULL)
			g_string_append_printf (text, "\t<title>%s</title>\n", title);
		else
			title = "";

		str = grss_feed_channel_get_description (channel);
		if (str != NULL)
			g_string_append_printf (text, "\t<description>%s</description>\n", str);

		link = grss_feed_channel_get_homepage (channel);
		if (link != NULL)
			g_string_append_printf (text, "\t<link>%s</link>\n", link);
		else
			link = "";

		str = grss_feed_channel_get_copyright (channel);
		if (str != NULL)
			g_string_append_printf (text, "\t<copyright>%s</copyright>\n", str);

		person = grss_feed_channel_get_editor (channel);
		if (person != NULL)
			// TODO: implement handling additional attrs
			g_string_append_printf (text, "\t<managingEditor>%s</managingEditor>\n", grss_person_get_name (person));

		str = grss_feed_channel_get_generator (channel);
		if (str != NULL)
			g_string_append_printf (text, "\t<generator>%s</generator>\n", str);

		date = grss_feed_channel_get_update_time (channel);
		formatted = date_to_ISO8601 (date);
		g_string_append_printf (text, "\t<pubDate>%s</pubDate>\n", formatted);
		g_free (formatted);

		str = grss_feed_channel_get_image (channel);
		if (str != NULL) {
			g_string_append_printf (text, "\t<image>\n");
			g_string_append_printf (text, "\t\t<title>%s</title>\n", title);
			g_string_append_printf (text, "\t\t<url>%s</url>\n", str);
			g_string_append_printf (text, "\t\t<link>%s</link>\n", link);
			g_string_append_printf (text, "\t</image>\n");
		}

		for (iter = items; iter; iter = iter->next) {
			item = iter->data;

			g_string_append (text, "\t<item>\n");

			str = grss_feed_item_get_title (item);
			if (str != NULL)
				g_string_append_printf (text, "\t\t<title>%s</title>\n", str);

			str = grss_feed_item_get_id (item);
			if (str != NULL)
				g_string_append_printf (text, "\t\t<guid>%s</guid>\n", str);

			str = grss_feed_item_get_source (item);
			if (str != NULL)
				g_string_append_printf (text, "\t\t<link>%s</link>\n", str);

			str = grss_feed_item_get_description (item);
			if (str != NULL)
				g_string_append_printf (text, "\t\t<description>%s</description>\n", str);

			person = grss_feed_item_get_author (item);
			if (person != NULL)
				// FIXME: implement adding email and uri
				g_string_append_printf (text, "\t\t<author>%s</author>\n", grss_person_get_name (person));

			date = grss_feed_item_get_publish_time (item);
			formatted = date_to_ISO8601 (date);
			g_string_append_printf (text, "\t\t<pubDate>%s</pubDate>\n", formatted);
			g_free (formatted);

			g_string_append (text, "\t</item>\n");
		}
	}

	g_string_append (text, "</channel>\n</rss>");
	return g_string_free (text, FALSE);
}

static void
grss_feed_rss_formatter_class_init (GrssFeedRssFormatterClass *klass)
{
	GrssFeedFormatterClass *formater_class = GRSS_FEED_FORMATTER_CLASS (klass);

	formater_class->format = feed_rss_formatter_format;
}

static void
grss_feed_rss_formatter_init (GrssFeedRssFormatter *object)
{
}

GrssFeedRssFormatter*
grss_feed_rss_formatter_new ()
{
	GrssFeedRssFormatter *formatter;

	formatter = g_object_new (GRSS_FEED_RSS_FORMATTER_TYPE, NULL);
	return formatter;
}

