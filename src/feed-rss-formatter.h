/*
 * Copyright (C) 2014, Roberto Guido <rguido@src.gnome.org>
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

#ifndef __GRSS_FEED_RSS_FORMATTER_H__
#define __GRSS_FEED_RSS_FORMATTER_H__

#include "libgrss.h"

#define GRSS_FEED_RSS_FORMATTER_TYPE		(grss_feed_rss_formatter_get_type())
#define GRSS_FEED_RSS_FORMATTER(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), GRSS_FEED_RSS_FORMATTER_TYPE, GrssFeedRssFormatter))
#define GRSS_FEED_RSS_FORMATTER_CLASS(c)	(G_TYPE_CHECK_CLASS_CAST ((c), GRSS_FEED_RSS_FORMATTER_TYPE, GrssFeedRssFormatterClass))
#define GRSS_IS_FEED_RSS_FORMATTER(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), GRSS_FEED_RSS_FORMATTER_TYPE))
#define GRSS_IS_FEED_RSS_FORMATTER_CLASS(c)	(G_TYPE_CHECK_CLASS_TYPE ((c),  GRSS_FEED_RSS_FORMATTER_TYPE))
#define GRSS_FEED_RSS_FORMATTER_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), GRSS_FEED_RSS_FORMATTER_TYPE, GrssFeedRssFormatterClass))

typedef struct GrssFeedRssFormatter        GrssFeedRssFormatter;
typedef struct GrssFeedRssFormatterPrivate GrssFeedRssFormatterPrivate;

struct GrssFeedRssFormatter {
	GrssFeedFormatter parent;
};

typedef struct {
	GrssFeedFormatterClass parent;
} GrssFeedRssFormatterClass;

GType			grss_feed_rss_formatter_get_type	(void) G_GNUC_CONST;

GrssFeedRssFormatter*	grss_feed_rss_formatter_new		();

#endif /* __GRSS_FEED_RSS_FORMATTER_H__ */

