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

#define FEED_ENCLOSURE_GET_PRIVATE(obj)     (G_TYPE_INSTANCE_GET_PRIVATE ((obj), FEED_ENCLOSURE_TYPE, FeedEnclosurePrivate))

/**
 * SECTION: feed-enclosure
 * @short_description: a component attached to an item
 *
 * #FeedEnclosure describes an external element embedded into a #FeedItem: it
 * may be an image, a video of other kind of file to be presented with the
 * parent item
 */

struct _FeedEnclosurePrivate {
	gchar	*url;
	gchar	*type;
	gsize	length;
};

G_DEFINE_TYPE (FeedEnclosure, feed_enclosure, G_TYPE_OBJECT);

static void feed_enclosure_finalize (GObject *obj)
{
	FeedEnclosure *enclosure;

	enclosure = FEED_ENCLOSURE (obj);
	FREE_STRING (enclosure->priv->url);
	FREE_STRING (enclosure->priv->type);
}

static void feed_enclosure_class_init (FeedEnclosureClass *klass)
{
	GObjectClass *gobject_class;

	g_type_class_add_private (klass, sizeof (FeedEnclosurePrivate));

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->finalize = feed_enclosure_finalize;
}

static void feed_enclosure_init (FeedEnclosure *node)
{
	node->priv = FEED_ENCLOSURE_GET_PRIVATE (node);
	memset (node->priv, 0, sizeof (FeedEnclosurePrivate));
}

/**
 * feed_enclosure_new:
 * @url: URL of the external element
 *
 * Allocates a new #FeedEnclosure, to be downloaded separately
 *
 * Return value: a new #FeedEnclosure
 */
FeedEnclosure* feed_enclosure_new (gchar *url)
{
	FeedEnclosure *ret;

	ret = g_object_new (FEED_ENCLOSURE_TYPE, NULL);
	ret->priv->url = g_strdup (url);
	return ret;
}

/**
 * feed_enclosure_get_url:
 * @enclosure: a #FeedEnclosure
 *
 * Retrieves the URL of the @enclosure
 *
 * Return value: the URL where the enclosure may be found
 */
const gchar* feed_enclosure_get_url (FeedEnclosure *enclosure)
{
	return (const gchar*) enclosure->priv->url;
}

/**
 * feed_enclosure_set_format:
 * @enclosure: a #FeedEnclosure
 * @type: type of content
 *
 * To set the type of the external file
 */
void feed_enclosure_set_format (FeedEnclosure *enclosure, gchar *type)
{
	FREE_STRING (enclosure->priv->type);
	enclosure->priv->type = g_strdup (type);
}

/**
 * feed_enclosure_get_format:
 * @enclosure: a #FeedEnclosure
 *
 * Retrieves the format of the enclosed file
 *
 * Return value: type of @enclosure
 */
const gchar* feed_enclosure_get_format (FeedEnclosure *enclosure)
{
	return (const gchar*) enclosure->priv->type;
}

/**
 * feed_enclosure_set_length:
 * @enclosure: a #FeedEnclosure
 * @length: size of the enclosure
 *
 * To set the size of the embedded @enclosure
 */
void feed_enclosure_set_length (FeedEnclosure *enclosure, gsize length)
{
	enclosure->priv->length = length;
}

/**
 * feed_enclosure_get_length:
 * @enclosure: a #FeedEnclosure
 *
 * Retrieves the size of the embedded file
 *
 * Return value: size of the @enclosure
 */
gsize feed_enclosure_get_length (FeedEnclosure *enclosure)
{
	return enclosure->priv->length;
}
