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

#ifndef __FEED_HANDLER_H__
#define __FEED_HANDLER_H__

#include "libgrss.h"
#include "ns-handler.h"

#define FEED_HANDLER_TYPE			(feed_handler_get_type ())
#define FEED_HANDLER(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), FEED_HANDLER_TYPE, FeedHandler))
#define IS_FEED_HANDLER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), FEED_HANDLER_TYPE))
#define FEED_HANDLER_GET_INTERFACE(inst)	(G_TYPE_INSTANCE_GET_INTERFACE ((inst), FEED_HANDLER_TYPE, FeedHandlerInterface))

typedef struct _FeedHandler		FeedHandler;
typedef struct _FeedHandlerInterface	FeedHandlerInterface;

struct _FeedHandlerInterface {
	GTypeInterface parent_iface;

	void (*set_ns_handler) (FeedHandler *self, NSHandler *handler);
	gboolean (*check_format) (FeedHandler *self, xmlDocPtr doc, xmlNodePtr cur);
	GList* (*parse) (FeedHandler *self, GrssFeedChannel *feed, xmlDocPtr doc, gboolean do_items, GError **error);
};

GType		feed_handler_get_type		();

void		feed_handler_set_ns_handler	(FeedHandler *self, NSHandler *handler);
gboolean	feed_handler_check_format	(FeedHandler *self, xmlDocPtr doc, xmlNodePtr cur);
GList*		feed_handler_parse		(FeedHandler *self, GrssFeedChannel *feed, xmlDocPtr doc, gboolean do_items, GError **error);

#endif /* __FEED_HANDLER_H__ */
