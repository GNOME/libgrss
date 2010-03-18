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

#include "common.h"
#include "feed-channel.h"
#include "feed-item.h"

#define FEEDS_STORE_TYPE		(feeds_store_get_type())
#define FEEDS_STORE(o)			(G_TYPE_CHECK_INSTANCE_CAST ((o), FEEDS_STORE_TYPE, FeedsStore))
#define FEEDS_STORE_CLASS(c)		(G_TYPE_CHECK_CLASS_CAST ((c), FEEDS_STORE_TYPE, FeedsStoreClass))
#define IS_FEEDS_STORE(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), FEEDS_STORE_TYPE))
#define IS_FEEDS_STORE_CLASS(c)		(G_TYPE_CHECK_CLASS_TYPE ((c),  FEEDS_STORE_TYPE))
#define FEEDS_STORE_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), FEEDS_STORE_TYPE, FeedsStoreClass))

typedef struct _FeedsStore		FeedsStore;
typedef struct _FeedsStorePrivate	FeedsStorePrivate;

struct _FeedsStore {
	GObject parent;
	FeedsStorePrivate *priv;
};

typedef struct {
	GObjectClass parent;

	GList* (*get_channels) (FeedsStore *store);
	GList* (*get_items_by_channel) (FeedsStore *store, FeedChannel *channel);
	gboolean (*has_item) (FeedsStore *store, FeedChannel *channel, const gchar *id);
	void (*add_item_in_channel) (FeedsStore *store, FeedChannel *channel, FeedItem *item);
} FeedsStoreClass;

GType		feeds_store_get_type	() G_GNUC_CONST;

GList*		feeds_store_get_channels		(FeedsStore *store);
GList*		feeds_store_get_items_by_channel	(FeedsStore *store, FeedChannel *channel);
gboolean	feeds_store_has_item			(FeedsStore *store, FeedChannel *channel, const gchar *id);
void		feeds_store_add_item_in_channel		(FeedsStore *store, FeedChannel *channel, FeedItem *item);
void		feeds_store_switch			(FeedsStore *store, gboolean run);

#endif /* __FEEDS_STORE_H__ */
