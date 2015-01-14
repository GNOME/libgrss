/*
 * Copyright (C) 2009-2015, Roberto Guido <rguido@src.gnome.org>
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
 * pie_feed.c Atom 0.3 channel parsing
 * Copyright (C) 2003-2009 Lars Lindner <lars.lindner@gmail.com>
 */

#include "feed-handler.h"
#include "feed-pie-handler.h"
#include "utils.h"
#include "feed-channel.h"
#include "feed-item.h"
#include "feed-enclosure.h"
#include "ns-handler.h"

#define FEED_PIE_HANDLER_GET_PRIVATE(o)	(G_TYPE_INSTANCE_GET_PRIVATE ((o), FEED_PIE_HANDLER_TYPE, FeedPieHandlerPrivate))
#define FEED_PIE_HANDLER_ERROR			feed_pie_handler_error_quark()

typedef void 	(*PieChannelParserFunc)		(xmlNodePtr cur, GrssFeedChannel *feed);
typedef void 	(*PieItemParserFunc)		(xmlNodePtr cur, GrssFeedItem *item, GrssFeedChannel *feed);

struct FeedPieHandlerPrivate {
	NSHandler	*handler;
};

enum {
	FEED_PIE_HANDLER_PARSE_ERROR,
};

static void feed_handler_interface_init (FeedHandlerInterface *iface);
G_DEFINE_TYPE_WITH_CODE (FeedPieHandler, feed_pie_handler, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (FEED_HANDLER_TYPE,
                                                feed_handler_interface_init));

static GQuark
feed_pie_handler_error_quark ()
{
	return g_quark_from_static_string ("feed_pie_handler_error");
}

static void
feed_pie_handler_finalize (GObject *object)
{
	G_OBJECT_CLASS (feed_pie_handler_parent_class)->finalize (object);
}

static void
feed_pie_handler_set_ns_handler (FeedHandler *self, NSHandler *handler)
{
	FeedPieHandler *pie;

	pie = FEED_PIE_HANDLER (self);
	pie->priv->handler = handler;
}

static gboolean
feed_pie_handler_check_format (FeedHandler *self, xmlDocPtr doc, xmlNodePtr cur)
{
	if (!xmlStrcmp (cur->name, BAD_CAST "feed"))
		return TRUE;
	else
		return FALSE;
}

static gchar*
pie_parse_content_construct (xmlNodePtr cur) {
	gchar *mode;
	gchar *type;
	gchar *ret;

	g_assert (NULL != cur);
	ret = NULL;

	/* determine encoding mode */
	mode = (gchar*) xmlGetProp (cur, BAD_CAST"mode");
	type = (gchar*) xmlGetProp (cur, BAD_CAST"type");

	/* Modes are used in older versions of ATOM, including 0.3. It
	   does not exist in the newer IETF drafts.*/
	if (NULL != mode) {
		if (!strcmp (mode, "escaped")) {
			gchar *tmp;

			tmp = xhtml_extract (cur, 0, NULL);
			if (NULL != tmp)
				ret = tmp;
		}
		else if (!strcmp (mode, "xml")) {
			ret = xhtml_extract (cur, 1,NULL);
		}
		else if (!strcmp (mode, "base64")) {
			g_warning ("Base64 encoded <content> in Atom feeds not supported!");
		}
		else if (!strcmp (mode, "multipart/alternative")) {
			if (NULL != cur->xmlChildrenNode)
				ret = pie_parse_content_construct (cur->xmlChildrenNode);
		}

		g_free (mode);
	}
	else {
		/* some feeds don'ts specify a mode but a MIME type in the
		   type attribute... */
		/* not sure what MIME types are necessary... */

		/* This that need to be de-encoded and should not contain sub-tags.*/
		if (NULL == type || !g_ascii_strcasecmp (type, "TEXT") || !strcmp (type, "text/plain")) {
			gchar *tmp;
			tmp = (gchar*) xmlNodeListGetString (cur->doc, cur->xmlChildrenNode, 1);
			ret = g_markup_printf_escaped ("<div xmlns=\"http://www.w3.org/1999/xhtml\"><pre>%s</pre></div>", tmp);
			g_free (tmp);
			/* Next are things that contain subttags */
		}
		else if (!g_ascii_strcasecmp (type, "HTML") || !strcmp(type, "text/html")) {
			ret = xhtml_extract (cur, 0,"http://default.base.com/");
		}
		/* HTML types */
		else if (!g_ascii_strcasecmp(type, "xhtml") || !strcmp(type, "application/xhtml+xml")) {
			ret = xhtml_extract (cur, 1,"http://default.base.com/");
		}
	}

	/* If the type was text, everything must be now escaped and
	   wrapped in pre tags.... Also, the atom 0.3 spec says that the
	   default type MUST be considered to be text/plain. The type tag
	   is required in 0.2.... */
	//if (ret != NULL && (type == NULL || !strcmp(type, "text/plain") || !strcmp(type,"TEXT")))) {
	g_free (type);

	return ret;
}

