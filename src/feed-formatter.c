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

#include "feed-formatter.h"
#include "utils.h"
#include "feed-item.h"
#include "feed-channel.h"

#define FEED_FORMATTER_GET_PRIVATE(obj)	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), GRSS_FEED_FORMATTER_TYPE, GrssFeedFormatterPrivate))

/**
 * SECTION: feed-formatter
 * @short_description: abstract class to format feeds in plain rappresentation
 *
 * #GrssFeedFormatter is a class abstracting the ability to format a
 * #GrssFeedChannel and related #GrssFeedItems into a plain text string, usually
 * in XML. Subclasses implement the effective required format (e.g. RSS,
 * Atom...)
 */

G_DEFINE_ABSTRACT_TYPE (GrssFeedFormatter, grss_feed_formatter, G_TYPE_OBJECT);

struct _GrssFeedFormatterPrivate {
	GrssFeedChannel	*channel;
	GList		*items;
};

static void
grss_feed_formatter_class_init (GrssFeedFormatterClass *klass)
{
	g_type_class_add_private (klass, sizeof (GrssFeedFormatterPrivate));
}

static void
grss_feed_formatter_init (GrssFeedFormatter *node)
{
	node->priv = FEED_FORMATTER_GET_PRIVATE (node);
	node->priv->channel = NULL;
	node->priv->items = NULL;
}

/**
 * grss_feed_formatter_set_channel:
 * @formatter: a #GrssFeedFormatter.
 * @channel: the reference #GrssFeedChannel for the @formatter.
 *
 * Inits the #GrssFeedFormatter with the given @channel. A #GrssFeedFormatter
 * can format a single #GrssFeedChannel each time, but may be reused by calling
 * grss_feed_formatter_reset()
 */
void
grss_feed_formatter_set_channel (GrssFeedFormatter *formatter, GrssFeedChannel *channel)
{
	if (formatter->priv->channel != NULL)
		g_object_unref (formatter->priv->channel);

	formatter->priv->channel = channel;
	g_object_ref (channel);
}

/**
 * grss_feed_formatter_get_channel:
 * @formatter: a #GrssFeedFormatter.
 *
 * Gets the current #GrssFeedChannel assigned to the @formatter.
 *
 * Return value: (transfer none): a #GrssFeedChannel, or %NULL if none has been
 * assigned.
 */
GrssFeedChannel*
grss_feed_formatter_get_channel (GrssFeedFormatter *formatter)
{
	return formatter->priv->channel;
}

/**
 * grss_feed_formatter_add_item:
 * @formatter: a #GrssFeedFormatter.
 * @item: a #GrssFeedItem to add into the @formatter.
 *
 * Adds a single #GrssFeedItem in the @formatter.
 */
void
grss_feed_formatter_add_item (GrssFeedFormatter *formatter, GrssFeedItem *item)
{
	g_object_ref (item);

	if (formatter->priv->items == NULL)
		formatter->priv->items = g_list_prepend (formatter->priv->items, item);
	else
		formatter->priv->items = g_list_append (formatter->priv->items, item);
}

/**
 * grss_feed_formatter_add_items:
 * @formatter: a #GrssFeedFormatter.
 * @items: (element-type GrssFeedItem): a list of #GrssFeedItems to add into
 *         the @formatter.
 *
 * Adds a list of #GrssFeedItems in the @formatter.
 */
void
grss_feed_formatter_add_items (GrssFeedFormatter *formatter, GList *items)
{
	GList *copy;

	copy = g_list_copy_deep (items, (GCopyFunc) g_object_ref, NULL);

	if (formatter->priv->items == NULL)
		formatter->priv->items = copy;
	else
		formatter->priv->items = g_list_concat (formatter->priv->items, copy);
}

/**
 * grss_feed_formatter_get_items:
 * @formatter: a #GrssFeedFormatter.
 *
 * Gets the current #GrssFeedItems assigned to the @formatter.
 *
 * Return value:  (element-type GrssFeedItem) (transfer none): a list of
 * #GrssFeedItems, or %NULL if none has been assigned.
 */
GList*
grss_feed_formatter_get_items (GrssFeedFormatter *formatter)
{
	return formatter->priv->items;
}

/**
 * grss_feed_formatter_reset:
 * @formatter: a #GrssFeedFormatter.
 *
 * Resets the status of the #GrssFeedFormatter, cleaning up the assigned
 * #GrssFeedChannel and related #GrssFeedItems. This way @formatter is ready to
 * be used again with new data.
 */
void
grss_feed_formatter_reset (GrssFeedFormatter *formatter)
{
	if (formatter->priv->channel != NULL) {
		g_object_unref (formatter->priv->channel);
		formatter->priv->channel = NULL;
	}

	if (formatter->priv->items != NULL) {
		g_list_free_full (formatter->priv->items, g_object_unref);
		formatter->priv->items = NULL;
	}
}

/**
 * grss_feed_formatter_format:
 * @formatter: a #GrssFeedFormatter.
 *
 * Formats the assigned #GrssFeedChannel and #GrssFeedItems into a plain text
 * string, accordly to the current #GrssFeedFormatter instance.
 *
 * Return value: (transfer full): a string containing the plain text
 * rappresentation of the given channel containing the given items.
 */
gchar*
grss_feed_formatter_format (GrssFeedFormatter *formatter)
{
	return GRSS_FEED_FORMATTER_GET_CLASS (formatter)->format (formatter);
}

