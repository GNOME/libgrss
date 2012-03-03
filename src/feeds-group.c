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

#include "utils.h"
#include "feeds-group.h"
#include "feeds-group-handler.h"

#include "feeds-opml-group-handler.h"
#include "feeds-xoxo-group-handler.h"
#include "feeds-xbel-group-handler.h"

#define FEEDS_GROUP_GET_PRIVATE(o)	(G_TYPE_INSTANCE_GET_PRIVATE ((o), FEEDS_GROUP_TYPE, GrssFeedsGroupPrivate))

/**
 * SECTION: feeds-group
 * @short_description: import and export group of channels
 *
 * #GrssFeedsGroup is an utility to import and export list of #GrssFeedChannels in
 * different formats, such as OPML and XOXO.
 */

#define FEEDS_GROUP_ERROR		grss_feeds_group_error_quark()

struct _GrssFeedsGroupPrivate {
	GSList *handlers;
};

enum {
	FEEDS_GROUP_PARSE_ERROR,
};

G_DEFINE_TYPE (GrssFeedsGroup, grss_feeds_group, G_TYPE_OBJECT)

static GQuark
grss_feeds_group_error_quark ()
{
	return g_quark_from_static_string ("grss_feeds_group_error");
}

static void
grss_feeds_group_finalize (GObject *object)
{
	G_OBJECT_CLASS (grss_feeds_group_parent_class)->finalize (object);
}

static void
grss_feeds_group_class_init (GrssFeedsGroupClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (GrssFeedsGroupPrivate));
	object_class->finalize = grss_feeds_group_finalize;
}

static void
grss_feeds_group_init (GrssFeedsGroup *object)
{
	object->priv = FEEDS_GROUP_GET_PRIVATE (object);
}

static GSList*
feeds_groups_get_list (GrssFeedsGroup *group)
{
	GrssFeedsGroupHandler *parser;

	if (group->priv->handlers == NULL) {
		/*
			TODO	Parsers may be dinamically loaded and managed as external plugins
		*/

		parser = FEEDS_GROUP_HANDLER (feeds_opml_group_handler_new ());
		group->priv->handlers = g_slist_append (group->priv->handlers, parser);

		parser = FEEDS_GROUP_HANDLER (feeds_xoxo_group_handler_new ());
		group->priv->handlers = g_slist_append (group->priv->handlers, parser);

		parser = FEEDS_GROUP_HANDLER (feeds_xbel_group_handler_new ());
		group->priv->handlers = g_slist_append (group->priv->handlers, parser);
	}

	return group->priv->handlers;
}

/**
 * grss_feeds_group_new:
 *
 * Allocates a new #GrssFeedsGroup
 *
 * Return value: a new #GrssFeedsGroup
 */
GrssFeedsGroup*
grss_feeds_group_new ()
{
	GrssFeedsGroup *group;

	group = g_object_new (FEEDS_GROUP_TYPE, NULL);
	return group;
}

static GrssFeedsGroupHandler*
retrieve_group_handler (GrssFeedsGroup *group, xmlDocPtr doc, xmlNodePtr cur)
{
	GSList *iter;
	GrssFeedsGroupHandler *handler;

	iter = feeds_groups_get_list (group);

	while (iter) {
		handler = (GrssFeedsGroupHandler*) (iter->data);

		if (handler && grss_feeds_group_handler_check_format (handler, doc, cur))
			return handler;

		iter = g_slist_next (iter);
	}

	g_warning ("No suitable parser has been found.");
	return NULL;
}

/**
 * grss_feeds_group_get_formats:
 * @group: a #GrssFeedsGroupClass
 *
 * Returns the list of supported file formats
 *
 * Return value: a list of constant strings with names of supported formats. The list must be
 * freed when no longer used
 */
GList*
grss_feeds_group_get_formats (GrssFeedsGroup *group)
{
	GSList *iter;
	GList *names;
	GrssFeedsGroupHandler *handler;

	iter = feeds_groups_get_list (group);
	names = NULL;

	while (iter) {
		handler = (GrssFeedsGroupHandler*) (iter->data);
		names = g_list_prepend (names, grss_feeds_group_handler_get_name (handler));
		iter = g_slist_next (iter);
	}

	return names;
}

/**
 * grss_feeds_group_parse_file:
 * @group: a #GrssFeedsGroup
 * @path: path of the file to parse
 * @error: location for eventual errors
 *
 * Parses the given file to obtain list of listed feeds
 *
 * Return value: a list of #GrssFeedChannels, or NULL if an error occours and
 * @error is set
 */
GList*
grss_feeds_group_parse_file (GrssFeedsGroup *group, const gchar *path, GError **error)
{
	GList *items;
	xmlDocPtr doc;
	xmlNodePtr cur;
	GrssFeedsGroupHandler *handler;

	items = NULL;
	doc = NULL;

	do {
		doc = file_to_xml (path);
		g_set_error (error, FEEDS_GROUP_ERROR, FEEDS_GROUP_PARSE_ERROR, "Empty document");

		if ((cur = xmlDocGetRootElement (doc)) == NULL)
			break;

		while (cur && xmlIsBlankNode (cur))
			cur = cur->next;

		if (!cur)
			break;

		if (!cur->name) {
			g_set_error (error, FEEDS_GROUP_ERROR, FEEDS_GROUP_PARSE_ERROR, "Invalid XML");
			break;
		}

		handler = retrieve_group_handler (group, doc, cur);
		if (handler == NULL)
			break;

		items = grss_feeds_group_handler_parse (handler, doc, error);

	} while (0);

	if (doc != NULL)
		xmlFreeDoc (doc);

	return items;
}

/**
 * grss_feeds_group_export_file:
 * @group:
 * @channels:
 * @format:
 * @uri:
 * @error:
 *
 * Return value: %FALSE
 */
gboolean
grss_feeds_group_export_file (GrssFeedsGroup *group, GList *channels, const gchar *format, const gchar *uri, GError **error)
{
	gboolean ret;
	gchar *contents;
	gsize written;
	GSList *iter;
	GError *err;
	GFile *file;
	GFileOutputStream *stream;
	GrssFeedsGroupHandler *handler;

	contents = NULL;
	stream = NULL;
	iter = feeds_groups_get_list (group);

	while (iter) {
		handler = (GrssFeedsGroupHandler*) (iter->data);

		if (strcasecmp (grss_feeds_group_handler_get_name (handler), format) == 0) {
			err = NULL;
			contents = grss_feeds_group_handler_dump (handler, channels, &err);

			if (contents == NULL) {
				g_propagate_error (error, err);
				ret = FALSE;
				break;
			}

			file = g_file_new_for_uri (uri);
			stream = g_file_append_to (file, G_FILE_CREATE_NONE, NULL, &err);

			if (stream == NULL) {
				g_propagate_error (error, err);
				ret = FALSE;
				break;
			}

			if (g_output_stream_write_all (G_OUTPUT_STREAM (stream), contents, strlen (contents), &written, NULL, &err) == FALSE) {
				g_propagate_error (error, err);
				ret = FALSE;
				break;
			}

			ret = TRUE;
			break;
		}
	}

	if (stream != NULL)
		g_object_unref (stream);
	if (contents != NULL)
		g_free (contents);

	return ret;
}
