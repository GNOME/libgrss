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
#include "feed-item.h"
#include "feed-channel.h"

#define FEED_ITEM_GET_PRIVATE(obj)     (G_TYPE_INSTANCE_GET_PRIVATE ((obj), FEED_ITEM_TYPE, GrssFeedItemPrivate))

/**
 * SECTION: feed-item
 * @short_description: a feed item
 *
 * #GrssFeedItem is an abstraction for an item, collects all information about a
 * single element found into a feed
 */

/*
	TODO	Drop this structure and migrate to libchamplain
*/
typedef struct {
	gboolean	has;
	double		lat;
	double		lon;
} GeoInfo;

struct _GrssFeedItemPrivate {
	GrssFeedChannel	*parent;

	gchar		*id;
	gchar		*title;
	gchar		*description;
	GList		*categories;
	gchar		*source;
	gchar		*real_source_url;
	gchar		*real_source_title;
	gchar		*related;

	gchar		*copyright;
	gchar		*author;
	GList		*contributors;
	gchar		*comments_url;

	GeoInfo		geo;
	time_t		pub_time;

	GList		*enclosures;
};

G_DEFINE_TYPE (GrssFeedItem, grss_feed_item, G_TYPE_OBJECT);

static void
grss_feed_item_finalize (GObject *obj)
{
	GList *iter;
	GrssFeedItem *item;

	item = FEED_ITEM (obj);
	FREE_STRING (item->priv->id);
	FREE_STRING (item->priv->title);
	FREE_STRING (item->priv->description);
	FREE_STRING (item->priv->source);
	FREE_STRING (item->priv->real_source_url);
	FREE_STRING (item->priv->real_source_title);
	FREE_STRING (item->priv->related);
	FREE_STRING (item->priv->copyright);
	FREE_STRING (item->priv->author);
	FREE_STRING (item->priv->comments_url);

	if (item->priv->enclosures != NULL) {
		for (iter = item->priv->enclosures; iter; iter = g_list_next (iter))
			g_object_unref (iter->data);
		g_list_free (item->priv->enclosures);
	}

	if (item->priv->categories != NULL) {
		for (iter = item->priv->categories; iter; iter = g_list_next (iter))
			g_free (iter->data);
		g_list_free (item->priv->categories);
	}

	if (item->priv->contributors != NULL) {
		for (iter = item->priv->contributors; iter; iter = g_list_next (iter))
			g_free (iter->data);
		g_list_free (item->priv->contributors);
	}
}

static void
grss_feed_item_class_init (GrssFeedItemClass *klass)
{
	GObjectClass *gobject_class;

	g_type_class_add_private (klass, sizeof (GrssFeedItemPrivate));

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->finalize = grss_feed_item_finalize;
}

static void
grss_feed_item_init (GrssFeedItem *node)
{
	node->priv = FEED_ITEM_GET_PRIVATE (node);
	memset (node->priv, 0, sizeof (GrssFeedItemPrivate));
}

/**
 * grss_feed_item_new:
 * @parent: the feed from which the new item belongs
 *
 * To allocate a new empty #GrssFeedItem
 *
 * Return value: a new #GrssFeedItem
 */
GrssFeedItem*
grss_feed_item_new (GrssFeedChannel *parent)
{
	GrssFeedItem *item;

	item = FEED_ITEM (g_object_new (FEED_ITEM_TYPE, NULL));
	item->priv->parent = parent;
	item->priv->pub_time = time (NULL);
	return item;
}

/**
 * grss_feed_item_get_parent:
 * @item: a #GrssFeedItem
 *
 * Retrieves the feed from which the item belongs
 *
 * Return value: the parent feed, as set in grss_feed_item_new()
 */
GrssFeedChannel*
grss_feed_item_get_parent (GrssFeedItem *item)
{
	return item->priv->parent;
}

/**
 * grss_feed_item_set_id:
 * @item: a #GrssFeedItem
 * @id: the new ID to set
 *
 * To set the ID of the @item. This parameter has not a particular format: it
 * is just a string used to identify in unique way the item
 */
