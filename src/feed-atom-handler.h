/*
 * Copyright (C) 2009-2012, Roberto Guido <rguido@src.gnome.org>
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

#ifndef __FEED_ATOM_HANDLER_H__
#define __FEED_ATOM_HANDLER_H__

#include "libgrss.h"

#define FEED_ATOM_HANDLER_TYPE		(feed_atom_handler_get_type())
#define FEED_ATOM_HANDLER(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), FEED_ATOM_HANDLER_TYPE, FeedAtomHandler))
#define FEED_ATOM_HANDLER_CLASS(c)	(G_TYPE_CHECK_CLASS_CAST ((c), FEED_ATOM_HANDLER_TYPE, FeedAtomHandlerClass))
#define IS_FEED_ATOM_HANDLER(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), FEED_ATOM_HANDLER_TYPE))
#define IS_FEED_ATOM_HANDLER_CLASS(c)	(G_TYPE_CHECK_CLASS_TYPE ((c),  FEED_ATOM_HANDLER_TYPE))
#define FEED_ATOM_HANDLER_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), FEED_ATOM_HANDLER_TYPE, FeedAtomHandlerClass))

typedef struct FeedAtomHandler        FeedAtomHandler;
typedef struct FeedAtomHandlerPrivate FeedAtomHandlerPrivate;

struct FeedAtomHandler {
	GObject parent;
	FeedAtomHandlerPrivate *priv;
};

typedef struct {
	GObjectClass parent;
} FeedAtomHandlerClass;

GType			feed_atom_handler_get_type	(void) G_GNUC_CONST;

FeedAtomHandler*	feed_atom_handler_new		();

#endif /* __FEED_ATOM_HANDLER_H__ */
