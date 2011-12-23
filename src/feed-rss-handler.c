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

/*
 * Original code is from Liferea:
 *
 * rss_channel.c  some tolerant and generic RSS/RDF channel parsing
 * Copyright (C) 2003-2009 Lars Lindner <lars.lindner@gmail.com>
 * Copyright (C) 2005-2006 Nathan Conrad <t98502@users.sourceforge.net>
 *
 * rss_item.c  RSS/RDF item parsing
 * Copyright (C) 2003-2009 Lars Lindner <lars.lindner@gmail.com>
 *
 * date.c  date formatting routines
 * Copyright (C) 2008-2009  Lars Lindner <lars.lindner@gmail.com>
 * Copyright (C) 2004-2006  Nathan J. Conrad <t98502@users.sourceforge.net>
 * Copyright (C) 2001 Ximian, Inc. - Chris Lahey <clahey@ximian.com>
 */

#include "feed-handler.h"
#include "feed-rss-handler.h"
#include "utils.h"
#include "feed-channel.h"
#include "feed-item.h"
#include "feed-enclosure.h"
#include "ns-handler.h"

#define FEED_RSS_HANDLER_GET_PRIVATE(o)	(G_TYPE_INSTANCE_GET_PRIVATE ((o), FEED_RSS_HANDLER_TYPE, FeedRssHandlerPrivate))

#define FEED_RSS_HANDLER_ERROR		feed_rss_handler_error_quark()

/* HTML output strings */
#define TEXT_INPUT_FORM_START		"<form class=\"rssform\" method=\"GET\" action=\""
#define TEXT_INPUT_TEXT_FIELD		"\"><input class=\"rssformtext\" type=\"text\" value=\"\" name=\""
#define TEXT_INPUT_SUBMIT		"\" /><input class=\"rssformsubmit\" type=\"submit\" value=\""
#define TEXT_INPUT_FORM_END		"\" /></form>"

struct FeedRssHandlerPrivate {
	NSHandler	*handler;
};

enum {
	FEED_RSS_HANDLER_PARSE_ERROR,
};

static void feed_handler_interface_init (FeedHandlerInterface *iface);
G_DEFINE_TYPE_WITH_CODE (FeedRssHandler, feed_rss_handler, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (FEED_HANDLER_TYPE,
                                                feed_handler_interface_init));

static GQuark
feed_rss_handler_error_quark ()
{
	return g_quark_from_static_string ("feed_rss_handler_error");
}

static void
feed_rss_handler_finalize (GObject *object)
{
	FeedRssHandler *parser;

	parser = FEED_RSS_HANDLER (object);
	G_OBJECT_CLASS (feed_rss_handler_parent_class)->finalize (object);
}

static void
feed_rss_handler_set_ns_handler (FeedHandler *self, NSHandler *handler)
{
	FeedRssHandler *rss;

	rss = FEED_RSS_HANDLER (self);
	rss->priv->handler = handler;
}

static gboolean
feed_rss_handler_check_format (FeedHandler *self, xmlDocPtr doc, xmlNodePtr cur)
{
	if (!xmlStrcmp (cur->name, BAD_CAST"rss") ||
	    !xmlStrcmp (cur->name, BAD_CAST"rdf") ||
	    !xmlStrcmp (cur->name, BAD_CAST"RDF")) {
		return TRUE;
	}

	/* RSS 1.1 */
	if ((NULL != cur->ns) &&
	    (NULL != cur->ns->href) &&
	    !xmlStrcmp (cur->name, BAD_CAST"Channel") &&
	    !xmlStrcmp (cur->ns->href, BAD_CAST"http://purl.org/net/rss1.1#")) {
	   	return TRUE;
	}

	return FALSE;
}

