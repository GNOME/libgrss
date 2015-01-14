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

#ifndef __FEEDS_GROUP_H__
#define __FEEDS_GROUP_H__

#include "libgrss.h"

#define FEEDS_GROUP_TYPE		(grss_feeds_group_get_type())
#define FEEDS_GROUP(o)			(G_TYPE_CHECK_INSTANCE_CAST ((o), FEEDS_GROUP_TYPE, GrssFeedsGroup))
#define FEEDS_GROUP_CLASS(c)		(G_TYPE_CHECK_CLASS_CAST ((c), FEEDS_GROUP_TYPE, GrssFeedsGroupClass))
#define IS_FEEDS_GROUP(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), FEEDS_GROUP_TYPE))
#define IS_FEEDS_GROUP_CLASS(c)		(G_TYPE_CHECK_CLASS_TYPE ((c),  FEEDS_GROUP_TYPE))
#define FEEDS_GROUP_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), FEEDS_GROUP_TYPE, GrssFeedsGroupClass))

typedef struct _GrssFeedsGroup		GrssFeedsGroup;
typedef struct _GrssFeedsGroupPrivate	GrssFeedsGroupPrivate;

struct _GrssFeedsGroup {
	GObject parent;
	GrssFeedsGroupPrivate *priv;
};

typedef struct {
	GObjectClass parent;
} GrssFeedsGroupClass;

GType		grss_feeds_group_get_type	() G_GNUC_CONST;

GrssFeedsGroup*	grss_feeds_group_new		();

GList*		grss_feeds_group_get_formats	(GrssFeedsGroup *group);
GList*		grss_feeds_group_parse_file	(GrssFeedsGroup *group, const gchar *path, GError **error);
gboolean	grss_feeds_group_export_file	(GrssFeedsGroup *group, GList *channels, const gchar *format, const gchar *uri, GError **error);

#endif /* __FEEDS_GROUP_H__ */
