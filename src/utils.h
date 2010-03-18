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

#ifndef __UTILS_LIBGRSS_H__
#define __UTILS_LIBGRSS_H__

#include "common.h"
#include "feed-channel.h"

#define PACKAGE			"libgrss"

#define FREE_STRING(__str)	if (__str) { g_free (__str); __str = NULL; }

gchar*		unhtmlize		(gchar *string);
gchar*		unxmlize		(gchar * string);
gchar*		xhtml_extract		(xmlNodePtr xml, gint xhtmlMode, const gchar *defaultBase);

xmlDocPtr	feed_content_to_xml	(const gchar *contents, gsize size);

time_t		date_parse_RFC822	(const gchar *date);
time_t		date_parse_ISO8601	(const gchar *date);

gboolean	address_seems_public	(GInetAddress *addr);

#endif /* __UTILS_LIBGRSS_H__ */
