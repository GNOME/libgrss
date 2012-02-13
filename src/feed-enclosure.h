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

#ifndef __FEED_ENCLOSURE_H__
#define __FEED_ENCLOSURE_H__

#include "libgrss.h"

#define GRSS_FEED_ENCLOSURE_TYPE		(grss_feed_enclosure_get_type())
#define GRSS_FEED_ENCLOSURE(o)			(G_TYPE_CHECK_INSTANCE_CAST ((o), GRSS_FEED_ENCLOSURE_TYPE, GrssFeedEnclosure))
#define GRSS_FEED_ENCLOSURE_CLASS(c)		(G_TYPE_CHECK_CLASS_CAST ((c), GRSS_FEED_ENCLOSURE_TYPE, GrssFeedEnclosureClass))
#define GRSS_IS_FEED_ENCLOSURE(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), GRSS_FEED_ENCLOSURE_TYPE))
#define GRSS_IS_FEED_ENCLOSURE_CLASS(c)		(G_TYPE_CHECK_CLASS_TYPE ((c),  GRSS_FEED_ENCLOSURE_TYPE))
#define GRSS_FEED_ENCLOSURE_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), GRSS_FEED_ENCLOSURE_TYPE, GrssFeedEnclosureClass))

typedef struct _GrssFeedEnclosure		GrssFeedEnclosure;
typedef struct _GrssFeedEnclosurePrivate	GrssFeedEnclosurePrivate;

struct _GrssFeedEnclosure {
	GObject parent;
	GrssFeedEnclosurePrivate *priv;
};

typedef struct {
	GObjectClass parent;
} GrssFeedEnclosureClass;

GType			grss_feed_enclosure_get_type	(void) G_GNUC_CONST;

GrssFeedEnclosure*	grss_feed_enclosure_new		(gchar *url);

const gchar*		grss_feed_enclosure_get_url	(GrssFeedEnclosure *enclosure);
void			grss_feed_enclosure_set_format	(GrssFeedEnclosure *enclosure, gchar *type);
const gchar*		grss_feed_enclosure_get_format	(GrssFeedEnclosure *enclosure);
void			grss_feed_enclosure_set_length	(GrssFeedEnclosure *enclosure, gsize length);
gsize			grss_feed_enclosure_get_length	(GrssFeedEnclosure *enclosure);

#endif /* __FEED_ENCLOSURE_H__ */
