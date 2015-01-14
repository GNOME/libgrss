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

#ifndef __GRSS_FEED_ATOM_FORMATTER_H__
#define __GRSS_FEED_ATOM_FORMATTER_H__

#include "libgrss.h"

#define GRSS_FEED_ATOM_FORMATTER_TYPE		(grss_feed_atom_formatter_get_type())
#define GRSS_FEED_ATOM_FORMATTER(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), GRSS_FEED_ATOM_FORMATTER_TYPE, GrssFeedAtomFormatter))
#define GRSS_FEED_ATOM_FORMATTER_CLASS(c)	(G_TYPE_CHECK_CLASS_CAST ((c), GRSS_FEED_ATOM_FORMATTER_TYPE, GrssFeedAtomFormatterClass))
#define GRSS_IS_FEED_ATOM_FORMATTER(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), GRSS_FEED_ATOM_FORMATTER_TYPE))
#define GRSS_IS_FEED_ATOM_FORMATTER_CLASS(c)	(G_TYPE_CHECK_CLASS_TYPE ((c),  GRSS_FEED_ATOM_FORMATTER_TYPE))
#define GRSS_FEED_ATOM_FORMATTER_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), GRSS_FEED_ATOM_FORMATTER_TYPE, GrssFeedAtomFormatterClass))

typedef struct GrssFeedAtomFormatter        GrssFeedAtomFormatter;
typedef struct GrssFeedAtomFormatterPrivate GrssFeedAtomFormatterPrivate;

struct GrssFeedAtomFormatter {
	GrssFeedFormatter parent;
};

typedef struct {
	GrssFeedFormatterClass parent;
} GrssFeedAtomFormatterClass;

GType			grss_feed_atom_formatter_get_type	(void) G_GNUC_CONST;

GrssFeedAtomFormatter*	grss_feed_atom_formatter_new		();

#endif /* __GRSS_FEED_ATOM_FORMATTER_H__ */

