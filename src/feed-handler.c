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

#include "utils.h"
#include "feed-handler.h"

/**
 * SECTION: feed-handler
 * @short_description: interface for specialized parsers
 *
 * The #FeedHandler interface defines a unique API for all specialized
 * parsers implementations
 */

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

/**
 * feed_handler_set_ns_handler:
 * @self: a #FeedHandler
 * @handler: instance of #NSHandler
 *
 * Permits to assign a #NSHandler to the specified #FeedHandler, to expand
 * his parsing capabilities to the external managed tags
 */
void
feed_handler_set_ns_handler (FeedHandler *self, NSHandler *handler)
{
	if (IS_FEED_HANDLER (self) == FALSE)
		return;

	return FEED_HANDLER_GET_INTERFACE (self)->set_ns_handler (self, handler);
}

/**
 * feed_handler_check_format:
 * @self: a #FeedHandler
 * @doc: XML document from a parsed feed
 * @cur: first valid  node into the XML document
 *
 * Used to check validity of an XML document against the given feed parser
 *
 * Return value: %TRUE if the document can be parsed with the given
 * #FeedHandler, %FALSE otherwise
 */
gboolean
feed_handler_check_format (FeedHandler *self, xmlDocPtr doc, xmlNodePtr cur)
{
	if (IS_FEED_HANDLER (self) == FALSE)
		return FALSE;

	return FEED_HANDLER_GET_INTERFACE (self)->check_format (self, doc, cur);
}

/**
 * feed_handler_parse:
 * @self: a #FeedHandler
 * @feed: feed to be parsed
 * @doc: XML document from the feed
 * @error: location for eventual errors
 *
 * Parses the given @doc (obtained fetching @feed) and extracts a list of
 * items
 *
 * Return value: a list of #FeedItem, to be freed when no longer in use, or
 * %NULL if an error occours (and @error is set accordly)
 */
GList*
feed_handler_parse (FeedHandler *self, FeedChannel *feed, xmlDocPtr doc, GError **error)
{
	if (IS_FEED_HANDLER (self) == FALSE)
		return FALSE;

	return FEED_HANDLER_GET_INTERFACE (self)->parse (self, feed, doc, error);
}