void
grss_feed_item_set_id (GrssFeedItem *item, gchar *id)
{
	gchar *iter;

	FREE_STRING (item->priv->id);
	item->priv->id = g_strdup (id);

	/*
		All blanks from the id are stripped and replaced by underscores
	*/

	for (iter = item->priv->id; *iter != '\0'; iter++)
		if (*iter == ' ')
			*iter = '_';
}

/**
 * grss_feed_item_get_id:
 * @item: #GrssFeedItem from which retrieve the ID
 *
 * Retrieves the ID assigned to the @item. If no ID was set with
 * grss_feed_item_set_id() this returns the same of grss_feed_item_get_source().
 * Pay attention to the fact this library do not check uniqueness of assigned
 * IDs
 *
 * Return value: ID of the item
 */
const gchar*
grss_feed_item_get_id (GrssFeedItem *item)
{
	if (item->priv->id != NULL)
		return (const gchar*) item->priv->id;
	else
		return grss_feed_item_get_source (item);
}

/**
 * grss_feed_item_set_title:
 * @item: a #GrssFeedItem
 * @title: title of the item
 *
 * To set a title to the @item
 */
void
grss_feed_item_set_title (GrssFeedItem *item, gchar *title)
{
	FREE_STRING (item->priv->title);
	item->priv->title = g_strdup (title);
}

/**
 * grss_feed_item_get_title:
 * @item: a #GrssFeedItem
 *
 * Retrieves the title assigned to @item
 *
 * Return value: title of the element
 */
const gchar*
grss_feed_item_get_title (GrssFeedItem *item)
{
	return (const gchar*) item->priv->title;
}

/**
 * grss_feed_item_set_description:
 * @item: a #GrssFeedItem
 * @description: content of the item
 *
 * To set the description of @item. Usually "description" means his content
 */
void
grss_feed_item_set_description (GrssFeedItem *item, gchar *description)
{
	FREE_STRING (item->priv->description);
	item->priv->description = g_strdup (description);
}

/**
 * grss_feed_item_get_description:
 * @item: a #GrssFeedItem
 *
 * Retrieves the description of the @item
 *
 * Return value: description of @item
 */
const gchar*
grss_feed_item_get_description (GrssFeedItem *item)
{
	return (const gchar*) item->priv->description;
}

/**
 * grss_feed_item_add_category:
 * @item: a #GrssFeedItem
 * @category: a new category to assign to the item
 *
 * Adds a category to the @item. The complete list can be obtained with
 * grss_feed_item_get_categories()
 */
void
grss_feed_item_add_category (GrssFeedItem *item, gchar *category)
{
	gchar *cat;

	cat = g_strdup (category);

	if (item->priv->categories == NULL)
		item->priv->categories = g_list_prepend (item->priv->categories, cat);
	else
		item->priv->categories = g_list_append (item->priv->categories, cat);
}

/**
 * grss_feed_item_get_categories:
 * @item: a #GrssFeedItem
 *
 * Retrieves list of categories assigned to the @item
 *
 * Return value: list of strings, one for assigned category. Do not free or
 * modify this list
 */
const GList*
grss_feed_item_get_categories (GrssFeedItem *item)
{
	return (const GList*) item->priv->categories;
}

/**
 * grss_feed_item_set_source:
 * @item: a #GrssFeedItem
 * @source: URL of the item
 *
 * To set the source of the @item
 */
void
grss_feed_item_set_source (GrssFeedItem *item, gchar *source)
{
	FREE_STRING (item->priv->source);
	item->priv->source = g_strdup (source);
}

/**
 * grss_feed_item_get_source:
 * @item: a #GrssFeedItem
 *
 * Retrieves the URL where the @item can be found
 *
 * Return value: URL of the item, or NULL
 */
const gchar*
grss_feed_item_get_source (GrssFeedItem *item)
{
	return (const gchar*) item->priv->source;
}

