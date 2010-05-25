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

#ifndef __FEEDS_GROUP_H__
#define __FEEDS_GROUP_H__

#include "libgrss.h"

#define FEEDS_GROUP_TYPE		(feeds_group_get_type())
#define FEEDS_GROUP(o)			(G_TYPE_CHECK_INSTANCE_CAST ((o), FEEDS_GROUP_TYPE, FeedsGroup))
#define FEEDS_GROUP_CLASS(c)		(G_TYPE_CHECK_CLASS_CAST ((c), FEEDS_GROUP_TYPE, FeedsGroupClass))
#define IS_FEEDS_GROUP(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), FEEDS_GROUP_TYPE))
#define IS_FEEDS_GROUP_CLASS(c)		(G_TYPE_CHECK_CLASS_TYPE ((c),  FEEDS_GROUP_TYPE))
#define FEEDS_GROUP_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), FEEDS_GROUP_TYPE, FeedsGroupClass))

typedef struct _FeedsGroup		FeedsGroup;
typedef struct _FeedsGroupPrivate	FeedsGroupPrivate;

struct _FeedsGroup {
	GObject parent;
	FeedsGroupPrivate *priv;
};

typedef struct {
	GObjectClass parent;
} FeedsGroupClass;

GType		feeds_group_get_type		() G_GNUC_CONST;

FeedsGroup*	feeds_group_new			();

GList*		feeds_group_parse_file		(FeedsGroup *group, const gchar *path, GError **error);
gboolean	feeds_group_export_file		(FeedsGroup *group, GList *channels, const gchar *path, GError *error);

#endif /* __FEEDS_GROUP_H__ */
