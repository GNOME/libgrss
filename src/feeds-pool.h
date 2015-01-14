/*
 * Copyright (C) 2009-2015, Roberto Guido <rguido@src.gnome.org>
 *                          Michele Tameni <michele@amdplanet.it>
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

#include "libgrss.h"

#define GRSS_FEEDS_POOL_TYPE		(grss_feeds_pool_get_type())
#define GRSS_FEEDS_POOL(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), GRSS_FEEDS_POOL_TYPE, GrssFeedsPool))
#define GRSS_FEEDS_POOL_CLASS(c)	(G_TYPE_CHECK_CLASS_CAST ((c), GRSS_FEEDS_POOL_TYPE, GrssFeedsPoolClass))
#define GRSS_IS_FEEDS_POOL(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), GRSS_FEEDS_POOL_TYPE))
#define GRSS_IS_FEEDS_POOL_CLASS(c)	(G_TYPE_CHECK_CLASS_TYPE ((c),  GRSS_FEEDS_POOL_TYPE))
#define GRSS_FEEDS_POOL_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), GRSS_FEEDS_POOL_TYPE, GrssFeedsPoolClass))

typedef struct _GrssFeedsPool		GrssFeedsPool;
typedef struct _GrssFeedsPoolPrivate	GrssFeedsPoolPrivate;

struct _GrssFeedsPool {
	GObject parent;
	GrssFeedsPoolPrivate *priv;
};

typedef struct {
	GObjectClass parent;

	void (*feed_fetching) (GrssFeedsPool *pool, GrssFeedChannel *feed);
	void (*feed_ready) (GrssFeedsPool *pool, GrssFeedChannel *feed, GList *items);
} GrssFeedsPoolClass;

GType		grss_feeds_pool_get_type		() G_GNUC_CONST;

GrssFeedsPool*	grss_feeds_pool_new			();

void		grss_feeds_pool_listen			(GrssFeedsPool *pool, GList *feeds);
GList*		grss_feeds_pool_get_listened		(GrssFeedsPool *pool);
int		grss_feeds_pool_get_listened_num	(GrssFeedsPool *pool);
void		grss_feeds_pool_switch			(GrssFeedsPool *pool, gboolean run);
SoupSession*	grss_feeds_pool_get_session		(GrssFeedsPool *pool);

#endif /* __FEEDS_POOL_H__ */
