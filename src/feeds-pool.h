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

#ifndef __FEEDS_POOL_H__
#define __FEEDS_POOL_H__

#include "common.h"
#include "feed-channel.h"

#define FEEDS_POOL_TYPE			(feeds_pool_get_type())
#define FEEDS_POOL(o)			(G_TYPE_CHECK_INSTANCE_CAST ((o), FEEDS_POOL_TYPE, FeedsPool))
#define FEEDS_POOL_CLASS(c)		(G_TYPE_CHECK_CLASS_CAST ((c), FEEDS_POOL_TYPE, FeedsPoolClass))
#define IS_FEEDS_POOL(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), FEEDS_POOL_TYPE))
#define IS_FEEDS_POOL_CLASS(c)		(G_TYPE_CHECK_CLASS_TYPE ((c),  FEEDS_POOL_TYPE))
#define FEEDS_POOL_GET_CLASS(o)		(G_TYPE_INSTANCE_GET_CLASS ((o), FEEDS_POOL_TYPE, FeedsPoolClass))

typedef struct _FeedsPool		FeedsPool;
typedef struct _FeedsPoolPrivate	FeedsPoolPrivate;

struct _FeedsPool {
	GObject parent;
	FeedsPoolPrivate *priv;
};

typedef struct {
	GObjectClass parent;

	void (*feed_fetching) (FeedsPool *pool, FeedChannel *feed);
	void (*feed_ready) (FeedsPool *pool, FeedChannel *feed, GList *items);
} FeedsPoolClass;

GType		feeds_pool_get_type		() G_GNUC_CONST;

FeedsPool*	feeds_pool_new			();

void		feeds_pool_listen		(FeedsPool *pool, GList *feeds);
GList*		feeds_pool_get_listened		(FeedsPool *pool);
int		feeds_pool_get_listened_num	(FeedsPool *pool);
void		feeds_pool_switch		(FeedsPool *pool, gboolean run);
SoupSession*	feeds_pool_get_session		(FeedsPool *pool);

#endif /* __FEEDS_POOL_H__ */
