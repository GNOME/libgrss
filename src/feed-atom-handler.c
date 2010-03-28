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
 * atom10.c  Atom 1.0 Parser
 * Copyright (C) 2005-2006 Nathan Conrad <t98502@users.sourceforge.net>
 * Copyright (C) 2003-2009 Lars Lindner <lars.lindner@gmail.com>
 */

#include "feed-atom-handler.h"
#include "utils.h"
#include "feed-handler.h"
#include "ns-handler.h"

#define FEED_ATOM_HANDLER_GET_PRIVATE(o)	(G_TYPE_INSTANCE_GET_PRIVATE ((o), FEED_ATOM_HANDLER_TYPE, FeedAtomHandlerPrivate))

/**
 * SECTION: feed-atom-handler
 * @short_description: specialized parser for Atom feeds
 *
 * #FeedAtomHandler is a #FeedHandler specialized for feeds in Atom format
 */

#define FEED_ATOM_HANDLER_ERROR			feed_atom_handler_error_quark()
#define ATOM10_NS				BAD_CAST"http://www.w3.org/2005/Atom"

typedef void 	(*AtomChannelParserFunc)	(xmlNodePtr cur, FeedChannel *feed);
typedef void 	(*AtomItemParserFunc)		(xmlNodePtr cur, FeedItem *item, FeedChannel *feed);

struct FeedAtomHandlerPrivate {
	NSHandler	*handler;
	GHashTable	*feed_elements_hash;
	GHashTable	*entry_elements_hash;
};

enum {
	FEED_ATOM_HANDLER_PARSE_ERROR,
};

static void feed_handler_interface_init (FeedHandlerInterface *iface);
G_DEFINE_TYPE_WITH_CODE (FeedAtomHandler, feed_atom_handler, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (FEED_HANDLER_TYPE,
                                                feed_handler_interface_init));

static GQuark
feed_atom_handler_error_quark ()
{
	return g_quark_from_static_string ("feed_atom_handler_error");
}

static void
feed_atom_handler_finalize (GObject *object)
{
	FeedAtomHandler *parser;

	parser = FEED_ATOM_HANDLER (object);
	G_OBJECT_CLASS (feed_atom_handler_parent_class)->finalize (object);
}

static void
feed_atom_handler_set_ns_handler (FeedHandler *self, NSHandler *handler)
{
	FeedAtomHandler *atom;

	atom = FEED_ATOM_HANDLER (self);
	atom->priv->handler = handler;
}

static gboolean
feed_atom_handler_check_format (FeedHandler *self, xmlDocPtr doc, xmlNodePtr cur)
{
	if (cur->name == NULL || cur->ns == NULL || cur->ns->href == NULL)
		return FALSE;
	return xmlStrEqual (cur->name, BAD_CAST"feed") && xmlStrEqual (cur->ns->href, ATOM10_NS);
}

static xmlChar *
common_build_url (const gchar *url, const gchar *baseURL)
{
	xmlChar	*escapedURL;
	xmlChar	*absURL;
	xmlChar	*escapedBaseURL;

	escapedURL = xmlURIEscape (BAD_CAST url);

	if (baseURL) {
		escapedBaseURL = xmlURIEscape (BAD_CAST baseURL);
		absURL = xmlBuildURI (escapedURL, escapedBaseURL);
		xmlFree (escapedURL);
		xmlFree (escapedBaseURL);
	}
	else {
		absURL = escapedURL;
	}

	return absURL;
}

static gchar *
atom10_mark_up_text_content (gchar* content)
{
	gchar **tokens;
	gchar **token;
	gchar *str, *old_str;

	if (!content)
		return NULL;
	if (!*content)
		return g_strdup (content);

	tokens = g_strsplit (content, "\n\n", 0);

	if (!tokens [0]) { /* No tokens */
		str = g_strdup("");
	}
	else if (!tokens [1]) { /* One token */
		str = g_markup_escape_text (tokens [0], -1);
	}
	else { /* Many tokens */
		token = tokens;
		while (*token) {
			old_str = *token;
			str = g_strchug (g_strchomp (*token)); /* WARNING: modifies the token string*/
			if (str[0] != '\0') {
				*token = g_markup_printf_escaped ("<p>%s</p>", str);
				g_free (old_str);
			}
			else {
				**token = '\0'; /* Erase the particular token because it is blank */
			}

			token++;
		}

		str = g_strjoinv ("\n", tokens);
	}

	g_strfreev (tokens);
	return str;
}

