/*
 * Copyright (C) 2010-2012, Roberto Guido <rguido@src.gnome.org>
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

/*
 * Original code is from Liferea:
 *
 * opml_source.c  OPML Planet/Blogroll feed list source
 *
 * Copyright (C) 2006-2010 Lars Lindner <lars.lindner@gmail.com>
 */

#include "feeds-group-handler.h"
#include "feeds-opml-group-handler.h"
#include "utils.h"
#include "feed-channel.h"

#define FEEDS_OPML_GROUP_HANDLER_GET_PRIVATE(o)	(G_TYPE_INSTANCE_GET_PRIVATE ((o), FEEDS_OPML_GROUP_HANDLER_TYPE, FeedsOpmlGroupHandlerPrivate))

struct FeedsOpmlGroupHandlerPrivate {
	int	rfu;
};

static void grss_feeds_group_handler_interface_init (GrssFeedsGroupHandlerInterface *iface);
G_DEFINE_TYPE_WITH_CODE (FeedsOpmlGroupHandler, feeds_opml_group_handler, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (FEEDS_GROUP_HANDLER_TYPE,
                                                grss_feeds_group_handler_interface_init));

static void
feeds_opml_group_handler_finalize (GObject *object)
{
	G_OBJECT_CLASS (feeds_opml_group_handler_parent_class)->finalize (object);
}

static const gchar*
feeds_opml_group_handler_get_name (GrssFeedsGroupHandler *self)
{
	return "OPML";
}

static gboolean
feeds_opml_group_handler_check_format (GrssFeedsGroupHandler *self, xmlDocPtr doc, xmlNodePtr cur)
{
	if (!xmlStrcmp (cur->name, BAD_CAST"opml"))
		return TRUE;
	else
		return FALSE;
}

static xmlChar*
get_source_url (xmlNodePtr cur)
{
	xmlChar *tmp;

	tmp = xmlGetProp (cur, BAD_CAST "xmlUrl");
	if (!tmp)
		tmp = xmlGetProp (cur, BAD_CAST "xmlurl");	/* e.g. for AmphetaDesk */
	if (!tmp)
		tmp = xmlGetProp (cur, BAD_CAST"xmlURL");	/* e.g. for LiveJournal */

	return tmp;
}

static GrssFeedChannel*
import_parse_outline (xmlNodePtr cur)
{
	xmlChar *tmp;
	GrssFeedChannel *channel;

	channel = grss_feed_channel_new ();

	tmp = xmlGetProp (cur, BAD_CAST"title");
	if (!tmp || !xmlStrcmp (tmp, BAD_CAST"")) {
		if (tmp)
			xmlFree (tmp);
		tmp = xmlGetProp (cur, BAD_CAST"text");
	}

	if (tmp) {
		grss_feed_channel_set_title (channel, (gchar*) tmp);
		xmlFree (tmp);
	}

	tmp = get_source_url (cur);

	if (tmp) {
		grss_feed_channel_set_source (channel, (gchar*) tmp);
		xmlFree (tmp);

		tmp = xmlGetProp (cur, BAD_CAST"htmlUrl");
		if (tmp && xmlStrcmp (tmp, BAD_CAST""))
			grss_feed_channel_set_homepage (channel, (gchar*) tmp);
		xmlFree (tmp);
	}

	return channel;
}

static GList*
import_parse_body (xmlNodePtr n)
{
	xmlChar *type;
	xmlChar *tmp;
	GList *items;
	GList *subitems;
	GrssFeedChannel *outline;
	xmlNodePtr cur;

	cur = n->xmlChildrenNode;
	items = NULL;

	while (cur) {
		if (!xmlStrcmp (cur->name, BAD_CAST"outline")) {
			outline = NULL;
			subitems = NULL;
			type = xmlGetProp (cur, BAD_CAST"type");

			if (type) {
				if (xmlStrcasecmp (type, BAD_CAST"rss") == 0 || xmlStrcasecmp (type, BAD_CAST"atom") == 0)
					outline = import_parse_outline (cur);
				else if (xmlStrcasecmp (type, BAD_CAST"folder") == 0)
					subitems = import_parse_body (cur);

				xmlFree (type);
			}
			else {
				/* if we didn't find a type attribute we use heuristics */

				tmp = get_source_url (cur);

				if (tmp) {
					outline = import_parse_outline (cur);
					xmlFree (tmp);
				}
				else {
					subitems = import_parse_body (cur);
				}
			}

			if (outline != NULL)
				items = g_list_prepend (items, outline);
			else if (subitems != NULL)
				items = g_list_concat (items, subitems);
		}

		cur = cur->next;
	}

	return items;
}

