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

#define FEED_ITEM_GET_PRIVATE(obj)     (G_TYPE_INSTANCE_GET_PRIVATE ((obj), FEED_ITEM_TYPE, FeedItemPrivate))

/**
 * SECTION: feed-item
 * @short_description: a feed item
 *
 * #FeedItem is an abstraction for an item, collects all informations about a
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

struct _FeedItemPrivate {
	FeedChannel	*parent;

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

G_DEFINE_TYPE (FeedItem, feed_item, G_TYPE_OBJECT);

static void
feed_item_finalize (GObject *obj)
{
	GList *iter;
	FeedItem *item;

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
feed_item_class_init (FeedItemClass *klass)
{
	GObjectClass *gobject_class;

	g_type_class_add_private (klass, sizeof (FeedItemPrivate));

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->finalize = feed_item_finalize;
}

static void
feed_item_init (FeedItem *node)
{
	node->priv = FEED_ITEM_GET_PRIVATE (node);
	memset (node->priv, 0, sizeof (FeedItemPrivate));
}

/**
 * feed_item_new:
 * @parent: the feed from which the new item belongs
 *
 * To allocate a new empty #FeedItem
 *
 * Return value: a new #FeedItem
 */
FeedItem*
feed_item_new (FeedChannel *parent)
{
	FeedItem *item;

	item = FEED_ITEM (g_object_new (FEED_ITEM_TYPE, NULL));
	item->priv->parent = parent;
	item->priv->pub_time = time (NULL);
	return item;
}

/**
 * feed_item_get_parent:
 * @item: a #FeedItem
 *
 * Retrieves the feed from which the item belongs
 *
 * Return value: the parent feed, as set in feed_item_new()
 */
FeedChannel*
feed_item_get_parent (FeedItem *item)
{
	return item->priv->parent;
}

/**
 * feed_item_set_id:
 * @item: a #FeedItem
 * @id: the new ID to set
 *
 * To set the ID of the @item. This parameter has not a particular format: it
 * is just a string used to identify in unique way the item
 */
void
feed_item_set_id (FeedItem *item, gchar *id)
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
 * feed_item_get_id:
 * @item: #FeedItem from which retrieve the ID
 *
 * Retrieves the ID assigned to the @item. If no ID was set with
 * feed_item_set_id() this returns the same of feed_item_get_source().
 * Pay attention to the fact this library do not check uniqueness of assigned
 * IDs
 *
 * Return value: ID of the item
 */
const gchar*
feed_item_get_id (FeedItem *item)
{
	if (item->priv->id != NULL)
		return (const gchar*) item->priv->id;
	else
		return feed_item_get_source (item);
}

/**
 * feed_item_set_title:
 * @item: a #FeedItem
 * @title: title of the item
 *
 * To set a title to the @item
 */
void
feed_item_set_title (FeedItem *item, gchar *title)
{
	FREE_STRING (item->priv->title);
	item->priv->title = g_strdup (title);
}

/**
 * feed_item_get_title:
 * @item: a #FeedItem
 *
 * Retrieves the title assigned to @item
 *
 * Return value: title of the element
 */
const gchar*
feed_item_get_title (FeedItem *item)
{
	return (const gchar*) item->priv->title;
}

/**
 * feed_item_set_description:
 * @item: a #FeedItem
 * @description: content of the item
 *
 * To set the description of @item. Usually "description" means his content
 */
void
feed_item_set_description (FeedItem *item, gchar *description)
{
	FREE_STRING (item->priv->description);
	item->priv->description = g_strdup (description);
}

/**
 * feed_item_get_description:
 * @item: a #FeedItem
 *
 * Retrieves the description of the @item
 *
 * Return value: description of @item
 */
const gchar*
feed_item_get_description (FeedItem *item)
{
	return (const gchar*) item->priv->description;
}

/**
 * feed_item_add_category:
 * @item: a #FeedItem
 * @category: a new category to assign to the item
 *
 * Adds a category to the @item. The complete list can be obtained with
 * feed_item_get_categories()
 */
