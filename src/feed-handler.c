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

#include "utils.h"
#include "feed-handler.h"

static void
feed_handler_base_init (gpointer g_class)
{
}

GType
feed_handler_get_type ()
{
	static GType iface_type = 0;

	if (iface_type == 0) {
		static const GTypeInfo info = {
			sizeof (FeedHandlerInterface),
			feed_handler_base_init,
			NULL,
		};

		iface_type = g_type_register_static (G_TYPE_INTERFACE, "FeedHandler", &info, 0);
	}

	return iface_type;
}

void
feed_handler_set_ns_handler (FeedHandler *self, NSHandler *handler)
{
	if (IS_FEED_HANDLER (self) == FALSE)
		return;

	return FEED_HANDLER_GET_INTERFACE (self)->set_ns_handler (self, handler);
}

gboolean
feed_handler_check_format (FeedHandler *self, xmlDocPtr doc, xmlNodePtr cur)
{
	if (IS_FEED_HANDLER (self) == FALSE)
		return FALSE;

	return FEED_HANDLER_GET_INTERFACE (self)->check_format (self, doc, cur);
}

GList*
feed_handler_parse (FeedHandler *self, GrssFeedChannel *feed, xmlDocPtr doc, gboolean do_items, GError **error)
{
	if (IS_FEED_HANDLER (self) == FALSE)
		return FALSE;

	return FEED_HANDLER_GET_INTERFACE (self)->parse (self, feed, doc, do_items, error);
}