static void
parse_rss_cloud (GrssFeedChannel *feed, xmlNodePtr cur) {
	gchar *domain;
	gchar *path;
	gchar *protocol;
	gchar *completepath;

	domain = (gchar*) xmlGetNsProp (cur, BAD_CAST"domain", NULL);
	path = (gchar*) xmlGetNsProp (cur, BAD_CAST"path", NULL);
	protocol = (gchar*) xmlGetNsProp (cur, BAD_CAST"protocol", NULL);

	if (domain != NULL && path != NULL && protocol != NULL) {
		if (strncmp (domain, "http://", 7) != 0)
			completepath = g_strdup_printf ("http://%s%s", domain, path);
		else
			completepath = g_strdup_printf ("%s%s", domain, path);

		grss_feed_channel_set_rsscloud (feed, completepath, protocol);

		g_free (domain);
		g_free (path);
		g_free (protocol);
		g_free (completepath);
	}
}

static void
parse_channel (FeedRssHandler *parser, GrssFeedChannel *feed, xmlDocPtr doc, xmlNodePtr cur) {
	gchar *tmp;
	time_t t;

	g_assert (NULL != cur);
	cur = cur->xmlChildrenNode;

	while (cur) {
		if (cur->type != XML_ELEMENT_NODE || cur->name == NULL) {
			cur = cur->next;
			continue;
		}

		/* check namespace of this tag */
		if (cur->ns) {
			if (ns_handler_channel (parser->priv->handler, feed, cur)) {
				cur = cur->next;
				continue;
			}
		}

		if (!xmlStrcmp (cur->name, BAD_CAST"copyright")) {
 			tmp = (gchar*) xmlNodeListGetString (doc, cur->xmlChildrenNode, 1);
			if (tmp) {
				grss_feed_channel_set_copyright (feed, tmp);
				g_free (tmp);
			}
		}
		else if (!xmlStrcmp (cur->name, BAD_CAST"webMaster") || !xmlStrcmp (cur->name, BAD_CAST"publisher")) {
 			tmp = (gchar*) xmlNodeListGetString (doc, cur->xmlChildrenNode, 1);
			if (tmp) {
				grss_feed_channel_set_webmaster (feed, tmp);
				g_free (tmp);
			}
		}
		else if (!xmlStrcmp (cur->name, BAD_CAST"language")) {
 			tmp = (gchar*) xmlNodeListGetString (doc, cur->xmlChildrenNode, 1);
			if (tmp) {
				grss_feed_channel_set_language (feed, tmp);
				g_free (tmp);
			}
		}
		else if (!xmlStrcmp (cur->name, BAD_CAST"managingEditor")) {
 			tmp = (gchar*) xmlNodeListGetString (doc, cur->xmlChildrenNode, 1);
			if (tmp) {
				grss_feed_channel_set_editor (feed, tmp);
				g_free (tmp);
			}
		}
		else if (!xmlStrcmp (cur->name, BAD_CAST"lastBuildDate")) {
 			tmp = (gchar*) xmlNodeListGetString (doc, cur->xmlChildrenNode, 1);
			if (tmp) {
				t = date_parse_RFC822 (tmp);
				grss_feed_channel_set_update_time (feed, t);
				g_free (tmp);
			}
		}
		else if (!xmlStrcmp (cur->name, BAD_CAST"generator")) {
 			tmp = (gchar*) xmlNodeListGetString (doc, cur->xmlChildrenNode, 1);
			if (tmp) {
				grss_feed_channel_set_generator (feed, tmp);
				g_free (tmp);
			}
		}
		else if (!xmlStrcmp (cur->name, BAD_CAST"pubDate")) {
 			if (NULL != (tmp = (gchar*) xmlNodeListGetString (doc, cur->xmlChildrenNode, 1))) {
				t = date_parse_RFC822 (tmp);
				grss_feed_channel_set_publish_time (feed, t);
				g_free (tmp);
			}
		}
		else if (!xmlStrcmp (cur->name, BAD_CAST"ttl")) {
 			if (NULL != (tmp = (gchar*) xmlNodeListGetString (doc, cur->xmlChildrenNode, TRUE))) {
				grss_feed_channel_set_update_interval (feed, atoi (tmp));
				g_free (tmp);
			}
		}
		else if (!xmlStrcmp (cur->name, BAD_CAST"title")) {
 			if (NULL != (tmp = unhtmlize ((gchar*) xmlNodeListGetString (doc, cur->xmlChildrenNode, TRUE)))) {
				grss_feed_channel_set_title (feed, tmp);
				g_free (tmp);
			}
		}
		/*
			<alink> has been found at least in Xinhua News Agency RSS feeds
		*/
		else if (!xmlStrcmp (cur->name, BAD_CAST"link") || !xmlStrcmp (cur->name, BAD_CAST"alink")) {
 			if (NULL != (tmp = unhtmlize ((gchar*) xmlNodeListGetString (doc, cur->xmlChildrenNode, TRUE)))) {
				grss_feed_channel_set_homepage (feed, tmp);
				g_free (tmp);
			}
		}
		else if (!xmlStrcmp (cur->name, BAD_CAST"description")) {
 			tmp = xhtml_extract (cur, 0, NULL);
			if (tmp) {
				grss_feed_channel_set_description (feed, tmp);
				g_free (tmp);
			}
		}
		else if (!xmlStrcmp (cur->name, BAD_CAST"cloud")) {
 			parse_rss_cloud (feed, cur);
		}

		cur = cur->next;
	}
}

