/*
 * Copyright (C) 2010-2012, Roberto Guido <rguido@src.gnome.org>
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

#ifndef __FEEDS_OPML_GROUP_HANDLER_H__
#define __FEEDS_OPML_GROUP_HANDLER_H__

#include "libgrss.h"

#define FEEDS_OPML_GROUP_HANDLER_TYPE		(feeds_opml_group_handler_get_type())
#define FEEDS_OPML_GROUP_HANDLER(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), FEEDS_OPML_GROUP_HANDLER_TYPE, FeedsOpmlGroupHandler))
#define FEEDS_OPML_GROUP_HANDLER_CLASS(c)	(G_TYPE_CHECK_CLASS_CAST ((c), FEEDS_OPML_GROUP_HANDLER_TYPE, FeedsOpmlGroupHandlerClass))
#define IS_FEEDS_OPML_GROUP_HANDLER(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), FEEDS_OPML_GROUP_HANDLER_TYPE))
#define IS_FEEDS_OPML_GROUP_HANDLER_CLASS(c)	(G_TYPE_CHECK_CLASS_TYPE ((c),  FEEDS_OPML_GROUP_HANDLER_TYPE))
#define FEEDS_OPML_GROUP_HANDLER_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), FEEDS_OPML_GROUP_HANDLER_TYPE, FeedsOpmlGroupHandlerClass))

typedef struct FeedsOpmlGroupHandler        FeedsOpmlGroupHandler;
typedef struct FeedsOpmlGroupHandlerPrivate FeedsOpmlGroupHandlerPrivate;

struct FeedsOpmlGroupHandler {
	GObject parent;
	FeedsOpmlGroupHandlerPrivate *priv;
};

typedef struct {
	GObjectClass parent;
} FeedsOpmlGroupHandlerClass;

GType			feeds_opml_group_handler_get_type	(void) G_GNUC_CONST;

FeedsOpmlGroupHandler*	feeds_opml_group_handler_new		();

#endif /* __FEEDS_OPML_GROUP_HANDLER_H__ */