static GList*
import_parse_OPML (xmlNodePtr n)
{
	GList *items;
	xmlNodePtr cur;

	cur = n->xmlChildrenNode;
	items = NULL;

	while (cur) {
		if (!xmlStrcmp (cur->name, BAD_CAST"body")) {
			items = import_parse_body (cur);
			break;
		}

		cur = cur->next;
	}

	return items;
}

static GList*
feeds_opml_group_handler_parse (GrssFeedsGroupHandler *self, xmlDocPtr doc, GError **error)
{
	xmlNodePtr cur;
	GList *items;

	items = NULL;
	cur = xmlDocGetRootElement (doc);

	while (cur) {
		if (!xmlIsBlankNode (cur))
			if (!xmlStrcmp (cur->name, BAD_CAST"opml")) {
				items = import_parse_OPML (cur);
				break;
			}

		cur = cur->next;
	}

	if (items != NULL)
		items = g_list_reverse (items);
	return items;
}

static gchar*
feeds_opml_group_handler_dump (GrssFeedsGroupHandler *self, GList *channels, GError **error)
{
	int size;
	xmlChar *ret;
	xmlDocPtr doc;
	xmlNodePtr cur;
	xmlNodePtr opmlNode;
	xmlNodePtr childNode;
	GList *iter;
	GrssFeedChannel *channel;

	doc = xmlNewDoc (BAD_CAST"1.0");

	opmlNode = xmlNewDocNode (doc, NULL, BAD_CAST"opml", NULL);
	xmlNewProp (opmlNode, BAD_CAST"version", BAD_CAST"1.0");
	xmlNewChild (opmlNode, NULL, BAD_CAST"head", NULL);

	cur = xmlNewChild (opmlNode, NULL, BAD_CAST"body", NULL);
	if (cur) {
		for (iter = channels; iter; iter = g_list_next (iter)) {
			channel = (GrssFeedChannel*) iter->data;
			childNode = xmlNewChild (cur, NULL, BAD_CAST"outline", NULL);
			xmlNewProp (childNode, BAD_CAST"text", BAD_CAST grss_feed_channel_get_title (channel));
			xmlNewProp (childNode, BAD_CAST"type", BAD_CAST"rss");
			xmlNewProp (childNode, BAD_CAST"xmlUrl", BAD_CAST grss_feed_channel_get_source (channel));
		}
	}

	xmlDocSetRootElement (doc, opmlNode);
	xmlDocDumpFormatMemoryEnc (doc, &ret, &size, "utf-8", 1);
	xmlFreeDoc (doc);

	return (gchar*) ret;
}

static void
grss_feeds_group_handler_interface_init (GrssFeedsGroupHandlerInterface *iface)
{
	iface->get_name = feeds_opml_group_handler_get_name;
	iface->check_format = feeds_opml_group_handler_check_format;
	iface->parse = feeds_opml_group_handler_parse;
	iface->dump = feeds_opml_group_handler_dump;
}

static void
feeds_opml_group_handler_class_init (FeedsOpmlGroupHandlerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (FeedsOpmlGroupHandlerPrivate));
	object_class->finalize = feeds_opml_group_handler_finalize;
}

static void
feeds_opml_group_handler_init (FeedsOpmlGroupHandler *object)
{
	object->priv = FEEDS_OPML_GROUP_HANDLER_GET_PRIVATE (object);
}

FeedsOpmlGroupHandler*
feeds_opml_group_handler_new ()
{
	FeedsOpmlGroupHandler *parser;

	parser = g_object_new (FEEDS_OPML_GROUP_HANDLER_TYPE, NULL);
	return parser;
}