/*
 * This parses an Atom content construct.
 *
 * @param cur	the XML node to be parsed
 * @returns g_strduped string which must be freed by the caller.
 */
static gchar*
atom10_parse_content_construct (xmlNodePtr cur)
{
	gchar *ret = NULL;

	if (xmlHasNsProp (cur, BAD_CAST"src", NULL )) {
		gchar *src = (gchar*) xmlGetNsProp (cur, BAD_CAST"src", NULL);

		if (!src) {
			ret = NULL;
		}
		else {
			gchar *baseURL = (gchar*) xmlNodeGetBase (cur->doc, cur);
			xmlChar *url;

			url = common_build_url (src, baseURL);
			ret = g_strdup_printf ("<p><a href=\"%s\">View this item's content.</a></p>", url);

			g_free (url);
			xmlFree (baseURL);
			xmlFree (src);
		}
	}
	else {
		gchar *type;

		/* determine encoding mode */
		type = (gchar*) xmlGetNsProp (cur, BAD_CAST"type", NULL);

		/* Contents need to be de-encoded and should not contain sub-tags.*/
		if (type && (g_str_equal (type,"html") || !g_ascii_strcasecmp (type, "text/html"))) {
			ret = xhtml_extract (cur, 0, NULL);
		}
		else if (!type || !strcmp (type, "text") || !strncasecmp (type, "text/",5)) {
			gchar *tmp;
			/* Assume that "text/ *" files can be directly displayed.. kinda stated in the RFC */
			ret = (gchar*) xmlNodeListGetString (cur->doc, cur->xmlChildrenNode, 1);

			g_strchug (g_strchomp (ret));

			if (!type || !strcasecmp (type, "text"))
				tmp = atom10_mark_up_text_content (ret);
			else
				tmp = g_markup_printf_escaped ("<pre>%s</pre>", ret);
			g_free (ret);
			ret = tmp;
		}
		else if (!strcmp (type,"xhtml") || !g_ascii_strcasecmp (type, "application/xhtml+xml")) {
			/* The spec says to only show the contents of the div tag that MUST be present */
			ret = xhtml_extract (cur, 2, NULL);
		}
		else {
			/* Do nothing on unsupported content types. This allows summaries to be used. */
			ret = NULL;
		}

		g_free (type);
	}

	return ret;
}

/*
 * Parse Atom 1.0 text tags of all sorts.
 *
 * @param htmlified	If set to 1, then HTML is returned.
 * 			When set to 0, All HTML tags are removed
 *
 * @returns an escaped version of a text construct.
 */
static gchar *
atom10_parse_text_construct (xmlNodePtr cur, gboolean htmlified)
{
	gchar	*type, *tmp, *ret = NULL;

	/* determine encoding mode */
	type = (gchar*) xmlGetNsProp (cur, BAD_CAST"type", NULL);

	/* not sure what MIME types are necessary... */

	/* This that need to be de-encoded and should not contain sub-tags.*/
	if (!type || !strcmp(type, "text")) {
		ret = (gchar *)xmlNodeListGetString (cur->doc, cur->xmlChildrenNode, 1);
		if (ret) {
			g_strchug (g_strchomp (ret));

			if (htmlified) {
				tmp = atom10_mark_up_text_content (ret);
				g_free (ret);
				ret = tmp;
			}
		}
	}
	else if (!strcmp(type, "html")) {
		ret = xhtml_extract (cur, 0, NULL);
		if (!htmlified)
			ret = unhtmlize (unxmlize (ret));
	}
	else if (!strcmp (type, "xhtml")) {
		/* The spec says to show the contents of the div tag that MUST be present */
		ret = xhtml_extract (cur, 2, NULL);

		if (!htmlified)
			ret = unhtmlize (ret);
	}
	else {
		/* Invalid Atom feed */
		ret = g_strdup ("This attribute was invalidly specified in this Atom feed.");
	}

	g_free (type);
	return ret;
}