void
feed_item_add_category (FeedItem *item, gchar *category)
{
	gchar *cat;

	cat = g_strdup (category);

	if (item->priv->categories == NULL)
		item->priv->categories = g_list_prepend (item->priv->categories, cat);
	else
		item->priv->categories = g_list_append (item->priv->categories, cat);
}

/**
 * feed_item_get_categories:
 * @item: a #FeedItem
 *
 * Retrieves list of categories assigned to the @item
 *
 * Return value: list of strings, one for assigned category. Do not free or
 * modify this list
 */
const GList*
feed_item_get_categories (FeedItem *item)
{
	return (const GList*) item->priv->categories;
}

/**
 * feed_item_set_source:
 * @item: a #FeedItem
 * @source: URL of the item
 *
 * To set the source of the @item
 */
void
feed_item_set_source (FeedItem *item, gchar *source)
{
	FREE_STRING (item->priv->source);
	item->priv->source = g_strdup (source);
}

/**
 * feed_item_get_source:
 * @item: a #FeedItem
 *
 * Retrieves the URL where the @item can be found
 *
 * Return value: URL of the item, or NULL
 */
const gchar*
feed_item_get_source (FeedItem *item)
{
	return (const gchar*) item->priv->source;
}

/**
 * feed_item_set_real_source:
 * @item: a #FeedItem
 * @realsource: URL of the real source for the item
 * @title: title of the real source
 *
 * To set an alternative real source for @item. This parameter is used by web
 * aggregators to explicit the origin of a content reproduced in them
 */
void
feed_item_set_real_source (FeedItem *item, gchar *realsource, gchar *title)
{
	FREE_STRING (item->priv->real_source_url);
	item->priv->real_source_url = g_strdup (realsource);
	FREE_STRING (item->priv->real_source_title);
	item->priv->real_source_title = g_strdup (title);
}

/**
 * feed_item_get_real_source:
 * @item: a #FeedItem
 * @realsource: will be assigned to the URL of the real source, or NULL
 * @title: will be assigned to the title of the real source, or NULL
 *
 * Retrieves references to the real source of @item
 */
void
feed_item_get_real_source (FeedItem *item, const gchar **realsource, const gchar **title)
{
	if (realsource != NULL)
		*realsource = item->priv->real_source_url;
	if (title != NULL)
		*title = item->priv->real_source_title;
}

/**
 * feed_item_set_related:
 * @item: a #FeedItem
 * @related: reference to a related post
 *
 * To set reference to a post related to @item
 */
void
feed_item_set_related (FeedItem *item, gchar *related)
{
	FREE_STRING (item->priv->related);
	item->priv->related = g_strdup (related);
}

/**
 * feed_item_get_related:
 * @item: a #FeedItem
 *
 * Retrieves indication about posts related to @item
 *
 * Return value: related posts, or NULL
 */
const gchar*
feed_item_get_related (FeedItem *item)
{
	return (const gchar*) item->priv->related;
}

/**
 * feed_item_set_copyright:
 * @item: a #FeedItem
 * @copyright: copyright declaration for the item
 *
 * To set a copyright reference to @item
 */
void
feed_item_set_copyright (FeedItem *item, gchar *copyright)
{
	FREE_STRING (item->priv->copyright);
	item->priv->copyright = g_strdup (copyright);
}

/**
 * feed_item_get_copyright:
 * @item: a #FeedItem
 *
 * Retrieves copyright reference for the @item
 *
 * Return value: copyright mark, or NULL
 */
const gchar*
feed_item_get_copyright (FeedItem *item)
{
	return (const gchar*) item->priv->copyright;
}

/**
 * feed_item_set_author:
 * @item: a #FeedItem
 * @author: name of the author
 *
 * To assign an author to the @item
 */
void
feed_item_set_author (FeedItem *item, gchar *author)
{
	FREE_STRING (item->priv->author);
	item->priv->author = g_strdup (author);
}

/**
 * feed_item_get_author:
 * @item: a #FeedItem
 *
 * Retrieves the author of @item
 *
 * Return value: author of the item, or NULL
 */
const gchar*
feed_item_get_author (FeedItem *item)
{
	return (const gchar*) item->priv->author;
}

