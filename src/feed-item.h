/*
 * Copyright (C) 2009-2015, Roberto Guido <rguido@src.gnome.org>
 *                          Michele Tameni <michele@amdplanet.it>
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

#ifndef __FEED_ITEM_H__
#define __FEED_ITEM_H__

#include "person.h"
#include "libgrss.h"

#define GRSS_FEED_ITEM_TYPE		(grss_feed_item_get_type())
#define GRSS_FEED_ITEM(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), GRSS_FEED_ITEM_TYPE, GrssFeedItem))
#define GRSS_FEED_ITEM_CLASS(c)		(G_TYPE_CHECK_CLASS_CAST ((c), GRSS_FEED_ITEM_TYPE, GrssFeedItemClass))
#define GRSS_IS_FEED_ITEM(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), GRSS_FEED_ITEM_TYPE))
#define GRSS_IS_FEED_ITEM_CLASS(c)	(G_TYPE_CHECK_CLASS_TYPE ((c),  GRSS_FEED_ITEM_TYPE))
#define GRSS_FEED_ITEM_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), GRSS_FEED_ITEM_TYPE, GrssFeedItemClass))

typedef struct _GrssFeedItem	GrssFeedItem;
typedef struct _GrssFeedItemPrivate	GrssFeedItemPrivate;

struct _GrssFeedItem {
	GObject parent;
	GrssFeedItemPrivate *priv;
};

typedef struct {
	GObjectClass parent;
} GrssFeedItemClass;

GType			grss_feed_item_get_type		() G_GNUC_CONST;

GrssFeedItem*		grss_feed_item_new		(GrssFeedChannel *parent);

GrssFeedChannel*	grss_feed_item_get_parent	(GrssFeedItem *item);

void			grss_feed_item_set_id		(GrssFeedItem *item, gchar *id);
const gchar*		grss_feed_item_get_id		(GrssFeedItem *item);
void			grss_feed_item_set_title	(GrssFeedItem *item, gchar *title);
const gchar*		grss_feed_item_get_title	(GrssFeedItem *item);
void			grss_feed_item_set_description	(GrssFeedItem *item, gchar *description);
const gchar*		grss_feed_item_get_description	(GrssFeedItem *item);
void			grss_feed_item_add_category	(GrssFeedItem *item, gchar *category);
const GList*		grss_feed_item_get_categories	(GrssFeedItem *item);
gboolean		grss_feed_item_set_source	(GrssFeedItem *item, gchar *source);
const gchar*		grss_feed_item_get_source	(GrssFeedItem *item);
gboolean		grss_feed_item_set_real_source	(GrssFeedItem *item, gchar *realsource, gchar *title);
void			grss_feed_item_get_real_source	(GrssFeedItem *item, const gchar **realsource, const gchar **title);
void			grss_feed_item_set_related	(GrssFeedItem *item, gchar *related);
const gchar*		grss_feed_item_get_related	(GrssFeedItem *item);

void			grss_feed_item_set_copyright	(GrssFeedItem *item, gchar *copyright);
const gchar*		grss_feed_item_get_copyright	(GrssFeedItem *item);
void			grss_feed_item_set_author	(GrssFeedItem *item, GrssPerson *author);
GrssPerson*		grss_feed_item_get_author	(GrssFeedItem *item);
void			grss_feed_item_add_contributor	(GrssFeedItem *item, GrssPerson *contributor);
const GList*		grss_feed_item_get_contributors	(GrssFeedItem *item);
gboolean		grss_feed_item_set_comments_url	(GrssFeedItem *item, gchar *url);
const gchar*		grss_feed_item_get_comments_url	(GrssFeedItem *item);

void			grss_feed_item_set_geo_point	(GrssFeedItem *item, double latitude, double longitude);
gboolean		grss_feed_item_get_geo_point	(GrssFeedItem *item, double *latitude, double *longitude);
void			grss_feed_item_set_publish_time	(GrssFeedItem *item, time_t publish);
time_t			grss_feed_item_get_publish_time	(GrssFeedItem *item);

void			grss_feed_item_add_enclosure	(GrssFeedItem *item, GrssFeedEnclosure *enclosure);
const GList*		grss_feed_item_get_enclosures	(GrssFeedItem *item);

#endif /* __FEED_ITEM_H__ */