static gchar*
parseAuthor (xmlNodePtr cur) {
	gchar *tmp = NULL;
	gchar *tmp2;
	gchar *tmp3;

	g_assert (NULL != cur);
	cur = cur->xmlChildrenNode;

	while (cur != NULL) {
		if (NULL == cur->name) {
			g_warning ("invalid XML: parser returns NULL value -> tag ignored!");
			cur = cur->next;
			continue;
		}

		if (!xmlStrcmp (cur->name, BAD_CAST"name"))
			tmp = (gchar*) xmlNodeListGetString (cur->doc, cur->xmlChildrenNode, 1);

		if (!xmlStrcmp (cur->name, BAD_CAST"email")) {
			tmp2 = (gchar*) xmlNodeListGetString (cur->doc, cur->xmlChildrenNode, 1);
			tmp3 = g_strdup_printf("%s <a href=\"mailto:%s\">%s</a>", tmp, tmp2, tmp2);
			g_free (tmp);
			g_free (tmp2);
			tmp = tmp3;
		}

		if (!xmlStrcmp (cur->name, BAD_CAST"url")) {
			tmp2 = (gchar*) xmlNodeListGetString (cur->doc, cur->xmlChildrenNode, 1);
			tmp3 = g_strdup_printf("%s (<a href=\"%s\">Website</a>)", tmp, tmp2);
			g_free (tmp);
			g_free (tmp2);
			tmp = tmp3;
		}

		cur = cur->next;
	}

	return tmp;
}

GrssFeedItem*
parse_entry (FeedPieHandler *parser, GrssFeedChannel *feed, xmlDocPtr doc, xmlNodePtr cur) {
	xmlChar *xtmp;
	gchar *tmp2;
	gchar *tmp;
	GrssFeedItem *item;

	g_assert (NULL != cur);
	item = grss_feed_item_new (feed);

	cur = cur->xmlChildrenNode;

	while (cur) {
		if (!cur->name) {
			g_warning ("invalid XML: parser returns NULL value -> tag ignored!");
			cur = cur->next;
			continue;
		}

		if (cur->ns) {
			if (ns_handler_item (parser->priv->handler, item, cur)) {
				cur = cur->next;
				continue;
			}
		}

		if (!xmlStrcmp (cur->name, BAD_CAST"title")) {
			if (NULL != (tmp = unhtmlize (pie_parse_content_construct (cur)))) {
				grss_feed_item_set_title (item, tmp);
				g_free (tmp);
			}
		}
		else if (!xmlStrcmp (cur->name, BAD_CAST"link")) {
			if (NULL != (tmp2 = (gchar*) xmlGetProp (cur, BAD_CAST"href"))) {
				/* 0.3 link : rel, type and href attribute */
				xtmp = xmlGetProp (cur, BAD_CAST"rel");

				if (xtmp != NULL && !xmlStrcmp (xtmp, BAD_CAST"alternate"))
					grss_feed_item_set_source(item, tmp2);
				/* else
					FIXME: Maybe do something with other links? */

				xmlFree (xtmp);
				g_free (tmp2);
			}
			else {
				/* 0.2 link : element content is the link, or non-alternate link in 0.3 */
				if (NULL != (tmp = (gchar*) xmlNodeListGetString (doc, cur->xmlChildrenNode, 1))) {
					grss_feed_item_set_source (item, tmp);
					g_free (tmp);
				}
			}
		}
		else if (!xmlStrcmp (cur->name, BAD_CAST"author")) {
			/* parse feed author */
			tmp =  parseAuthor (cur);
			grss_feed_item_set_author (item, tmp);
			g_free (tmp);
		}
		else if (!xmlStrcmp (cur->name, BAD_CAST"contributor")) {
			/* parse feed contributors */
			tmp = parseAuthor (cur);
			grss_feed_item_add_contributor (item, tmp);
			g_free (tmp);
		}
		else if (!xmlStrcmp (cur->name, BAD_CAST"id")) {
			if (NULL != (tmp = (gchar*) xmlNodeListGetString (doc, cur->xmlChildrenNode, 1))) {
				grss_feed_item_set_id (item, tmp);
				g_free (tmp);
			}
		}
		else if (!xmlStrcmp (cur->name, BAD_CAST"issued")) {
			/* FIXME: is <modified> or <issued> or <created> the time tag we want to display? */
 			if (NULL != (tmp = (gchar*) xmlNodeListGetString (doc, cur->xmlChildrenNode, 1))) {
				grss_feed_item_set_publish_time (item, date_parse_ISO8601 (tmp));
				g_free (tmp);
			}
		}
		else if (!xmlStrcmp(cur->name, BAD_CAST"content")) {
			/* <content> support */
			if (NULL != (tmp = pie_parse_content_construct (cur))) {
				grss_feed_item_set_description (item, tmp);
				g_free (tmp);
			}
		}
		else if (!xmlStrcmp (cur->name, BAD_CAST"summary")) {
			/* <summary> can be used for short text descriptions, if there is no
			   <content> description we show the <summary> content */
			if (!grss_feed_item_get_description (item)) {
				if (NULL != (tmp = pie_parse_content_construct (cur))) {
					grss_feed_item_set_description (item, tmp);
					g_free (tmp);
				}
			}
		}
		else if (!xmlStrcmp (cur->name, BAD_CAST"copyright")) {
 			if (NULL != (tmp = (gchar*) xmlNodeListGetString (doc, cur->xmlChildrenNode, 1))) {
				grss_feed_item_set_copyright (item, tmp);
				g_free (tmp);
			}
		}

		cur = cur->next;
	}

	return item;
}