static gchar *
atom10_parse_person_construct (xmlNodePtr cur)
{
	gchar *tmp = NULL;
	gchar *name = NULL;
	gchar *uri = NULL;
	gchar *email = NULL;
	gboolean invalid = FALSE;

	cur = cur->xmlChildrenNode;
	while (cur) {
		if (NULL == cur->name || cur->type != XML_ELEMENT_NODE || cur->ns == NULL || cur->ns->href == NULL) {
			cur = cur->next;
			continue;
		}

		if (xmlStrEqual (cur->ns->href, ATOM10_NS)) {
			if (xmlStrEqual (cur->name, BAD_CAST"name")) {
				g_free (name);
				name = (gchar*) xmlNodeListGetString (cur->doc, cur->xmlChildrenNode, 1);
			}

			if (xmlStrEqual (cur->name, BAD_CAST"email")) {
				if (email)
					invalid = TRUE;
				g_free (email);
				tmp = (gchar *)xmlNodeListGetString(cur->doc, cur->xmlChildrenNode, 1);
				email = g_strdup_printf(" - <a href=\"mailto:%s\">%s</a>", tmp, tmp);
				g_free(tmp);
			}

			if (xmlStrEqual (cur->name, BAD_CAST"uri")) {
				if (!uri)
					invalid = TRUE;
				g_free (uri);
				tmp = (gchar *)xmlNodeListGetString (cur->doc, cur->xmlChildrenNode, 1);
				uri = g_strdup_printf (" (<a href=\"%s\">Website</a>)", tmp);
				g_free (tmp);
			}
		}
		else {
			/* FIXME: handle extension elements here */
		}

		cur = cur->next;
	}
	if (!name) {
		invalid = TRUE;
		name = g_strdup ("Invalid Atom feed: unknown author");
	}

	/* FIXME: so somthing with "invalid" flag */
	tmp = g_strdup_printf ("%s%s%s", name, uri ? uri : "", email ? email : "");

	if (uri)
		g_free (uri);

	if (email)
		g_free (email);

	g_free (name);
	return tmp;
}

/* Note: this function is called for both item and feed context */
static gchar *
atom10_parse_link (xmlNodePtr cur, FeedChannel *feed, FeedItem *item)
{
	gchar *href;
	const gchar *home;
	gchar *alternate = NULL;

	href = (gchar*) xmlGetNsProp (cur, BAD_CAST"href", NULL);

	if (href) {
		xmlChar *baseURL = xmlNodeGetBase (cur->doc, cur);
		gchar *url, *relation, *type, *escTitle = NULL, *title;

		home = feed_channel_get_homepage (feed);

		if (!baseURL && home && strstr (home, "://"))
			baseURL = xmlStrdup (BAD_CAST home);
		url = (gchar*) common_build_url (href, (gchar*) baseURL);

		type = (gchar*) xmlGetNsProp (cur, BAD_CAST"type", NULL);
		relation = (gchar*) xmlGetNsProp (cur, BAD_CAST"rel", NULL);

		title = (gchar*) xmlGetNsProp (cur, BAD_CAST"title", NULL);
		if (title)
			escTitle = g_markup_escape_text (title, -1);

		if (!relation || g_str_equal (relation, BAD_CAST"alternate")) {
			alternate = g_strdup (url);
		}
		else if (g_str_equal (relation, "replies")) {
			if (item != NULL && (!type || g_str_equal (type, BAD_CAST"application/atom+xml"))) {
				gchar *commentUri = (gchar*) common_build_url ((gchar*) url, home);
				feed_item_set_comments_url (item, commentUri);
				g_free (commentUri);
			}
		}
		else if (g_str_equal (relation, "enclosure")) {
			if (item != NULL) {
				gsize length = 0;
				gchar *lengthStr = (gchar*) xmlGetNsProp (cur, BAD_CAST"length", NULL);
				FeedEnclosure *enclosure;

				if (lengthStr)
					length = atol (lengthStr);
				g_free (lengthStr);

				enclosure = feed_enclosure_new (url);
				feed_enclosure_set_format (enclosure, type);
				feed_enclosure_set_length (enclosure, length);
				feed_item_add_enclosure (item, enclosure);
			}
		}
		else if (g_str_equal (relation, "related")) {
			if (item != NULL)
				feed_item_set_related (item, url);
		}
		else if (g_str_equal (relation, "via")) {
			/*
				FIXME	How to handle "via" relation? With feed_item_set_source() ?
			*/
		}
		else if (g_str_equal (relation, "hub")) {
			if (feed != NULL)
				feed_channel_set_pubsubhub (feed, url, NULL);
		}
		else if (g_str_equal (relation, "self")) {
			if (feed != NULL)
				feed_channel_set_pubsubhub (feed, NULL, url);
		}

		xmlFree (title);
		xmlFree (baseURL);
		g_free (escTitle);
		g_free (url);
		g_free (relation);
		g_free (type);
		g_free (href);
	}
	else {
		/* FIXME: @href is required, this document is not valid Atom */;
	}

	return alternate;
}