static GrssFeedItem*
parse_rss_item (FeedRssHandler *parser, GrssFeedChannel *feed, xmlDocPtr doc, xmlNodePtr cur)
{
	gchar *tmp;
	gchar *tmp2;
	gchar *tmp3;
	time_t t;
	GrssFeedItem *item;

	g_assert (cur != NULL);

	item = grss_feed_item_new (feed);

	/* try to get an item about id */
	tmp = (gchar*) xmlGetProp (cur, BAD_CAST"about");
	if (tmp) {
		grss_feed_item_set_id (item, tmp);
		grss_feed_item_set_source (item, tmp);
		g_free (tmp);
	}

	cur = cur->xmlChildrenNode;

	while (cur) {
		if (cur->type != XML_ELEMENT_NODE || !cur->name) {
			cur = cur->next;
			continue;
		}

		/* check namespace of this tag */
		if (cur->ns) {
			if (ns_handler_item (parser->priv->handler, item, cur)) {
				cur = cur->next;
				continue;
			}
		}

		if (!xmlStrcmp (cur->name, BAD_CAST"category")) {
 			tmp = (gchar*) xmlNodeListGetString (doc, cur->xmlChildrenNode, 1);
			if (tmp) {
				grss_feed_item_add_category (item, tmp);
				g_free (tmp);
			}
		}
		else if (!xmlStrcmp (cur->name, BAD_CAST"author")) {
 			tmp = (gchar*) xmlNodeListGetString (doc, cur->xmlChildrenNode, 1);
			if (tmp) {
				grss_feed_item_set_author (item, tmp);
				g_free (tmp);
			}
		}
		else if (!xmlStrcmp (cur->name, BAD_CAST"comments")) {
 			tmp = (gchar*) xmlNodeListGetString (doc, cur->xmlChildrenNode, 1);
			if (tmp) {
				grss_feed_item_set_comments_url (item, tmp);
				g_free (tmp);
			}
		}
		else if (!xmlStrcmp (cur->name, BAD_CAST"pubDate")) {
 			tmp = (gchar*) xmlNodeListGetString (doc, cur->xmlChildrenNode, 1);
			if (tmp) {
				t = date_parse_RFC822 (tmp);
				grss_feed_item_set_publish_time (item, t);
				g_free (tmp);
			}
		}
		else if (!xmlStrcmp (cur->name, BAD_CAST"enclosure")) {
			/* RSS 0.93 allows multiple enclosures */
			tmp = (gchar*) xmlGetProp (cur, BAD_CAST"url");

			if (tmp) {
				gchar *type = (gchar*) xmlGetProp (cur, BAD_CAST"type");
				gssize length = 0;
				GrssFeedEnclosure *enclosure;

				tmp2 = (gchar*) xmlGetProp (cur, BAD_CAST"length");
				if (tmp2) {
					length = atol (tmp2);
					xmlFree (tmp2);
				}

				tmp3 = (gchar*) grss_feed_channel_get_homepage (feed);

				if ((strstr (tmp, "://") == NULL) &&
				    (tmp3 != NULL) &&
				    (strstr (tmp3, "://") != NULL)) {
					/* add base URL if necessary and possible */
					tmp2 = g_strdup_printf ("%s/%s", tmp3, tmp);
					xmlFree (tmp);
					tmp = tmp2;
				}

				enclosure = grss_feed_enclosure_new (tmp);
				grss_feed_enclosure_set_format (enclosure, type);
				grss_feed_enclosure_set_length (enclosure, length);
				grss_feed_item_add_enclosure (item, enclosure);

				xmlFree (tmp);
				xmlFree (type);
			}
		}
		else if (!xmlStrcmp (cur->name, BAD_CAST"guid")) {
			if (!grss_feed_item_get_id (item)) {
				tmp = (gchar*) xmlNodeListGetString (doc, cur->xmlChildrenNode, 1);
				if (tmp) {
					if (strlen (tmp) > 0) {
						grss_feed_item_set_id (item, tmp);
						tmp2 = (gchar*) xmlGetProp (cur, BAD_CAST"isPermaLink");

						if (!grss_feed_item_get_source (item) && (tmp2 == NULL || g_str_equal (tmp2, "true")))
							grss_feed_item_set_source (item, tmp); /* Per the RSS 2.0 spec. */
						if (tmp2)
							xmlFree (tmp2);
					}

					xmlFree (tmp);
				}
			}
		}
		else if (!xmlStrcmp (cur->name, BAD_CAST"title")) {
 			tmp = unhtmlize ((gchar*) xmlNodeListGetString (doc, cur->xmlChildrenNode, TRUE));
			if (tmp) {
				grss_feed_item_set_title (item, tmp);
				g_free (tmp);
			}
		}
		else if (!xmlStrcmp (cur->name, BAD_CAST"link")) {
 			tmp = unhtmlize ((gchar*) xmlNodeListGetString (doc, cur->xmlChildrenNode, TRUE));
			if (tmp) {
				grss_feed_item_set_source (item, tmp);
				g_free (tmp);
			}
		}
		else if (!xmlStrcmp (cur->name, BAD_CAST"description")) {
 			tmp = xhtml_extract (cur, 0, NULL);
			if (tmp) {
				/* don't overwrite content:encoded descriptions... */
				if (!grss_feed_item_get_description (item))
					grss_feed_item_set_description (item, tmp);
				g_free (tmp);
			}
		}
		else if (!xmlStrcmp (cur->name, BAD_CAST"source")) {
			tmp = (gchar*) xmlGetProp (cur, BAD_CAST"url");
			tmp2 = unhtmlize ((gchar*) xmlNodeListGetString (doc, cur->xmlChildrenNode, 1));

			if (tmp) {
				grss_feed_item_set_real_source (item, g_strchomp (tmp), tmp2 ? g_strchomp (tmp2) : NULL);
				g_free (tmp);
			}

			if (tmp2)
				g_free (tmp2);
		}

		cur = cur->next;
	}

	return item;
}