static GList*
feed_pie_handler_parse (FeedHandler *self, GrssFeedChannel *feed, xmlDocPtr doc, gboolean do_items, GError **error)
{
	gchar *tmp2;
	gchar *tmp = NULL;
	gchar *tmp3;
	time_t t;
	time_t now;
	xmlNodePtr cur;
	GList *items;
	GrssFeedItem *item;
	FeedPieHandler *parser;

	items = NULL;
	now = time (NULL);
	parser = FEED_PIE_HANDLER (self);

	cur = xmlDocGetRootElement (doc);
	while (cur && xmlIsBlankNode (cur))
		cur = cur->next;

	while (TRUE) {
		if (xmlStrcmp (cur->name, BAD_CAST"feed")) {
			g_set_error (error, FEED_PIE_HANDLER_ERROR, FEED_PIE_HANDLER_PARSE_ERROR, "Could not find Atom/PIE header!");
			break;
		}

		/* parse feed contents */
		cur = cur->xmlChildrenNode;
		while (cur) {
			if(!cur->name || cur->type != XML_ELEMENT_NODE) {
				cur = cur->next;
				continue;
			}

			if (cur->ns) {
				if (ns_handler_channel (parser->priv->handler, feed, cur)) {
					cur = cur->next;
					continue;
				}
			}

			if (!xmlStrcmp(cur->name, BAD_CAST"title")) {
				tmp = unhtmlize (pie_parse_content_construct (cur));
				if (tmp)
					grss_feed_channel_set_title (feed, tmp);
			}
			else if (!xmlStrcmp (cur->name, BAD_CAST"link")) {
				tmp = (gchar*) xmlGetNsProp (cur, BAD_CAST"href", NULL);

				if (tmp) {
					/* 0.3 link : rel, type and href attribute */
					tmp2 = (gchar*) xmlGetNsProp (cur, BAD_CAST"rel", NULL);

					if (tmp2 && g_str_equal (tmp2, "alternate"))
						grss_feed_channel_set_homepage (feed, tmp);
					/* else
						FIXME: Maybe do something with other links? */

					g_free (tmp2);
					g_free (tmp);
				}
				else {
					/* 0.2 link : element content is the link, or non-alternate link in 0.3 */
					tmp = (gchar*) xmlNodeListGetString (doc, cur->xmlChildrenNode, 1);
					if (tmp) {
						grss_feed_channel_set_homepage (feed, tmp);
						g_free (tmp);
					}
				}
			}
			/* parse feed author */
			else if (!xmlStrcmp (cur->name, BAD_CAST"author")) {
				/* parse feed author */
				tmp = parseAuthor (cur);
				if (tmp) {
					grss_feed_channel_set_editor (feed, tmp);
					g_free (tmp);
				}
			}
			else if (!xmlStrcmp (cur->name, BAD_CAST"tagline")) {
				tmp = pie_parse_content_construct (cur);
				if (tmp) {
					grss_feed_channel_set_description (feed, tmp);
					g_free (tmp);
				}
			}
			else if (!xmlStrcmp (cur->name, BAD_CAST"generator")) {
				tmp = unhtmlize ((gchar*) xmlNodeListGetString(doc, cur->xmlChildrenNode, 1));
				if (tmp && tmp[0] != '\0') {
					tmp2 = (gchar*) xmlGetNsProp (cur, BAD_CAST"version", NULL);
					if (tmp2) {
						tmp3 = g_strdup_printf ("%s %s", tmp, tmp2);
						g_free (tmp);
						g_free (tmp2);
						tmp = tmp3;
					}

					tmp2 = (gchar*) xmlGetNsProp (cur, BAD_CAST"url", NULL);
					if (tmp2) {
						tmp3 = g_strdup_printf ("<a href=\"%s\">%s</a>", tmp2, tmp);
						g_free (tmp2);
						g_free (tmp);
						tmp = tmp3;
					}

					grss_feed_channel_set_generator (feed, tmp);
				}

				g_free (tmp);
			}
			else if (!xmlStrcmp (cur->name, BAD_CAST"copyright")) {
				tmp = pie_parse_content_construct (cur);
				if (tmp) {
					grss_feed_channel_set_copyright (feed, tmp);
					g_free (tmp);
				}
			}
			else if (!xmlStrcmp (cur->name, BAD_CAST"modified")) { /* Modified was last used in IETF draft 02) */
				tmp = (gchar*) xmlNodeListGetString (doc, cur->xmlChildrenNode, 1);
				if (tmp) {
					t = date_parse_ISO8601 (tmp);
					grss_feed_channel_set_update_time (feed, t);
					g_free (tmp);
				}
			}
			else if (!xmlStrcmp (cur->name, BAD_CAST"updated")) { /* Updated was added in IETF draft 03 */
				tmp = (gchar*) xmlNodeListGetString (doc, cur->xmlChildrenNode, 1);
				if (tmp) {
					t = date_parse_ISO8601 (tmp);
					grss_feed_channel_set_update_time (feed, t);
					g_free(tmp);
				}
			}
			else if (!xmlStrcmp (cur->name, BAD_CAST"contributor")) {
				tmp = parseAuthor (cur);
				if (tmp) {
					grss_feed_channel_add_contributor (feed, tmp);
					g_free (tmp);
				}
			}
			else if (do_items == TRUE && (!xmlStrcmp (cur->name, BAD_CAST"entry"))) {
				item = parse_entry (parser, feed, doc, cur);
				if (item) {
					if (grss_feed_item_get_publish_time (item) == 0)
						grss_feed_item_set_publish_time (item, now);
					items = g_list_prepend (items, item);
				}
			}

			cur = cur->next;
		}

		break;
	}

	/*
		I've not found a more appropriate mimetype for PIE...
	*/
	grss_feed_channel_set_format (feed, "application/atom+xml");

	if (items != NULL)
		items = g_list_reverse (items);
	return items;
}

static void
feed_handler_interface_init (FeedHandlerInterface *iface)
{
	iface->set_ns_handler = feed_pie_handler_set_ns_handler;
	iface->check_format = feed_pie_handler_check_format;
	iface->parse = feed_pie_handler_parse;
}

static void
feed_pie_handler_class_init (FeedPieHandlerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (FeedPieHandlerPrivate));
	object_class->finalize = feed_pie_handler_finalize;
}

static void
feed_pie_handler_init (FeedPieHandler *object)
{
	object->priv = FEED_PIE_HANDLER_GET_PRIVATE (object);
}

FeedPieHandler*
feed_pie_handler_new ()
{
	FeedPieHandler *parser;

	parser = g_object_new (FEED_PIE_HANDLER_TYPE, NULL);
	return parser;
}