static void
atom10_parse_entry_author (xmlNodePtr cur, FeedItem *item, FeedChannel *feed)
{
	gchar *author;

	author = atom10_parse_person_construct (cur);
	if (author) {
		feed_item_set_author (item, author);
		g_free (author);
	}
}

static void
atom10_parse_entry_category (xmlNodePtr cur, FeedItem *item, FeedChannel *feed)
{
	gchar *category = NULL;

	category = (gchar*) xmlGetNsProp (cur, BAD_CAST"label", NULL);
	if (!category)
		category = (gchar*) xmlGetNsProp (cur, BAD_CAST"term", NULL);

	if (category) {
		gchar *escaped = g_markup_escape_text (category, -1);
		feed_item_add_category (item, escaped);
		g_free (escaped);
		xmlFree (category);
	}
}

static void
atom10_parse_entry_content (xmlNodePtr cur, FeedItem *item, FeedChannel *feed)
{
	gchar *content;

	content = atom10_parse_content_construct (cur);
	if (content) {
		feed_item_set_description (item, content);
		g_free (content);
	}
}

static void
atom10_parse_entry_contributor (xmlNodePtr cur, FeedItem *item, FeedChannel *feed)
{
	gchar *contributor;

	contributor = atom10_parse_person_construct (cur);
	if (contributor) {
		feed_item_add_contributor (item, contributor);
		g_free (contributor);
	}
}

static void
atom10_parse_entry_id (xmlNodePtr cur, FeedItem *item, FeedChannel *feed)
{
	gchar *id;

	id = (gchar*) xmlNodeListGetString (cur->doc, cur->xmlChildrenNode, 1);
	if (id) {
		if (strlen (id) > 0)
			feed_item_set_id (item, id);
		g_free (id);
	}
}

static void
atom10_parse_entry_link (xmlNodePtr cur, FeedItem *item, FeedChannel *feed)
{
	gchar *href;

	href = atom10_parse_link (cur, feed, item);
	if (href) {
		feed_item_set_source (item, href);
		g_free (href);
	}
}

static void
atom10_parse_entry_rights (xmlNodePtr cur, FeedItem *item, FeedChannel *feed)
{
	gchar *rights;

	rights = atom10_parse_text_construct (cur, FALSE);
	if (rights) {
		feed_item_set_copyright (item, rights);
		g_free (rights);
	}
}

/* <summary> can be used for short text descriptions, if there is no
   <content> description we show the <summary> content */
static void
atom10_parse_entry_summary (xmlNodePtr cur, FeedItem *item, FeedChannel *feed)
{
	gchar *summary;

	summary = atom10_parse_text_construct (cur, TRUE);
	if (summary) {
		feed_item_set_description (item, summary);
		g_free (summary);
	}

	/* FIXME: set a flag to show a "Read more" link to the user; but where? */
}

