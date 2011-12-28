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

#include "feeds-group-handler.h"
#include "feeds-xbel-group-handler.h"
#include "utils.h"
#include "feed-channel.h"

#define FEEDS_XBEL_GROUP_HANDLER_GET_PRIVATE(o)	(G_TYPE_INSTANCE_GET_PRIVATE ((o), FEEDS_XBEL_GROUP_HANDLER_TYPE, FeedsXbelGroupHandlerPrivate))

struct FeedsXbelGroupHandlerPrivate {
	int	rfu;
};

static void grss_feeds_group_handler_interface_init (GrssFeedsGroupHandlerInterface *iface);
G_DEFINE_TYPE_WITH_CODE (FeedsXbelGroupHandler, feeds_xbel_group_handler, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (FEEDS_GROUP_HANDLER_TYPE,
                                                grss_feeds_group_handler_interface_init));

static void
feeds_xbel_group_handler_finalize (GObject *object)
{
	G_OBJECT_CLASS (feeds_xbel_group_handler_parent_class)->finalize (object);
}

static gboolean
feeds_xbel_group_handler_check_format (GrssFeedsGroupHandler *self, xmlDocPtr doc, xmlNodePtr cur)
{
	if (!xmlStrcmp (cur->name, BAD_CAST"xbel"))
		return TRUE;
	else
		return FALSE;
}

static GList*
feeds_xbel_group_handler_parse (GrssFeedsGroupHandler *self, xmlDocPtr doc, GError **error)
{
	int i;
	gchar *str;
	xmlNodePtr cur;
	xmlNodePtr subnode;
	GList *items;
	xmlXPathObjectPtr xpathObj;
	xmlXPathContextPtr xpathCtx;
	GrssFeedChannel *channel;

	items = NULL;

	xpathCtx = xmlXPathNewContext (doc);

	/*
		TODO	This XPath query may be improved to check only "bookmark" tags into the main "xbel"
	*/
	xpathObj = xmlXPathEvalExpression (BAD_CAST"//bookmark", xpathCtx);

	for (i = 0; i < xpathObj->nodesetval->nodeNr; ++i) {
		cur = xpathObj->nodesetval->nodeTab [i];

		if (cur->type == XML_ELEMENT_NODE) {
			str = (gchar*) xmlGetProp (cur, BAD_CAST"href");

			if (str != NULL && strlen (str) != 0) {
				channel = grss_feed_channel_new ();

				grss_feed_channel_set_source (channel, str);
				xmlFree (str);

				if (cur->children != NULL && strcmp ((gchar*) cur->children->name, "title") == 0) {
					subnode = cur->children;
					str = (gchar*) xmlNodeListGetString (doc, subnode->xmlChildrenNode, TRUE);
					if (str != NULL) {
						grss_feed_channel_set_title (channel, str);
						g_free (str);
					}
				}

				items = g_list_prepend (items, channel);
			}
		}
	}

	xmlXPathFreeObject (xpathObj);
	xmlXPathFreeContext (xpathCtx);

	if (items != NULL)
		items = g_list_reverse (items);
	return items;
}

static gchar*
feeds_xbel_group_handler_dump (GrssFeedsGroupHandler *self, GList *channels, GError **error)
{
	/**
		TODO
	*/

	return NULL;
}

static void
grss_feeds_group_handler_interface_init (GrssFeedsGroupHandlerInterface *iface)
{
	iface->check_format = feeds_xbel_group_handler_check_format;
	iface->parse = feeds_xbel_group_handler_parse;
	iface->dump = feeds_xbel_group_handler_dump;
}

static void
feeds_xbel_group_handler_class_init (FeedsXbelGroupHandlerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (FeedsXbelGroupHandlerPrivate));
	object_class->finalize = feeds_xbel_group_handler_finalize;
}

static void
feeds_xbel_group_handler_init (FeedsXbelGroupHandler *object)
{
	object->priv = FEEDS_XBEL_GROUP_HANDLER_GET_PRIVATE (object);
}

FeedsXbelGroupHandler*
feeds_xbel_group_handler_new ()
{
	FeedsXbelGroupHandler *parser;

	parser = g_object_new (FEEDS_XBEL_GROUP_HANDLER_TYPE, NULL);
	return parser;
}