/**
 * grss_feed_item_set_real_source:
 * @item: a #GrssFeedItem
 * @realsource: URL of the real source for the item
 * @title: title of the real source
 *
 * To set an alternative real source for @item. This parameter is used by web
 * aggregators to explicit the origin of a content reproduced in them
 */
void
grss_feed_item_set_real_source (GrssFeedItem *item, gchar *realsource, gchar *title)
{
	FREE_STRING (item->priv->real_source_url);
	item->priv->real_source_url = g_strdup (realsource);
	FREE_STRING (item->priv->real_source_title);
	item->priv->real_source_title = g_strdup (title);
}

/**
 * grss_feed_item_get_real_source:
 * @item: a #GrssFeedItem
 * @realsource: will be assigned to the URL of the real source, or NULL
 * @title: will be assigned to the title of the real source, or NULL
 *
 * Retrieves references to the real source of @item
 */
void
grss_feed_item_get_real_source (GrssFeedItem *item, const gchar **realsource, const gchar **title)
{
	if (realsource != NULL)
		*realsource = item->priv->real_source_url;
	if (title != NULL)
		*title = item->priv->real_source_title;
}

/**
 * grss_feed_item_set_related:
 * @item: a #GrssFeedItem
 * @related: reference to a related post
 *
 * To set reference to a post related to @item
 */
void
grss_feed_item_set_related (GrssFeedItem *item, gchar *related)
{
	FREE_STRING (item->priv->related);
	item->priv->related = g_strdup (related);
}

/**
 * grss_feed_item_get_related:
 * @item: a #GrssFeedItem
 *
 * Retrieves indication about posts related to @item
 *
 * Return value: related posts, or NULL
 */
const gchar*
grss_feed_item_get_related (GrssFeedItem *item)
{
	return (const gchar*) item->priv->related;
}

/**
 * grss_feed_item_set_copyright:
 * @item: a #GrssFeedItem
 * @copyright: copyright declaration for the item
 *
 * To set a copyright reference to @item
 */
void
grss_feed_item_set_copyright (GrssFeedItem *item, gchar *copyright)
{
	FREE_STRING (item->priv->copyright);
	item->priv->copyright = g_strdup (copyright);
}

/**
 * grss_feed_item_get_copyright:
 * @item: a #GrssFeedItem
 *
 * Retrieves copyright reference for the @item
 *
 * Return value: copyright mark, or NULL
 */
const gchar*
grss_feed_item_get_copyright (GrssFeedItem *item)
{
	return (const gchar*) item->priv->copyright;
}

/**
 * grss_feed_item_set_author:
 * @item: a #GrssFeedItem
 * @author: name of the author
 *
 * To assign an author to the @item
 */
void
grss_feed_item_set_author (GrssFeedItem *item, gchar *author)
{
	FREE_STRING (item->priv->author);
	item->priv->author = g_strdup (author);
}

/**
 * grss_feed_item_get_author:
 * @item: a #GrssFeedItem
 *
 * Retrieves the author of @item
 *
 * Return value: author of the item, or NULL
 */
const gchar*
grss_feed_item_get_author (GrssFeedItem *item)
{
	return (const gchar*) item->priv->author;
}

/**
 * grss_feed_item_add_contributor:
 * @item: a #GrssFeedItem
 * @contributor: name of the contributor for the item
 *
 * To add a contributor to the @item
 */
void
grss_feed_item_add_contributor (GrssFeedItem *item, gchar *contributor)
{
	gchar *con;

	con = g_strdup (contributor);

	if (item->priv->contributors == NULL)
		item->priv->contributors = g_list_prepend (item->priv->contributors, con);
	else
		item->priv->contributors = g_list_append (item->priv->contributors, con);
}

/**
 * grss_feed_item_get_contributors:
 * @item: a #GrssFeedItem
 *
 * Retrieves contributors for @item
 *
 * Return value: list of contributors to the item
 */
const GList*
grss_feed_item_get_contributors (GrssFeedItem *item)
{
	return (const GList*) item->priv->contributors;
}

/**
 * grss_feed_item_set_comments_url:
 * @item: a #GrssFeedItem
 * @url: URL where to retrieve comments to the item
 *
 * To assign the URL where to fetch comments for the item
 */