static void
atom10_parse_entry_title (xmlNodePtr cur, FeedItem *item, FeedChannel *feed)
{
	gchar *title;

	title = atom10_parse_text_construct (cur, FALSE);
	if (title) {
		feed_item_set_title (item, title);
		g_free (title);
	}
}

static void
atom10_parse_entry_published (xmlNodePtr cur, FeedItem *item, FeedChannel *feed)
{
	gchar *datestr;
	time_t t;

	datestr = (gchar *)xmlNodeListGetString (cur->doc, cur->xmlChildrenNode, 1);

	/* if pubDate is already set, don't overwrite it */
	if (datestr && feed_item_get_publish_time (item) == 0) {
		t = date_parse_ISO8601 (datestr);
		feed_item_set_publish_time (item, t);
	}

	g_free (datestr);
}

/* <content> tag support, FIXME: base64 not supported */
/* method to parse standard tags for each item element */
static FeedItem*
atom10_parse_entry (FeedHandler *self, FeedChannel *feed, xmlNodePtr cur)
{
	AtomItemParserFunc func;
	FeedAtomHandler *parser;
	FeedItem *item;

	item = feed_item_new (feed);
	parser = FEED_ATOM_HANDLER (self);
	cur = cur->xmlChildrenNode;

	while (cur) {
		if (cur->type != XML_ELEMENT_NODE || cur->name == NULL || cur->ns == NULL) {
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

		if (!cur->ns->href) {
			/* This is an invalid feed... no idea what to do with the current element */
			cur = cur->next;
			continue;
		}


		if (xmlStrcmp (cur->ns->href, ATOM10_NS)) {
			cur = cur->next;
			continue;
		}

		/* At this point, the namespace must be the Atom 1.0 namespace */
		func = g_hash_table_lookup (parser->priv->entry_elements_hash, cur->name);
		if (func) {
			(*func) (cur, item, feed);
		}

		cur = cur->next;
	}

	return item;
}

static void
atom10_parse_feed_author (xmlNodePtr cur, FeedChannel *feed)
{
	gchar *author;

	author = atom10_parse_person_construct (cur);
	if (author) {
		feed_channel_set_editor (feed, author);
		g_free (author);
	}
}

static void
atom10_parse_feed_category (xmlNodePtr cur, FeedChannel *feed)
{
	gchar *label = NULL;

	label = (gchar*) xmlGetNsProp (cur, BAD_CAST"label", NULL);
	if (!label)
		label = (gchar*) xmlGetNsProp (cur, BAD_CAST"term", NULL);

	if (label) {
		gchar *escaped = g_markup_escape_text (label, -1);
		feed_channel_set_category (feed, escaped);
		g_free (escaped);
		xmlFree (label);
	}
}

static void
atom10_parse_feed_contributor (xmlNodePtr cur, FeedChannel *feed)
{
	/* parse feed contributors */
	gchar *contributer;

	contributer = atom10_parse_person_construct (cur);
	if (contributer) {
		feed_channel_add_contributor (feed, contributer);
		g_free (contributer);
	}
}

static void
atom10_parse_feed_generator (xmlNodePtr cur, FeedChannel *feed)
{
	gchar *ret;
	gchar *version;
	gchar *tmp = NULL;
	gchar *uri;

	ret = unhtmlize ((gchar*) xmlNodeListGetString (cur->doc, cur->xmlChildrenNode, 1));
	if (ret && ret [0] != '\0') {
		version = (gchar*) xmlGetNsProp (cur, BAD_CAST"version", NULL);
		if (version) {
			tmp = g_strdup_printf ("%s %s", ret, version);
			g_free (ret);
			g_free (version);
			ret = tmp;
		}

		uri = (gchar*) xmlGetNsProp (cur, BAD_CAST"uri", NULL);
		if (uri) {
			tmp = g_strdup_printf ("<a href=\"%s\">%s</a>", uri, ret);
			g_free (uri);
			g_free (ret);
			ret = tmp;
		}

		feed_channel_set_generator (feed, tmp);
	}

	g_free (ret);
}

static void
atom10_parse_feed_icon (xmlNodePtr cur, FeedChannel *feed)
{
	gchar *icon_uri;

	icon_uri = (gchar *)xmlNodeListGetString (cur->doc, cur->xmlChildrenNode, 1);
	if (icon_uri) {
		feed_channel_set_icon (feed, icon_uri);
		g_free (icon_uri);
	}
}

static void
atom10_parse_feed_link (xmlNodePtr cur, FeedChannel *feed)
{
	gchar *href;

	href = atom10_parse_link (cur, feed, NULL);
	if (href) {
		xmlChar *baseURL = xmlNodeGetBase (cur->doc, xmlDocGetRootElement (cur->doc));
		feed_channel_set_homepage (feed, href);

		/* Set the default base to the feed's HTML URL if not set yet */
		if (baseURL == NULL)
			xmlNodeSetBase (xmlDocGetRootElement (cur->doc), (xmlChar *)href);
		else
			xmlFree (baseURL);

		g_free (href);
	}
}

static void
atom10_parse_feed_logo (xmlNodePtr cur, FeedChannel *feed)
{
	gchar *logo;

	logo = atom10_parse_text_construct (cur, FALSE);
	if (logo) {
		feed_channel_set_image (feed, logo);
		g_free (logo);
	}
}

static void
atom10_parse_feed_rights (xmlNodePtr cur, FeedChannel *feed)
{
	gchar *rights;

	rights = atom10_parse_text_construct (cur, FALSE);
	if (rights) {
		feed_channel_set_copyright (feed, rights);
		g_free (rights);
	}
}

static void
atom10_parse_feed_subtitle (xmlNodePtr cur, FeedChannel *feed)
{
	gchar *subtitle;

	subtitle = atom10_parse_text_construct (cur, TRUE);
	if (subtitle) {
 		feed_channel_set_description (feed, subtitle);
		g_free (subtitle);
	}
}

static void
atom10_parse_feed_title (xmlNodePtr cur, FeedChannel *feed)
{
	gchar *title;

	title = atom10_parse_text_construct (cur, FALSE);
	if (title) {
		feed_channel_set_title (feed, title);
		g_free (title);
	}
}

static void
atom10_parse_feed_updated (xmlNodePtr cur, FeedChannel *feed)
{
	gchar *timestamp;
	time_t t;

	timestamp = (gchar *)xmlNodeListGetString (cur->doc, cur->xmlChildrenNode, 1);
	if (timestamp) {
		t = date_parse_ISO8601 (timestamp);
		feed_channel_set_update_time (feed, t);
		g_free (timestamp);
	}
}

static GList*
feed_atom_handler_parse (FeedHandler *self, FeedChannel *feed, xmlDocPtr doc, GError **error)
{
	time_t now;
	xmlNodePtr cur;
	GList *items;
	AtomChannelParserFunc func;
	FeedAtomHandler *parser;
	FeedItem *item;

	items = NULL;

	cur = xmlDocGetRootElement (doc);
	while (cur && xmlIsBlankNode (cur))
		cur = cur->next;

	now = time ( NULL );
	parser = FEED_ATOM_HANDLER (self);

	while (TRUE) {
		if (xmlStrcmp (cur->name, BAD_CAST"feed")) {
			g_set_error (error, FEED_ATOM_HANDLER_ERROR, FEED_ATOM_HANDLER_PARSE_ERROR, "Could not find Atom 1.0 header!");
			break;
		}

		/* parse feed contents */
		cur = cur->xmlChildrenNode;

		while (cur) {
		 	if (!cur->name || cur->type != XML_ELEMENT_NODE || !cur->ns) {
				cur = cur->next;
				continue;
			}

			/* check if supported namespace should handle the current tag
			   by trying to determine a namespace handler */

			if (cur->ns) {
				if (ns_handler_channel (parser->priv->handler, feed, cur)) {
					cur = cur->next;
					continue;
				}
			}

			/* check namespace of this tag */
			if (!cur->ns->href || xmlStrcmp (cur->ns->href, ATOM10_NS)) {
				/* This is an invalid feed... no idea what to do with the current element */
				cur = cur->next;
				continue;
			}

			/* At this point, the namespace must be the Atom 1.0 namespace */

			func = g_hash_table_lookup (parser->priv->feed_elements_hash, cur->name);
			if (func) {
				(*func) (cur, feed);
			}
			else if (xmlStrEqual (cur->name, BAD_CAST"entry")) {
				item = atom10_parse_entry (self, feed, cur);
				if (item) {
					if (feed_item_get_publish_time (item) == 0)
						feed_item_set_publish_time (item, now);
					items = g_list_append (items, item);
				}
			}

			cur = cur->next;
		}

		/* FIXME: Maybe check to see that the required information was actually provided (persuant to the RFC). */
		/* after parsing we fill in the infos into the feedPtr structure */

		break;
	}

	if (items != NULL)
		items = g_list_reverse (items);
	return items;
}

static void
feed_handler_interface_init (FeedHandlerInterface *iface)
{
	iface->set_ns_handler = feed_atom_handler_set_ns_handler;
	iface->check_format = feed_atom_handler_check_format;
	iface->parse = feed_atom_handler_parse;
}

static void
feed_atom_handler_class_init (FeedAtomHandlerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (FeedAtomHandlerPrivate));
	object_class->finalize = feed_atom_handler_finalize;
}

