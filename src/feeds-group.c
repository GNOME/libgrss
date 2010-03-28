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
#include "feeds-group.h"
#include "feeds-group-handler.h"

#include "feeds-opml-group-handler.h"

#define FEEDS_GROUP_GET_PRIVATE(o)	(G_TYPE_INSTANCE_GET_PRIVATE ((o), FEEDS_GROUP_TYPE, FeedsGroupPrivate))

/**
 * SECTION: feeds-group
 * @short_description: import and export group of channels
 *
 * #FeedsGroup is an utility to import and export list of #FeedChannels in
 * different formats, such as OPML
 */

#define FEEDS_GROUP_ERROR		feeds_group_error_quark()

struct _FeedsGroupPrivate {
	GSList *handlers;
};

enum {
	FEEDS_GROUP_PARSE_ERROR,
};

G_DEFINE_TYPE (FeedsGroup, feeds_group, G_TYPE_OBJECT)

static GQuark
feeds_group_error_quark ()
{
	return g_quark_from_static_string ("feeds_group_error");
}

static void
feeds_group_finalize (GObject *object)
{
	FeedsGroup *group;

	group = FEEDS_GROUP (object);
	G_OBJECT_CLASS (feeds_group_parent_class)->finalize (object);
}

static void
feeds_group_class_init (FeedsGroupClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (FeedsGroupPrivate));
	object_class->finalize = feeds_group_finalize;
}

static void
feeds_group_init (FeedsGroup *object)
{
	object->priv = FEEDS_GROUP_GET_PRIVATE (object);
}

static GSList*
feeds_groups_get_list (FeedsGroup *group)
{
	FeedsGroupHandler *parser;

	if (group->priv->handlers == NULL) {
		/*
			TODO	Parsers may be dinamically loaded and managed as external plugins
		*/

		parser = FEEDS_GROUP_HANDLER (feeds_opml_group_handler_new ());
		group->priv->handlers = g_slist_append (group->priv->handlers, parser);
	}

	return group->priv->handlers;
}

/**
 * feeds_group_new:
 *
 * Allocates a new #FeedsGroup
 *
 * Return value: a new #FeedsGroup
 */
FeedsGroup*
feeds_group_new ()
{
	FeedsGroup *group;

	group = g_object_new (FEEDS_GROUP_TYPE, NULL);
	return group;
}

static FeedsGroupHandler*
retrieve_group_handler (FeedsGroup *group, xmlDocPtr doc, xmlNodePtr cur)
{
	GSList *iter;
	FeedsGroupHandler *handler;

	iter = feeds_groups_get_list (group);

	while (iter) {
		handler = (FeedsGroupHandler*) (iter->data);

		if (handler && feeds_group_handler_check_format (handler, doc, cur))
			return handler;

		iter = g_slist_next (iter);
	}

	return NULL;
}

GList*
feeds_group_parse_file (FeedsGroup *groups, const gchar *path, GError *error)
{
	gchar *contents;
	gsize len;
	GList *items;
	GError *err;
	xmlDocPtr doc;
	xmlNodePtr cur;
	FeedsGroupHandler *handler;

	items = NULL;
	doc = NULL;
	contents = NULL;

	do {
		err = NULL;
		if (g_file_get_contents (path, &contents, &len, &err) == FALSE) {
			g_propagate_error (&error, err);
			break;
		}

		doc = content_to_xml (contents, len);

		g_set_error (&error, FEEDS_GROUP_ERROR, FEEDS_GROUP_PARSE_ERROR, "Empty document");

		if ((cur = xmlDocGetRootElement (doc)) == NULL)
			break;

		while (cur && xmlIsBlankNode (cur))
			cur = cur->next;

		if (!cur)
			break;

		if (!cur->name) {
			g_set_error (&error, FEEDS_GROUP_ERROR, FEEDS_GROUP_PARSE_ERROR, "Invalid XML");
			break;
		}

		handler = retrieve_group_handler (groups, doc, cur);
		if (handler == NULL)
			break;

		items = feeds_group_handler_parse (handler, doc, error);

	} while (0);

	if (doc != NULL)
		xmlFreeDoc (doc);

	if (contents != NULL)
		g_free (contents);

	return items;
}

gboolean
feeds_group_export_file (FeedsGroup *groups, GList *channels, const gchar *path, GError *error)
{
	return FALSE;
}
