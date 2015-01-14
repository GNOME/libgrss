/*
 * Copyright (C) 2010-2015, Roberto Guido <rguido@src.gnome.org>
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

#ifndef __FEEDS_GROUP_HANDLER_H__
#define __FEEDS_GROUP_HANDLER_H__

#include "libgrss.h"

#define FEEDS_GROUP_HANDLER_TYPE		(grss_feeds_group_handler_get_type ())
#define FEEDS_GROUP_HANDLER(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), FEEDS_GROUP_HANDLER_TYPE, GrssFeedsGroupHandler))
#define IS_FEEDS_GROUP_HANDLER(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), FEEDS_GROUP_HANDLER_TYPE))
#define FEEDS_GROUP_HANDLER_GET_INTERFACE(inst)	(G_TYPE_INSTANCE_GET_INTERFACE ((inst), FEEDS_GROUP_HANDLER_TYPE, GrssFeedsGroupHandlerInterface))

typedef struct _GrssFeedsGroupHandler		GrssFeedsGroupHandler;
typedef struct _GrssFeedsGroupHandlerInterface	GrssFeedsGroupHandlerInterface;

struct _GrssFeedsGroupHandlerInterface {
	GTypeInterface parent_iface;

	const gchar* (*get_name) (GrssFeedsGroupHandler *self);
	gboolean (*check_format) (GrssFeedsGroupHandler *self, xmlDocPtr doc, xmlNodePtr cur);
	GList* (*parse) (GrssFeedsGroupHandler *self, xmlDocPtr doc, GError **error);
	gchar* (*dump) (GrssFeedsGroupHandler *self, GList *channels, GError **error);
};

GType		grss_feeds_group_handler_get_type	();

const gchar*	grss_feeds_group_handler_get_name	(GrssFeedsGroupHandler *self);
gboolean	grss_feeds_group_handler_check_format	(GrssFeedsGroupHandler *self, xmlDocPtr doc, xmlNodePtr cur);
GList*		grss_feeds_group_handler_parse		(GrssFeedsGroupHandler *self, xmlDocPtr doc, GError **error);
gchar*		grss_feeds_group_handler_dump		(GrssFeedsGroupHandler *self, GList *channels, GError **error);

#endif /* __FEEDS_GROUP_HANDLER_H__ */