static void
feed_atom_handler_init (FeedAtomHandler *object)
{
	GHashTable *hash;

	object->priv = FEED_ATOM_HANDLER_GET_PRIVATE (object);

	hash = g_hash_table_new (g_str_hash, g_str_equal);
	g_hash_table_insert (hash, "author", &atom10_parse_feed_author);
	g_hash_table_insert (hash, "category", &atom10_parse_feed_category);
	g_hash_table_insert (hash, "contributor", &atom10_parse_feed_contributor);
	g_hash_table_insert (hash, "generator", &atom10_parse_feed_generator);
	g_hash_table_insert (hash, "icon", &atom10_parse_feed_icon);
	g_hash_table_insert (hash, "link", &atom10_parse_feed_link);
	g_hash_table_insert (hash, "logo", &atom10_parse_feed_logo);
	g_hash_table_insert (hash, "rights", &atom10_parse_feed_rights);
	g_hash_table_insert (hash, "subtitle", &atom10_parse_feed_subtitle);
	g_hash_table_insert (hash, "title", &atom10_parse_feed_title);
	g_hash_table_insert (hash, "updated", &atom10_parse_feed_updated);
	object->priv->feed_elements_hash = hash;

	hash = g_hash_table_new (g_str_hash, g_str_equal);
	g_hash_table_insert (hash, "author", &atom10_parse_entry_author);
	g_hash_table_insert (hash, "category", &atom10_parse_entry_category);
	g_hash_table_insert (hash, "content", &atom10_parse_entry_content);
	g_hash_table_insert (hash, "contributor", &atom10_parse_entry_contributor);
	g_hash_table_insert (hash, "id", &atom10_parse_entry_id);
	g_hash_table_insert (hash, "link", &atom10_parse_entry_link);
	g_hash_table_insert (hash, "published", &atom10_parse_entry_published);
	g_hash_table_insert (hash, "rights", &atom10_parse_entry_rights);
	/* FIXME: Parse "source" */
	g_hash_table_insert (hash, "summary", &atom10_parse_entry_summary);
	g_hash_table_insert (hash, "title", &atom10_parse_entry_title);
	g_hash_table_insert (hash, "updated", &atom10_parse_entry_published);
	object->priv->entry_elements_hash = hash;
}

/**
 * feed_atom_handler_new:
 *
 * Allocates a new #FeedAtomHandler
 *
 * Return value: a new #FeedAtomHandler
 */
FeedAtomHandler*
feed_atom_handler_new ()
{
	FeedAtomHandler *parser;

	parser = g_object_new (FEED_ATOM_HANDLER_TYPE, NULL);
	return parser;
}