/**
 * feed_item_add_contributor:
 * @item: a #FeedItem
 * @contributor: name of the contributor for the item
 *
 * To add a contributor to the @item
 */
void
feed_item_add_contributor (FeedItem *item, gchar *contributor)
{
	gchar *con;

	con = g_strdup (contributor);

	if (item->priv->contributors == NULL)
		item->priv->contributors = g_list_prepend (item->priv->contributors, con);
	else
		item->priv->contributors = g_list_append (item->priv->contributors, con);
}

/**
 * feed_item_get_contributors:
 * @item: a #FeedItem
 *
 * Retrieves contributors for @item
 *
 * Return value: list of contributors to the item
 */
const GList*
feed_item_get_contributors (FeedItem *item)
{
	return (const GList*) item->priv->contributors;
}

/**
 * feed_item_set_comments_url:
 * @item: a #FeedItem
 * @url: URL where to retrieve comments to the item
 *
 * To assign the URL where to fetch comments for the item
 */
void
feed_item_set_comments_url (FeedItem *item, gchar *url)
{
	FREE_STRING (item->priv->comments_url);
	item->priv->comments_url = g_strdup (url);
}

/**
 * feed_item_get_comments_url:
 * @item: a #FeedItem
 *
 * Retrieves the URL where to catch comments to the @item
 *
 * Return value: URL to parse to read comments for @item, or NULL
 */
const gchar*
feed_item_get_comments_url (FeedItem *item)
{
	return (const gchar*) item->priv->comments_url;
}

/**
 * feed_item_set_geo_point:
 * @item: a #FeedItem
 * @latitude: latitude of the point, or -1 to leave the previous one
 * @longitude: longitude of the point, or -1 to leave the previous one
 *
 * To assign geographic context to the @item.
 * Passing -1 as @latitude or @longitude, the relative value is untouched in
 * the object. This is to easy assignment of coordinates in more than a
 * single step. If both are -1, nothing happens
 */
void
feed_item_set_geo_point (FeedItem *item, double latitude, double longitude)
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
 * feed_item_get_geo_point:
 * @item: a #FeedItem
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
feed_item_get_geo_point (FeedItem *item, double *latitude, double *longitude)
{
	if (latitude)
		*latitude = item->priv->geo.lat;
	if (longitude)
		*longitude = item->priv->geo.lon;

	return item->priv->geo.has;
}

/**
 * feed_item_set_publish_time:
 * @item: a #FeedItem
 * @publish: publishing timestamp of the item
 *
 * To set the publish time of the item
 */
void
feed_item_set_publish_time (FeedItem *item, time_t publish)
{
	item->priv->pub_time = publish;
}

/**
 * feed_item_get_publish_time:
 * @item: a #FeedItem
 *
 * Retrieves the publish time of the item. By default this value is the
 * current timestamp assigned when creating the #FeedItem, and may be changed
 * with feed_item_set_publish_time()
 *
 * Return value: publish time of @item
 */
time_t
feed_item_get_publish_time (FeedItem *item)
{
	return item->priv->pub_time;
}

/**
 * feed_item_add_enclosure:
 * @item: a #FeedItem
 * @enclosure: a #FeedEnclosure to add to the item
 *
 * Adds an enclosure to the @item. That external elements may be references
 * to images, videos, or other contents (usually multimedial) embedded in the
 * element
 */
void
feed_item_add_enclosure (FeedItem *item, FeedEnclosure *enclosure)
{
	if (item->priv->enclosures == NULL)
		item->priv->enclosures = g_list_prepend (item->priv->enclosures, enclosure);
	else
		item->priv->enclosures = g_list_append (item->priv->enclosures, enclosure);
}

/**
 * feed_item_get_enclosures:
 * @item: a #FeedItem
 *
 * Retrieves the list of enclosures added with feed_item_add_enclosure()
 *
 * Return value: a list of #FeedEnclosure. This is a direct reference to the
 * internal list, do not free or modify it
 */
const GList*
feed_item_get_enclosures (FeedItem *item)
{
	return item->priv->enclosures;
}
