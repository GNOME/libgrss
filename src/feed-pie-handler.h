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

#ifndef __FEED_PIE_HANDLER_H__
#define __FEED_PIE_HANDLER_H__

#include "common.h"
#include "feed-handler.h"

#define FEED_PIE_HANDLER_TYPE		(feed_pie_handler_get_type())
#define FEED_PIE_HANDLER(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), FEED_PIE_HANDLER_TYPE, FeedPieHandler))
#define FEED_PIE_HANDLER_CLASS(c)	(G_TYPE_CHECK_CLASS_CAST ((c), FEED_PIE_HANDLER_TYPE, FeedPieHandlerClass))
#define IS_FEED_PIE_HANDLER(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), FEED_PIE_HANDLER_TYPE))
#define IS_FEED_PIE_HANDLER_CLASS(c)	(G_TYPE_CHECK_CLASS_TYPE ((c),  FEED_PIE_HANDLER_TYPE))
#define FEED_PIE_HANDLER_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), FEED_PIE_HANDLER_TYPE, FeedPieHandlerClass))

typedef struct FeedPieHandler        FeedPieHandler;
typedef struct FeedPieHandlerPrivate FeedPieHandlerPrivate;

struct FeedPieHandler {
	GObject parent;
	FeedPieHandlerPrivate *priv;
};

typedef struct {
	GObjectClass parent;
} FeedPieHandlerClass;

GType		feed_pie_handler_get_type	() G_GNUC_CONST;

FeedPieHandler*	feed_pie_handler_new		();

#endif /* __FEED_PIE_HANDLER_H__ */
