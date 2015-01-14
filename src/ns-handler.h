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

#ifndef __NS_HANDLER_H__
#define __NS_HANDLER_H__

#include "libgrss.h"

#define NS_HANDLER_TYPE			(ns_handler_get_type())
#define NS_HANDLER(o)			(G_TYPE_CHECK_INSTANCE_CAST ((o), NS_HANDLER_TYPE, NSHandler))
#define NS_HANDLER_CLASS(c)		(G_TYPE_CHECK_CLASS_CAST ((c), NS_HANDLER_TYPE, NSHandlerClass))
#define IS_NS_HANDLER(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), NS_HANDLER_TYPE))
#define IS_NS_HANDLER_CLASS(c)		(G_TYPE_CHECK_CLASS_TYPE ((c),  NS_HANDLER_TYPE))
#define NS_HANDLER_GET_CLASS(o)		(G_TYPE_INSTANCE_GET_CLASS ((o), NS_HANDLER_TYPE, NSHandlerClass))

typedef struct _NSHandler		NSHandler;
typedef struct _NSHandlerPrivate	NSHandlerPrivate;

struct _NSHandler {
	GObject parent;
	NSHandlerPrivate *priv;
};

typedef struct {
	GObjectClass parent;
} NSHandlerClass;

GType		ns_handler_get_type	() G_GNUC_CONST;

NSHandler*	ns_handler_new		();

gboolean	ns_handler_channel	(NSHandler *handler, GrssFeedChannel *feed, xmlNodePtr cur);
gboolean	ns_handler_item		(NSHandler *handler, GrssFeedItem *item, xmlNodePtr cur);

#endif /* __NS_HANDLER_H__ */
