/*
 * Copyright (C) 2015, Roberto Guido <rguido@src.gnome.org>
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

#include "libgrss.h"

#ifndef __FEED_FORMATTER_H__
#define __FEED_FORMATTER_H__

#define GRSS_FEED_FORMATTER_TYPE		(grss_feed_formatter_get_type())
#define GRSS_FEED_FORMATTER(o)			(G_TYPE_CHECK_INSTANCE_CAST ((o), GRSS_FEED_FORMATTER_TYPE, GrssFeedFormatter))
#define GRSS_FEED_FORMATTER_CLASS(c)		(G_TYPE_CHECK_CLASS_CAST ((c), GRSS_FEED_FORMATTER_TYPE, GrssFeedFormatterClass))
#define GRSS_IS_FEED_FORMATTER(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), GRSS_FEED_FORMATTER_TYPE))
#define GRSS_IS_FEED_FORMATTER_CLASS(c)		(G_TYPE_CHECK_CLASS_TYPE ((c),  GRSS_FEED_FORMATTER_TYPE))
#define GRSS_FEED_FORMATTER_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), GRSS_FEED_FORMATTER_TYPE, GrssFeedFormatterClass))

typedef struct _GrssFeedFormatter		GrssFeedFormatter;
typedef struct _GrssFeedFormatterPrivate	GrssFeedFormatterPrivate;

struct _GrssFeedFormatter {
	GObject parent;
	GrssFeedFormatterPrivate *priv;
};

typedef struct {
	GObjectClass parent;

	gchar* (*format) (GrssFeedFormatter *formatter);
} GrssFeedFormatterClass;

GType		grss_feed_formatter_get_type	() G_GNUC_CONST;

void			grss_feed_formatter_set_channel	(GrssFeedFormatter *formatter, GrssFeedChannel *channel);
GrssFeedChannel*	grss_feed_formatter_get_channel	(GrssFeedFormatter *formatter);
void			grss_feed_formatter_add_item	(GrssFeedFormatter *formatter, GrssFeedItem *item);
void			grss_feed_formatter_add_items	(GrssFeedFormatter *formatter, GList *items);
GList*			grss_feed_formatter_get_items	(GrssFeedFormatter *formatter);
void			grss_feed_formatter_reset	(GrssFeedFormatter *formatter);
gchar*			grss_feed_formatter_format	(GrssFeedFormatter *formatter);

#endif /* __FEED_FORMATTER_H__ */
