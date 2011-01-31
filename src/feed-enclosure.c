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

#include "utils.h"
#include "feed-enclosure.h"

#define FEED_ENCLOSURE_GET_PRIVATE(obj)     (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GRSS_FEED_ENCLOSURE_TYPE, GrssFeedEnclosurePrivate))

/**
 * SECTION: feed-enclosure
 * @short_description: a component attached to an item
 *
 * #GrssFeedEnclosure describes an external element embedded into a #GrssFeedItem: it
 * may be an image, a video of other kind of file to be presented with the
 * parent item
 */

struct _GrssFeedEnclosurePrivate {
	gchar	*url;
	gchar	*type;
	gsize	length;
};

G_DEFINE_TYPE (GrssFeedEnclosure, grss_feed_enclosure, G_TYPE_OBJECT);

static void grss_feed_enclosure_finalize (GObject *obj)
{
	GrssFeedEnclosure *enclosure;

	enclosure = GRSS_FEED_ENCLOSURE (obj);
	FREE_STRING (enclosure->priv->url);
	FREE_STRING (enclosure->priv->type);
}

static void grss_feed_enclosure_class_init (GrssFeedEnclosureClass *klass)
{
	GObjectClass *gobject_class;

	g_type_class_add_private (klass, sizeof (GrssFeedEnclosurePrivate));

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->finalize = grss_feed_enclosure_finalize;
}

static void grss_feed_enclosure_init (GrssFeedEnclosure *node)
{
	node->priv = FEED_ENCLOSURE_GET_PRIVATE (node);
	memset (node->priv, 0, sizeof (GrssFeedEnclosurePrivate));
}

/**
 * grss_feed_enclosure_new:
 * @url: URL of the external element
 *
 * Allocates a new #GrssFeedEnclosure, to be downloaded separately
 *
 * Return value: a new #GrssFeedEnclosure
 */
GrssFeedEnclosure* grss_feed_enclosure_new (gchar *url)
{
	GrssFeedEnclosure *ret;

	ret = g_object_new (GRSS_FEED_ENCLOSURE_TYPE, NULL);
	ret->priv->url = g_strdup (url);
	return ret;
}

/**
 * grss_feed_enclosure_get_url:
 * @enclosure: a #GrssFeedEnclosure
 *
 * Retrieves the URL of the @enclosure
 *
 * Return value: the URL where the enclosure may be found
 */
const gchar* grss_feed_enclosure_get_url (GrssFeedEnclosure *enclosure)
{
	return (const gchar*) enclosure->priv->url;
}

/**
 * grss_feed_enclosure_set_format:
 * @enclosure: a #GrssFeedEnclosure
 * @type: type of content
 *
 * To set the type of the external file
 */
void grss_feed_enclosure_set_format (GrssFeedEnclosure *enclosure, gchar *type)
{
	FREE_STRING (enclosure->priv->type);
	enclosure->priv->type = g_strdup (type);
}

/**
 * grss_feed_enclosure_get_format:
 * @enclosure: a #GrssFeedEnclosure
 *
 * Retrieves the format of the enclosed file
 *
 * Return value: type of @enclosure
 */
const gchar* grss_feed_enclosure_get_format (GrssFeedEnclosure *enclosure)
{
	return (const gchar*) enclosure->priv->type;
}

/**
 * grss_feed_enclosure_set_length:
 * @enclosure: a #GrssFeedEnclosure
 * @length: size of the enclosure, in bytes
 *
 * To set the size of the embedded @enclosure
 */
void grss_feed_enclosure_set_length (GrssFeedEnclosure *enclosure, gsize length)
{
	enclosure->priv->length = length;
}

/**
 * grss_feed_enclosure_get_length:
 * @enclosure: a #GrssFeedEnclosure
 *
 * Retrieves the size of the embedded file
 *
 * Return value: size of the @enclosure, in bytes
 */
gsize grss_feed_enclosure_get_length (GrssFeedEnclosure *enclosure)
{
	return enclosure->priv->length;
}
