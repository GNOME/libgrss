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
#include "feeds-xoxo-group-handler.h"
#include "utils.h"
#include "feed-channel.h"

#define FEEDS_XOXO_GROUP_HANDLER_GET_PRIVATE(o)	(G_TYPE_INSTANCE_GET_PRIVATE ((o), FEEDS_XOXO_GROUP_HANDLER_TYPE, FeedsXoxoGroupHandlerPrivate))

/**
 * SECTION: feeds-xoxo-group-handler
 * @short_description: specialized parser for XOXO files
 *
 * #FeedsXoxoGroupHandler is a #GrssFeedsGroupHandler specialized for XOXO contents
 */

struct FeedsXoxoGroupHandlerPrivate {
	int	rfu;
};

static void grss_feeds_group_handler_interface_init (GrssFeedsGroupHandlerInterface *iface);
G_DEFINE_TYPE_WITH_CODE (FeedsXoxoGroupHandler, feeds_xoxo_group_handler, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (FEEDS_GROUP_HANDLER_TYPE,
                                                grss_feeds_group_handler_interface_init));

static xmlNode* find_first_element (xmlDocPtr doc)
{
	xmlNode *ret;
	xmlXPathObjectPtr xpathObj;
	xmlXPathContextPtr xpathCtx;

	xpathCtx = xmlXPathNewContext (doc);
	xmlXPathRegisterNs (xpathCtx, BAD_CAST"xhtml", BAD_CAST"http://www.w3.org/1999/xhtml");
	xpathObj = xmlXPathEvalExpression (BAD_CAST"//xhtml:ol[@class='xoxo']", xpathCtx);

	if (xpathObj->nodesetval == NULL || xpathObj->nodesetval->nodeNr < 1)
		ret = NULL;
	else
		ret = xpathObj->nodesetval->nodeTab [0];

	xmlXPathFreeObject (xpathObj);
	xmlXPathFreeContext (xpathCtx);
	return ret;
}

static void
feeds_xoxo_group_handler_finalize (GObject *object)
{
	FeedsXoxoGroupHandler *parser;

	parser = FEEDS_XOXO_GROUP_HANDLER (object);
	G_OBJECT_CLASS (feeds_xoxo_group_handler_parent_class)->finalize (object);
}

static gboolean
feeds_xoxo_group_handler_check_format (GrssFeedsGroupHandler *self, xmlDocPtr doc, xmlNodePtr cur)
{
	return (find_first_element (doc) != NULL);
}

static GList*
feeds_xoxo_group_handler_parse (GrssFeedsGroupHandler *self, xmlDocPtr doc, GError **error)
{
	int i;
	gchar *str;
	xmlNodePtr cur;
	GList *items;
	xmlXPathObjectPtr xpathObj;
	xmlXPathContextPtr xpathCtx;
	GrssFeedChannel *channel;
	FeedsXoxoGroupHandler *parser;

	items = NULL;
	parser = FEEDS_XOXO_GROUP_HANDLER (self);

	xpathCtx = xmlXPathNewContext (doc);
	xmlXPathRegisterNs (xpathCtx, BAD_CAST"xhtml", BAD_CAST"http://www.w3.org/1999/xhtml");

	/**
		TODO	This XPath query may be improved to check only "a" tags into the main "ol"
	*/
	xpathObj = xmlXPathEvalExpression (BAD_CAST"//xhtml:a[@type='webfeed']", xpathCtx);

	for (i = 0; i < xpathObj->nodesetval->nodeNr; ++i) {
		cur = xpathObj->nodesetval->nodeTab [i];

		if (cur->type == XML_ELEMENT_NODE) {
			str = (gchar*) xmlGetProp (cur, BAD_CAST"href");

			if (str != NULL && strlen (str) != 0) {
				channel = grss_feed_channel_new ();

				grss_feed_channel_set_source (channel, str);
				xmlFree (str);

				str = unhtmlize ((gchar*) xmlNodeListGetString (doc, cur->xmlChildrenNode, TRUE));
				if (str != NULL) {
					grss_feed_channel_set_title (channel, str);
					g_free (str);
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
feeds_xoxo_group_handler_dump (GrssFeedsGroupHandler *self, GList *channels, GError **error)
{
	/**
		TODO
	*/

	return NULL;
}

static void
grss_feeds_group_handler_interface_init (GrssFeedsGroupHandlerInterface *iface)
{
	iface->check_format = feeds_xoxo_group_handler_check_format;
	iface->parse = feeds_xoxo_group_handler_parse;
	iface->dump = feeds_xoxo_group_handler_dump;
}

static void
feeds_xoxo_group_handler_class_init (FeedsXoxoGroupHandlerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (FeedsXoxoGroupHandlerPrivate));
	object_class->finalize = feeds_xoxo_group_handler_finalize;
}

static void
feeds_xoxo_group_handler_init (FeedsXoxoGroupHandler *object)
{
	object->priv = FEEDS_XOXO_GROUP_HANDLER_GET_PRIVATE (object);
}

FeedsXoxoGroupHandler*
feeds_xoxo_group_handler_new ()
{
	FeedsXoxoGroupHandler *parser;

	parser = g_object_new (FEEDS_XOXO_GROUP_HANDLER_TYPE, NULL);
	return parser;
}
