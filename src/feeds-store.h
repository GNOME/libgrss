/*
 * Copyright (C) 2009/2010, Roberto Guido <rguido@src.gnome.org>
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

#ifndef __FEEDS_STORE_H__
#define __FEEDS_STORE_H__

#include "libgrss.h"

#define GRSS_FEEDS_STORE_TYPE		(grss_feeds_store_get_type())
#define GRSS_FEEDS_STORE(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), GRSS_FEEDS_STORE_TYPE, GrssFeedsStore))
#define GRSS_FEEDS_STORE_CLASS(c)	(G_TYPE_CHECK_CLASS_CAST ((c), GRSS_FEEDS_STORE_TYPE, GrssFeedsStoreClass))
#define GRSS_IS_FEEDS_STORE(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), GRSS_FEEDS_STORE_TYPE))
#define GRSS_IS_FEEDS_STORE_CLASS(c)	(G_TYPE_CHECK_CLASS_TYPE ((c),  GRSS_FEEDS_STORE_TYPE))
#define GRSS_FEEDS_STORE_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), GRSS_FEEDS_STORE_TYPE, GrssFeedsStoreClass))

typedef struct _GrssFeedsStore		GrssFeedsStore;
typedef struct _GrssFeedsStorePrivate	GrssFeedsStorePrivate;

struct _GrssFeedsStore {
	GObject parent;
	GrssFeedsStorePrivate *priv;
};

typedef struct {
	GObjectClass parent;

	GList* (*get_channels) (GrssFeedsStore *store);
	GList* (*get_items_by_channel) (GrssFeedsStore *store, GrssFeedChannel *channel);
	gboolean (*has_item) (GrssFeedsStore *store, GrssFeedChannel *channel, const gchar *id);
	void (*add_item_in_channel) (GrssFeedsStore *store, GrssFeedChannel *channel, GrssFeedItem *item);
} GrssFeedsStoreClass;

GType		grss_feeds_store_get_type		() G_GNUC_CONST;

GList*		grss_feeds_store_get_channels		(GrssFeedsStore *store);
GList*		grss_feeds_store_get_items_by_channel	(GrssFeedsStore *store, GrssFeedChannel *channel);
gboolean	grss_feeds_store_has_item		(GrssFeedsStore *store, GrssFeedChannel *channel, const gchar *id);
void		grss_feeds_store_add_item_in_channel	(GrssFeedsStore *store, GrssFeedChannel *channel, GrssFeedItem *item);
void		grss_feeds_store_switch			(GrssFeedsStore *store, gboolean run);

#endif /* __FEEDS_STORE_H__ */