static gchar*
parse_image (xmlNodePtr cur) {
	gchar *tmp;

	g_assert (NULL != cur);

	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		if (cur->type == XML_ELEMENT_NODE) {
			if (!xmlStrcmp (cur->name, BAD_CAST"url")) {
				tmp = (gchar*) xmlNodeListGetString (cur->doc, cur->xmlChildrenNode, 1);
				if (NULL != tmp)
					return tmp;
			}
		}

		cur = cur->next;
	}

	return NULL;
}

static GList*
feed_rss_handler_parse (FeedHandler *self, GrssFeedChannel *feed, xmlDocPtr doc, GError **error)
{
	gchar *tmp;
	gboolean rdf;
	time_t now;
	GList *items;
	xmlNodePtr cur;
	GrssFeedItem *item;
	FeedRssHandler *parser;

	items = NULL;
	rdf = FALSE;
	now = time (NULL);
	parser = FEED_RSS_HANDLER (self);

	cur = xmlDocGetRootElement (doc);
	while (cur && xmlIsBlankNode (cur))
		cur = cur->next;

	if (!xmlStrcmp (cur->name, BAD_CAST"rss")) {
		cur = cur->xmlChildrenNode;
		rdf = FALSE;
	}
	else if (!xmlStrcmp (cur->name, BAD_CAST"rdf") ||
	         !xmlStrcmp (cur->name, BAD_CAST"RDF")) {
		cur = cur->xmlChildrenNode;
		rdf = TRUE;
	}
	else if (!xmlStrcmp (cur->name, BAD_CAST"Channel")) {
		rdf = FALSE;
	}
	else {
		g_set_error (error, FEED_RSS_HANDLER_ERROR, FEED_RSS_HANDLER_PARSE_ERROR, "Could not find RDF/RSS header!");
		return NULL;
	}

	while (cur && xmlIsBlankNode (cur))
		cur = cur->next;

	while (cur) {
		if (!cur->name) {
			g_warning ("invalid XML: parser returns NULL value -> tag ignored!");
			cur = cur->next;
			continue;
		}

		if ((!xmlStrcmp (cur->name, BAD_CAST"channel")) ||
		    (!xmlStrcmp (cur->name, BAD_CAST"Channel"))) {
			parse_channel (parser, feed, doc, cur);
			if (rdf == FALSE)
				cur = cur->xmlChildrenNode;
			break;
		}

		cur = cur->next;
	}

	/* For RDF (rss 0.9 or 1.0), cur now points to the item after the channel tag. */
	/* For RSS, cur now points to the first item inside of the channel tag */
	/* This ends up being the thing with the items, (and images/textinputs for RDF) */

	/* parse channel contents */
	while (cur) {
		if (cur->type != XML_ELEMENT_NODE || NULL == cur->name) {
			cur = cur->next;
			continue;
		}

		/* save link to channel image */
		if ((!xmlStrcmp (cur->name, BAD_CAST"image"))) {
			if (NULL != (tmp = parse_image (cur))) {
				grss_feed_channel_set_image (feed, tmp);
				g_free (tmp);
			}
		}
		else if ((!xmlStrcmp (cur->name, BAD_CAST"items"))) { /* RSS 1.1 */
			xmlNodePtr iter = cur->xmlChildrenNode;

			while (iter) {
				item = parse_rss_item (parser, feed, doc, iter);

				if (item != NULL) {
					if (grss_feed_item_get_publish_time (item) == 0)
						grss_feed_item_set_publish_time (item, now);
					items = g_list_append (items, item);
				}

				iter = iter->next;
			}
		}
		else if ((!xmlStrcmp (cur->name, BAD_CAST"item"))) { /* RSS 1.0, 2.0 */
			item = parse_rss_item (parser, feed, doc, cur);

			if (item != NULL) {
				if (grss_feed_item_get_publish_time (item) == 0)
					grss_feed_item_set_publish_time (item, now);
				items = g_list_append (items, item);
			}

		}

		cur = cur->next;
	}

	if (items != NULL)
		items = g_list_reverse (items);
	return items;
}

static void
feed_handler_interface_init (FeedHandlerInterface *iface)
{
	iface->set_ns_handler = feed_rss_handler_set_ns_handler;
	iface->check_format = feed_rss_handler_check_format;
	iface->parse = feed_rss_handler_parse;
}

static void
feed_rss_handler_class_init (FeedRssHandlerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (FeedRssHandlerPrivate));
	object_class->finalize = feed_rss_handler_finalize;
}

static void
feed_rss_handler_init (FeedRssHandler *object)
{
	object->priv = FEED_RSS_HANDLER_GET_PRIVATE (object);
}

FeedRssHandler*
feed_rss_handler_new ()
{
	FeedRssHandler *parser;

	parser = g_object_new (FEED_RSS_HANDLER_TYPE, NULL);
	return parser;
}
