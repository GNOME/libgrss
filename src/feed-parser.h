/*
 * Copyright (C) 2009, Roberto Guido <rguido@src.gnome.org>
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

#ifndef __FEED_PARSER_H__
#define __FEED_PARSER_H__

#include "libgrss.h"

#define FEED_PARSER_TYPE		(feed_parser_get_type())
#define FEED_PARSER(o)			(G_TYPE_CHECK_INSTANCE_CAST ((o), FEED_PARSER_TYPE, FeedParser))
#define FEED_PARSER_CLASS(c)		(G_TYPE_CHECK_CLASS_CAST ((c), FEED_PARSER_TYPE, FeedParserClass))
#define IS_FEED_PARSER(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), FEED_PARSER_TYPE))
#define IS_FEED_PARSER_CLASS(c)		(G_TYPE_CHECK_CLASS_TYPE ((c),  FEED_PARSER_TYPE))
#define FEED_PARSER_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), FEED_PARSER_TYPE, FeedParserClass))

typedef struct _FeedParser		FeedParser;
typedef struct _FeedParserPrivate	FeedParserPrivate;

struct _FeedParser {
	GObject parent;
	FeedParserPrivate *priv;
};

typedef struct {
	GObjectClass parent;
} FeedParserClass;

GType		feed_parser_get_type	() G_GNUC_CONST;

FeedParser*	feed_parser_new		();

GList*		feed_parser_parse	(FeedParser *parser, FeedChannel *feed, xmlDocPtr doc, GError **error);

#endif /* __FEED_PARSER_H__ */
