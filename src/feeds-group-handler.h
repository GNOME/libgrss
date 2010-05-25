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

#ifndef __FEEDS_GROUP_HANDLER_H__
#define __FEEDS_GROUP_HANDLER_H__

#include "libgrss.h"

#define FEEDS_GROUP_HANDLER_TYPE		(feeds_group_handler_get_type ())
#define FEEDS_GROUP_HANDLER(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), FEEDS_GROUP_HANDLER_TYPE, FeedsGroupHandler))
#define IS_FEEDS_GROUP_HANDLER(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), FEEDS_GROUP_HANDLER_TYPE))
#define FEEDS_GROUP_HANDLER_GET_INTERFACE(inst)	(G_TYPE_INSTANCE_GET_INTERFACE ((inst), FEEDS_GROUP_HANDLER_TYPE, FeedsGroupHandlerInterface))

typedef struct _FeedsGroupHandler		FeedsGroupHandler;
typedef struct _FeedsGroupHandlerInterface	FeedsGroupHandlerInterface;

struct _FeedsGroupHandlerInterface {
	GTypeInterface parent_iface;

	gboolean (*check_format) (FeedsGroupHandler *self, xmlDocPtr doc, xmlNodePtr cur);
	GList* (*parse) (FeedsGroupHandler *self, xmlDocPtr doc, GError **error);
	gchar* (*dump) (FeedsGroupHandler *self, GList *channels, GError **error);
};

GType		feeds_group_handler_get_type		();

gboolean	feeds_group_handler_check_format	(FeedsGroupHandler *self, xmlDocPtr doc, xmlNodePtr cur);
GList*		feeds_group_handler_parse		(FeedsGroupHandler *self, xmlDocPtr doc, GError **error);
gchar*		feeds_group_handler_dump		(FeedsGroupHandler *self, GList *channels, GError **error);

#endif /* __FEEDS_GROUP_HANDLER_H__ */
