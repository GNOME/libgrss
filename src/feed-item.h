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

#ifndef __FEED_ITEM_H__
#define __FEED_ITEM_H__

#include "libgrss.h"

#define FEED_ITEM_TYPE		(feed_item_get_type())
#define FEED_ITEM(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), FEED_ITEM_TYPE, FeedItem))
#define FEED_ITEM_CLASS(c)	(G_TYPE_CHECK_CLASS_CAST ((c), FEED_ITEM_TYPE, FeedItemClass))
#define IS_FEED_ITEM(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), FEED_ITEM_TYPE))
#define IS_FEED_ITEM_CLASS(c)	(G_TYPE_CHECK_CLASS_TYPE ((c),  FEED_ITEM_TYPE))
#define FEED_ITEM_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), FEED_ITEM_TYPE, FeedItemClass))

typedef struct _FeedItem	FeedItem;
typedef struct _FeedItemPrivate	FeedItemPrivate;

struct _FeedItem {
	GObject parent;
	FeedItemPrivate *priv;
};

typedef struct {
	GObjectClass parent;
} FeedItemClass;

GType		feed_item_get_type		() G_GNUC_CONST;

FeedItem*	feed_item_new			(FeedChannel *parent);

FeedChannel*	feed_item_get_parent		(FeedItem *item);

void		feed_item_set_id		(FeedItem *item, gchar *id);
const gchar*	feed_item_get_id		(FeedItem *item);
void		feed_item_set_title		(FeedItem *item, gchar *title);
const gchar*	feed_item_get_title		(FeedItem *item);
void		feed_item_set_description	(FeedItem *item, gchar *description);
const gchar*	feed_item_get_description	(FeedItem *item);
void		feed_item_add_category		(FeedItem *item, gchar *category);
const GList*	feed_item_get_categories	(FeedItem *item);
void		feed_item_set_source		(FeedItem *item, gchar *source);
const gchar*	feed_item_get_source		(FeedItem *item);
void		feed_item_set_real_source	(FeedItem *item, gchar *realsource, gchar *title);
void		feed_item_get_real_source	(FeedItem *item, const gchar **realsource, const gchar **title);
void		feed_item_set_related		(FeedItem *item, gchar *related);
const gchar*	feed_item_get_related		(FeedItem *item);

void		feed_item_set_copyright		(FeedItem *item, gchar *copyright);
const gchar*	feed_item_get_copyright		(FeedItem *item);
void		feed_item_set_author		(FeedItem *item, gchar *author);
const gchar*	feed_item_get_author		(FeedItem *item);
void		feed_item_add_contributor	(FeedItem *item, gchar *contributor);
const GList*	feed_item_get_contributors	(FeedItem *item);
void		feed_item_set_comments_url	(FeedItem *item, gchar *url);
const gchar*	feed_item_get_comments_url	(FeedItem *item);

void		feed_item_set_geo_point		(FeedItem *item, double latitude, double longitude);
gboolean	feed_item_get_geo_point		(FeedItem *item, double *latitude, double *longitude);
void		feed_item_set_publish_time	(FeedItem *item, time_t publish);
time_t		feed_item_get_publish_time	(FeedItem *item);

void		feed_item_add_enclosure		(FeedItem *item, FeedEnclosure *enclosure);
const GList*	feed_item_get_enclosures	(FeedItem *item);

#endif /* __FEED_ITEM_H__ */
