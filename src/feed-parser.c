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
 * feed_parser.c  parsing of different feed formats
 * Copyright (C) 2008 Lars Lindner <lars.lindner@gmail.com>
 */

#include "utils.h"
#include "feed-parser.h"
#include "feed-handler.h"

#include "feed-rss-handler.h"
#include "feed-atom-handler.h"
#include "feed-pie-handler.h"

#define FEED_PARSER_GET_PRIVATE(o)	(G_TYPE_INSTANCE_GET_PRIVATE ((o), FEED_PARSER_TYPE, FeedParserPrivate))

/**
 * SECTION: feed-parser
 * @short_description: feed parser
 *
 * The #FeedParser is a wrapper to the many handlers available: given a
 * #FeedChannel provides to identify his type and invoke the correct parser.
 */

#define FEED_PARSER_ERROR		feed_parser_error_quark()

struct _FeedParserPrivate {
	GSList *handlers;
};

enum {
	FEED_PARSER_PARSE_ERROR,
};

G_DEFINE_TYPE (FeedParser, feed_parser, G_TYPE_OBJECT)

static GQuark
feed_parser_error_quark ()
{
	return g_quark_from_static_string ("feed_parser_error");
}

static void
feed_parser_finalize (GObject *object)
{
	FeedParser *parser;

	parser = FEED_PARSER (object);
	G_OBJECT_CLASS (feed_parser_parent_class)->finalize (object);
}

static void
feed_parser_class_init (FeedParserClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (FeedParserPrivate));
	object_class->finalize = feed_parser_finalize;
}

static void
feed_parser_init (FeedParser *object)
{
	object->priv = FEED_PARSER_GET_PRIVATE (object);
}

static GSList*
feed_parsers_get_list (FeedParser *parser)
{
	FeedHandler *feed;
	NSHandler *ns;

	ns = ns_handler_new ();

	if (parser->priv->handlers == NULL) {
		/*
			TODO	Parsers may be dinamically loaded and managed as external plugins
		*/

		feed = FEED_HANDLER (feed_rss_handler_new ());
		feed_handler_set_ns_handler (feed, ns);
		parser->priv->handlers = g_slist_append (parser->priv->handlers, feed);

		feed = FEED_HANDLER (feed_atom_handler_new ());					/* Must be before pie */
		feed_handler_set_ns_handler (feed, ns);
		parser->priv->handlers = g_slist_append (parser->priv->handlers, feed);

		feed = FEED_HANDLER (feed_pie_handler_new ());
		feed_handler_set_ns_handler (feed, ns);
		parser->priv->handlers = g_slist_append (parser->priv->handlers, feed);
	}

	return parser->priv->handlers;
}

/**
 * feed_parser_new:
 *
 * Allocates a new #FeedParser
 *
 * Return value: a new #FeedParser
 */
FeedParser*
feed_parser_new ()
{
	FeedParser *parser;

	parser = g_object_new (FEED_PARSER_TYPE, NULL);
	return parser;
}

static FeedHandler*
retrieve_feed_handler (FeedParser *parser, xmlDocPtr doc, xmlNodePtr cur)
{
	GSList *iter;
	FeedHandler *handler;

	iter = feed_parsers_get_list (parser);

	while (iter) {
		handler = (FeedHandler*) (iter->data);

		if (handler && feed_handler_check_format (handler, doc, cur))
			return handler;

		iter = g_slist_next (iter);
	}

	return NULL;
}

/**
 * feed_parser_parse:
 * @parser: a #FeedParser
 * @feed: a #FeedChannel to be parsed
 * @doc: XML document extracted from the contents of the feed, which must
 * already been fetched
 * @error: location for eventual errors
 *
 * Parses the given XML @doc, belonging to the given @feed, to obtain a list
 * of #FeedItem
 *
 * Return value: a list of #FeedItem, to be freed when no longer in use, or
 * NULL if an error occours and @error is set
 */
GList*
feed_parser_parse (FeedParser *parser, FeedChannel *feed, xmlDocPtr doc, GError **error)
{
	xmlNodePtr cur;
	GList *items;
	FeedHandler *handler;

	items = NULL;

	do {
		g_set_error (error, FEED_PARSER_ERROR, FEED_PARSER_PARSE_ERROR, "Empty document!");

		if ((cur = xmlDocGetRootElement (doc)) == NULL)
			break;

		while (cur && xmlIsBlankNode (cur))
			cur = cur->next;

		if (!cur)
			break;

		if (!cur->name) {
			g_set_error (error, FEED_PARSER_ERROR, FEED_PARSER_PARSE_ERROR, "Invalid XML!");
			break;
		}

		handler = retrieve_feed_handler (parser, doc, cur);
		if (handler == NULL)
			break;

		items = feed_handler_parse (handler, feed, doc, error);

	} while (0);

	return items;
}
