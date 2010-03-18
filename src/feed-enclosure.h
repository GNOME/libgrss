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

#ifndef __FEED_ENCLOSURE_H__
#define __FEED_ENCLOSURE_H__

#include "common.h"

#define FEED_ENCLOSURE_TYPE		(feed_enclosure_get_type())
#define FEED_ENCLOSURE(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), FEED_ENCLOSURE_TYPE, FeedEnclosure))
#define FEED_ENCLOSURE_CLASS(c)		(G_TYPE_CHECK_CLASS_CAST ((c), FEED_ENCLOSURE_TYPE, FeedEnclosureClass))
#define IS_FEED_ENCLOSURE(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), FEED_ENCLOSURE_TYPE))
#define IS_FEED_ENCLOSURE_CLASS(c)	(G_TYPE_CHECK_CLASS_TYPE ((c),  FEED_ENCLOSURE_TYPE))
#define FEED_ENCLOSURE_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), FEED_ENCLOSURE_TYPE, FeedEnclosureClass))

typedef struct _FeedEnclosure		FeedEnclosure;
typedef struct _FeedEnclosurePrivate	FeedEnclosurePrivate;

struct _FeedEnclosure {
	GObject parent;
	FeedEnclosurePrivate *priv;
};

typedef struct {
	GObjectClass parent;
} FeedEnclosureClass;

GType		feed_enclosure_get_type		(void) G_GNUC_CONST;

FeedEnclosure*	feed_enclosure_new		(gchar *url);

const gchar*	feed_enclosure_get_url		(FeedEnclosure *enclosure);
void		feed_enclosure_set_format	(FeedEnclosure *enclosure, gchar *type);
const gchar*	feed_enclosure_get_format	(FeedEnclosure *enclosure);
void		feed_enclosure_set_length	(FeedEnclosure *enclosure, gsize length);
gsize		feed_enclosure_get_length	(FeedEnclosure *enclosure);

#endif /* __FEED_ENCLOSURE_H__ */
