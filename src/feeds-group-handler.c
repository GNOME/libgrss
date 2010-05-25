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

#include "utils.h"
#include "feeds-group-handler.h"

/**
 * SECTION: feeds-group-handler
 * @short_description: interface for specialized groups parsers
 *
 * The #FeedsGroupHandler interface defines a unique API for all specialized
 * groups parsers implementations
 */

static void
feeds_group_handler_base_init (gpointer g_class)
{
}

GType
feeds_group_handler_get_type ()
{
	static GType iface_type = 0;

	if (iface_type == 0) {
		static const GTypeInfo info = {
			sizeof (FeedsGroupHandlerInterface),
			feeds_group_handler_base_init,
			NULL,
		};

		iface_type = g_type_register_static (G_TYPE_INTERFACE, "FeedsGroupHandler", &info, 0);
	}

	return iface_type;
}

/**
 * feeds_group_handler_check_format:
 * @self: a #FeedsGroupHandler
 * @doc: XML document from a parsed feed
 * @cur: first valid  node into the XML document
 *
 * Used to check validity of an XML document against the given group parser
 *
 * Return value: %TRUE if the document can be parsed with the given
 * #FeedsGroupHandler, %FALSE otherwise
 */
gboolean
feeds_group_handler_check_format (FeedsGroupHandler *self, xmlDocPtr doc, xmlNodePtr cur)
{
	if (IS_FEEDS_GROUP_HANDLER (self) == FALSE)
		return FALSE;

	return FEEDS_GROUP_HANDLER_GET_INTERFACE (self)->check_format (self, doc, cur);
}

/**
 * feeds_group_handler_parse:
 * @self: a #FeedsGroupHandler
 * @doc: XML document from the feed
 * @error: location for eventual errors
 *
 * Parses the given @doc and extracts a list of #FeedChannels
 *
 * Return value: a list of #FeedChannels, to be freed when no longer in use,
 * or %NULL if an error occours (and @error is set accordly)
 */
GList*
feeds_group_handler_parse (FeedsGroupHandler *self, xmlDocPtr doc, GError **error)
{
	if (IS_FEEDS_GROUP_HANDLER (self) == FALSE)
		return FALSE;

	return FEEDS_GROUP_HANDLER_GET_INTERFACE (self)->parse (self, doc, error);
}

/**
 * feeds_group_handler_dump:
 * @self: a #FeedsGroupHandler
 * @channels: list of #FeedChannels
 * @error: location for eventual errors
 *
 * Builds a rappresentation of the given list of @channels for the managed
 * group type
 *
 * Return value: a text to be dump on a file or transmitted, to be freed when
 * no longer in use, or %NULL if an error occours (and @error is set accordly)
 */
gchar*
feeds_group_handler_dump (FeedsGroupHandler *self, GList *channels, GError **error)
{
	if (IS_FEEDS_GROUP_HANDLER (self) == FALSE)
		return FALSE;

	return FEEDS_GROUP_HANDLER_GET_INTERFACE (self)->dump (self, channels, error);
}
