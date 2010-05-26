/*
 * Copyright (C) 2010, Roberto Guido <rguido@src.gnome.org>
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

#ifndef __FEEDS_XBEL_GROUP_HANDLER_H__
#define __FEEDS_XBEL_GROUP_HANDLER_H__

#include "libgrss.h"

#define FEEDS_XBEL_GROUP_HANDLER_TYPE		(feeds_xbel_group_handler_get_type())
#define FEEDS_XBEL_GROUP_HANDLER(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), FEEDS_XBEL_GROUP_HANDLER_TYPE, FeedsXbelGroupHandler))
#define FEEDS_XBEL_GROUP_HANDLER_CLASS(c)	(G_TYPE_CHECK_CLASS_CAST ((c), FEEDS_XBEL_GROUP_HANDLER_TYPE, FeedsXbelGroupHandlerClass))
#define IS_FEEDS_XBEL_GROUP_HANDLER(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), FEEDS_XBEL_GROUP_HANDLER_TYPE))
#define IS_FEEDS_XBEL_GROUP_HANDLER_CLASS(c)	(G_TYPE_CHECK_CLASS_TYPE ((c),  FEEDS_XBEL_GROUP_HANDLER_TYPE))
#define FEEDS_XBEL_GROUP_HANDLER_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), FEEDS_XBEL_GROUP_HANDLER_TYPE, FeedsXbelGroupHandlerClass))

typedef struct FeedsXbelGroupHandler        FeedsXbelGroupHandler;
typedef struct FeedsXbelGroupHandlerPrivate FeedsXbelGroupHandlerPrivate;

struct FeedsXbelGroupHandler {
	GObject parent;
	FeedsXbelGroupHandlerPrivate *priv;
};

typedef struct {
	GObjectClass parent;
} FeedsXbelGroupHandlerClass;

GType			feeds_xbel_group_handler_get_type	(void) G_GNUC_CONST;

FeedsXbelGroupHandler*	feeds_xbel_group_handler_new		();

#endif /* __FEEDS_XBEL_GROUP_HANDLER_H__ */