void
grss_feed_item_set_comments_url (GrssFeedItem *item, gchar *url)
{
	FREE_STRING (item->priv->comments_url);
	item->priv->comments_url = g_strdup (url);
}

/**
 * grss_feed_item_get_comments_url:
 * @item: a #GrssFeedItem
 *
 * Retrieves the URL where to catch comments to the @item
 *
 * Return value: URL to parse to read comments for @item, or NULL
 */
const gchar*
grss_feed_item_get_comments_url (GrssFeedItem *item)
{
	return (const gchar*) item->priv->comments_url;
}

/**
 * grss_feed_item_set_geo_point:
 * @item: a #GrssFeedItem
 * @latitude: latitude of the point, or -1 to leave the previous one
 * @longitude: longitude of the point, or -1 to leave the previous one
 *
 * To assign geographic context to the @item.
 * Passing -1 as @latitude or @longitude, the relative value is untouched in
 * the object. This is to easy assignment of coordinates in more than a
 * single step. If both are -1, nothing happens
 */
void
grss_feed_item_set_geo_point (GrssFeedItem *item, double latitude, double longitude)
{
	if (latitude == -1 && longitude == -1)
		return;

	item->priv->geo.has = TRUE;

	if (latitude != -1)
		item->priv->geo.lat = latitude;

	if (longitude != -1)
		item->priv->geo.lon = longitude;
}

/**
 * grss_feed_item_get_geo_point:
 * @item: a #GrssFeedItem
 * @latitude: will be assigned to the latitude of the point, or NULL
 * @longitude: will be assigned to the longitude of the point, or NULL
 *
 * Retrieves the geo reference of the @item
 *
 * Return value: TRUE if @item has geographic coordinates assigned and
 * @latitude and @longitude have been set, FALSE if @item has not geo
 * reference
 */
gboolean
grss_feed_item_get_geo_point (GrssFeedItem *item, double *latitude, double *longitude)
{
	if (latitude)
		*latitude = item->priv->geo.lat;
	if (longitude)
		*longitude = item->priv->geo.lon;

	return item->priv->geo.has;
}

/**
 * grss_feed_item_set_publish_time:
 * @item: a #GrssFeedItem
 * @publish: publishing timestamp of the item
 *
 * To set the publish time of the item
 */
void
grss_feed_item_set_publish_time (GrssFeedItem *item, time_t publish)
{
	item->priv->pub_time = publish;
}

/**
 * grss_feed_item_get_publish_time:
 * @item: a #GrssFeedItem
 *
 * Retrieves the publish time of the item. By default this value is the
 * current timestamp assigned when creating the #GrssFeedItem, and may be changed
 * with grss_feed_item_set_publish_time()
 *
 * Return value: publish time of @item
 */
time_t
grss_feed_item_get_publish_time (GrssFeedItem *item)
{
	return item->priv->pub_time;
}

/**
 * grss_feed_item_add_enclosure:
 * @item: a #GrssFeedItem
 * @enclosure: a #GrssFeedEnclosure to add to the item
 *
 * Adds an enclosure to the @item. That external elements may be references
 * to images, videos, or other contents (usually multimedial) embedded in the
 * element
 */
void
grss_feed_item_add_enclosure (GrssFeedItem *item, GrssFeedEnclosure *enclosure)
{
	if (item->priv->enclosures == NULL)
		item->priv->enclosures = g_list_prepend (item->priv->enclosures, enclosure);
	else
		item->priv->enclosures = g_list_append (item->priv->enclosures, enclosure);
}

/**
 * grss_feed_item_get_enclosures:
 * @item: a #GrssFeedItem
 *
 * Retrieves the list of enclosures added with grss_feed_item_add_enclosure()
 *
 * Return value: a list of #GrssFeedEnclosure. This is a direct reference to the
 * internal list, do not free or modify it
 */
const GList*
grss_feed_item_get_enclosures (GrssFeedItem *item)
{
	return item->priv->enclosures;
}
