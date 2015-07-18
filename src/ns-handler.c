/*
 * Copyright (C) 2009-2015, Roberto Guido <rguido@src.gnome.org>
 *                          Michele Tameni <michele@amdplanet.it>
 * Copyright (C) 2015 Igor Gnatenko <ignatenko@src.gnome.org>
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
#include "ns-handler.h"

#define NS_HANDLER_GET_PRIVATE(obj)     (G_TYPE_INSTANCE_GET_PRIVATE ((obj), NS_HANDLER_TYPE, NSHandlerPrivate))

struct _NSHandlerPrivate {
	GHashTable	*href_handlers;
	GHashTable	*prefix_handlers;
};

typedef struct {
	gboolean	(*handle_channel)	(GrssFeedChannel *channel, xmlNodePtr cur);
	void		(*handle_item)		(GrssFeedItem *item, xmlNodePtr cur);
} InternalNsHandler;

G_DEFINE_TYPE (NSHandler, ns_handler, G_TYPE_OBJECT);

static void
ns_handler_finalize (GObject *obj)
{
	NSHandler *hand;

	hand = NS_HANDLER (obj);
	g_hash_table_destroy (hand->priv->href_handlers);
	g_hash_table_destroy (hand->priv->prefix_handlers);
}

static void
ns_handler_class_init (NSHandlerClass *klass)
{
	GObjectClass *gobject_class;

	g_type_class_add_private (klass, sizeof (NSHandlerPrivate));

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->finalize = ns_handler_finalize;
}

static gboolean
ns_admin_channel (GrssFeedChannel *feed, xmlNodePtr cur)
{
	gchar *value;
	gboolean ret;

	value = (gchar*) xmlGetProp (cur, BAD_CAST "resource");
	ret = FALSE;

	if (!xmlStrcmp (BAD_CAST "errorReportsTo", cur->name)) {
		grss_feed_channel_set_webmaster (feed, value);
		ret = TRUE;
	}
	else if (!xmlStrcmp (BAD_CAST "generatorAgent", cur->name)) {
		grss_feed_channel_set_generator (feed, value);
		ret = TRUE;
	}

	g_free (value);
	return ret;
}

static void
ns_content_item (GrssFeedItem *item, xmlNodePtr cur)
{
	gchar *tmp;

  	if (!xmlStrcmp (cur->name, BAD_CAST "encoded")) {
		tmp = xhtml_extract (cur, 0, NULL);
		if (tmp) {
			grss_feed_item_set_description (item, tmp);
			g_free (tmp);
		}
	}
}

static void
ns_dc_item (GrssFeedItem *item, xmlNodePtr cur)
{
	time_t t;
	gchar *value;

	value = (gchar*) xmlNodeListGetString (cur->doc, cur->xmlChildrenNode, 1);

	if (value) {
		if (!xmlStrcmp (BAD_CAST "date", cur->name)) {
			t = date_parse_ISO8601 (value);
			grss_feed_item_set_publish_time (item, t);
		}
		else if (!xmlStrcmp (BAD_CAST "title", cur->name)) {
			grss_feed_item_set_title (item, value);
		}
		else if (!xmlStrcmp (BAD_CAST "creator", cur->name)) {
			GrssPerson *person = grss_person_new (value, NULL, NULL);
			grss_feed_item_set_author (item, person);
			grss_person_unref (person);
		}
		else if (!xmlStrcmp (BAD_CAST "subject", cur->name)) {
			grss_feed_item_add_category (item, value);
		}
		else if (!xmlStrcmp (BAD_CAST "description", cur->name)) {
			grss_feed_item_set_description (item, value);
		}
		else if (!xmlStrcmp (BAD_CAST "contributor", cur->name)) {
			GrssPerson *person = grss_person_new (value, NULL, NULL);
			grss_feed_item_add_contributor (item, person);
			grss_person_unref (person);
		}
		else if (!xmlStrcmp (BAD_CAST "rights", cur->name)) {
			grss_feed_item_set_copyright (item, value);
		}

		g_free (value);
	}
}

static gboolean
ns_dc_channel (GrssFeedChannel *feed, xmlNodePtr cur)
{
	gchar *value;
	gboolean ret;

	value = (gchar*) xmlNodeListGetString (cur->doc, cur->xmlChildrenNode, 1);
	ret = FALSE;

	if (value) {
		if (!xmlStrcmp (BAD_CAST "title", cur->name)) {
			grss_feed_channel_set_title (feed, value);
			ret = TRUE;
		}
		else if (!xmlStrcmp (BAD_CAST "creator", cur->name)) {
			GrssPerson *person = grss_person_new (value, NULL, NULL);
			grss_feed_channel_set_editor (feed, person);
			grss_person_unref (person);
			ret = TRUE;
		}
		else if (!xmlStrcmp (BAD_CAST "subject", cur->name)) {
			grss_feed_channel_set_category (feed, value);
			ret = TRUE;
		}
		else if (!xmlStrcmp (BAD_CAST "description", cur->name)) {
			grss_feed_channel_set_description (feed, value);
			ret = TRUE;
		}
		else if (!xmlStrcmp (BAD_CAST "publisher", cur->name)) {
			grss_feed_channel_set_webmaster (feed, value);
			ret = TRUE;
		}
		else if (!xmlStrcmp (BAD_CAST "contributor", cur->name)) {
			GrssPerson *person = grss_person_new (value, NULL, NULL);
			grss_feed_channel_add_contributor (feed, person);
			grss_person_unref (person);
			ret = TRUE;
		}
		else if (!xmlStrcmp (BAD_CAST "rights", cur->name)) {
			grss_feed_channel_set_copyright (feed, value);
			ret = TRUE;
		}

		g_free (value);
	}

	return ret;
}

static void
ns_georss_item (GrssFeedItem *item, xmlNodePtr cur)
{
	gchar *tmp;
	gchar *sep;
	double latitude;
	double longitude;

	if (!xmlStrcmp (cur->name, BAD_CAST"point")) {
		tmp = (gchar*) xmlNodeListGetString (cur->doc, cur->xmlChildrenNode, 1);
		if (tmp) {
			sep = strchr (tmp, ' ');

			if (sep) {
				*sep = '\0';
				latitude = strtod (tmp, NULL);
				longitude = strtod (sep + 1, NULL);
				grss_feed_item_set_geo_point (item, latitude, longitude);
			}

			g_free (tmp);
		}
	}
}

static void
ns_geo_item (GrssFeedItem *item, xmlNodePtr cur)
{
	gchar *tmp;
	double latitude;
	double longitude;

	latitude = -1;
	longitude = -1;

	if (!xmlStrcmp (cur->name, BAD_CAST"lat")) {
		tmp = (gchar*) xmlNodeListGetString (cur->doc, cur->xmlChildrenNode, 1);
		if (tmp) {
			latitude = strtod (tmp, NULL);
			g_free (tmp);
		}
	}
	else if (!xmlStrcmp (cur->name, BAD_CAST"long")) {
		tmp = (gchar*) xmlNodeListGetString (cur->doc, cur->xmlChildrenNode, 1);
		if (tmp) {
			longitude = strtod (tmp, NULL);
			g_free (tmp);
		}
	}

	grss_feed_item_set_geo_point (item, latitude, longitude);
}

static void
ns_itunes_item (GrssFeedItem *item, xmlNodePtr cur)
{
	gchar *tmp;

	if (!xmlStrcmp (cur->name, BAD_CAST"author")) {
		tmp = (gchar*) xmlNodeListGetString (cur->doc, cur->xmlChildrenNode, 1);
		if (tmp) {
			GrssPerson *person = grss_person_new (tmp, NULL, NULL);
			grss_feed_item_set_author (item, person);
			grss_person_unref (person);
			g_free (tmp);
		}
	}

	if (!xmlStrcmp (cur->name, BAD_CAST"summary")) {
		tmp = xhtml_extract (cur, 0, NULL);
		grss_feed_item_add_category (item, tmp);
		g_free (tmp);
	}

	if (!xmlStrcmp(cur->name, BAD_CAST"keywords")) {
		gchar *keyword = tmp = (gchar*) xmlNodeListGetString (cur->doc, cur->xmlChildrenNode, 1);

		/* parse comma separated list and strip leading spaces... */
		while (tmp) {
			tmp = strchr (tmp, ',');
			if (tmp) {
				*tmp = 0;
				tmp++;
			}

			while (g_unichar_isspace (*keyword)) {
				keyword = g_utf8_next_char (keyword);
			}

			grss_feed_item_add_category (item, keyword);
			keyword = tmp;
		}

		g_free (tmp);
	}
}

static gboolean
ns_itunes_channel (GrssFeedChannel *feed, xmlNodePtr cur)
{
	gchar *tmp;
	const gchar *old;

	if (!xmlStrcmp (cur->name, BAD_CAST"summary") || !xmlStrcmp (cur->name, BAD_CAST"subtitle")) {
		tmp = xhtml_extract (cur, 0, NULL);

		old = grss_feed_channel_get_description (feed);
		if (!old || strlen (old) < strlen (tmp))
			grss_feed_channel_set_description (feed, tmp);

		g_free (tmp);
		return TRUE;
	}

	return FALSE;
}

static void
ns_media_item (GrssFeedItem *item, xmlNodePtr cur)
{
	gchar *tmp;
	gchar *tmp2;
	gchar *tmp3;
	GrssFeedEnclosure *enclosure;
	GrssFeedChannel *feed;

	/*
	   Maximual definition could look like this:

        	<media:content
        	       url="http://www.foo.com/movie.mov"
        	       fileSize="12216320"
        	       type="video/quicktime"
        	       medium="video"
        	       isDefault="true"
        	       expression="full"
        	       bitrate="128"
        	       framerate="25"
        	       samplingrate="44.1"
        	       channels="2"
        	       duration="185"
        	       height="200"
        	       width="300"
        	       lang="en" />

	   (example quoted from specification)
	*/
  	if (!xmlStrcmp (cur->name, BAD_CAST"content")) {
		tmp = (gchar*) xmlGetProp (cur, BAD_CAST "url");

		if (tmp) {
			/* the following code is duplicated from rss_item.c! */
			gchar *type = (gchar*) xmlGetProp (cur, BAD_CAST "type");
			gchar *lengthStr = (gchar*) xmlGetProp (cur, BAD_CAST "length");
			gchar *medium = (gchar*) xmlGetProp (cur, BAD_CAST "medium");

			gssize length = 0;
			if (lengthStr)
				length = atol (lengthStr);

			feed = grss_feed_item_get_parent (item);
			tmp3 = (gchar*) grss_feed_channel_get_homepage (feed);

			if ((strstr (tmp, "://") == NULL) &&
			    (tmp3 != NULL) &&
			    (tmp3 [0] != '|') &&
			    (strstr (tmp3, "://") != NULL)) {
				/* add base URL if necessary and possible */
				tmp2 = g_strdup_printf ("%s/%s", tmp3, tmp);
				g_free (tmp);
				tmp = tmp2;
			}

			if (medium && !strcmp (medium, "image") && strstr (tmp, "www.gravatar.com")) {
				/* gravatars are often supplied as media:content with medium='image'
				   so we treat do not treat such occurences as enclosures */
			}
			else {
				/* Never add enclosures for images already contained in the description */
				tmp2 = (gchar*) grss_feed_item_get_description (item);

				if (!(tmp2 && strstr (tmp2, tmp))) {
					enclosure = grss_feed_enclosure_new (tmp);
					grss_feed_enclosure_set_format (enclosure, type);
					grss_feed_enclosure_set_length (enclosure, length);
					grss_feed_item_add_enclosure (item, enclosure);
				}
			}

			g_free (tmp);
			g_free (type);
			g_free (medium);
			g_free (lengthStr);
		}
	}

	// FIXME: should we support media:player too?
}

static gboolean
ns_syn_channel (GrssFeedChannel *channel, xmlNodePtr cur)
{
	xmlChar	*tmp;
	gint period;
	gint frequency = 1;

	period = grss_feed_channel_get_update_interval (channel);

	if (!xmlStrcmp (cur->name, BAD_CAST"updatePeriod")) {
		if (NULL != (tmp = xmlNodeListGetString (cur->doc, cur->xmlChildrenNode, 1))) {
			if (!xmlStrcmp (tmp, BAD_CAST"hourly"))
				period = 60;
			else if (!xmlStrcmp (tmp, BAD_CAST"daily"))
				period = 60 * 24;
			else if (!xmlStrcmp (tmp, BAD_CAST"weekly"))
				period = 7 * 24 * 60;
			else if (!xmlStrcmp (tmp, BAD_CAST"monthly"))
				/* FIXME: not really exact...*/
				period = 31 * 7 * 24 * 60;
			else if (!xmlStrcmp (tmp, BAD_CAST"yearly"))
				period = 365 * 24 * 60;

			xmlFree (tmp);
		}
	}
	else if (!xmlStrcmp (cur->name, BAD_CAST"updateFrequency")) {
		tmp = xmlNodeListGetString (cur->doc, cur->xmlChildrenNode, 1);
		if (tmp) {
			frequency = atoi ((gchar*) tmp);
			xmlFree (tmp);
		}
	}

	/* postprocessing */
	if (0 != frequency)
		period /= frequency;

	grss_feed_channel_set_update_interval (channel, period);
	return TRUE;
}

static void
ns_trackback_item (GrssFeedItem *item, xmlNodePtr cur)
{
	gchar *tmp;

	/* We ignore the "ping" tag */

  	if (xmlStrcmp (cur->name, BAD_CAST"about"))
		return;

	/* RSS 1.0 */
	tmp = (gchar*) xmlGetProp (cur, BAD_CAST"about");

	/* RSS 2.0 */
	if (!tmp)
		tmp = (gchar*) xmlNodeListGetString (cur->doc, cur->xmlChildrenNode, 1);

	if (tmp) {
		grss_feed_item_set_related (item, tmp);
		g_free (tmp);
	}
}

static void
ns_wfw_item (GrssFeedItem *item, xmlNodePtr cur)
{
	gchar *uri = NULL;

 	if (!xmlStrcmp (BAD_CAST"commentRss", cur->name) || !xmlStrcmp (BAD_CAST"commentRSS", cur->name))
		uri = (gchar*) xmlNodeListGetString (cur->doc, cur->xmlChildrenNode, 1);

	if (uri) {
		grss_feed_item_set_comments_url (item, uri);
		g_free (uri);
	}
}

static gboolean
ns_atom10_channel (GrssFeedChannel *feed, xmlNodePtr cur)
{
	gchar *href;
	gchar *relation;

	if (!xmlStrcmp (BAD_CAST "link", cur->name)) {
		relation = (gchar*) xmlGetNsProp (cur, BAD_CAST "rel", NULL);

		if (relation != NULL) {
			href = (gchar*) xmlGetNsProp (cur, BAD_CAST "href", NULL);

			if (strcmp (relation, "self") == 0)
				grss_feed_channel_set_source (feed, href);
			else if (strcmp (relation, "hub") == 0)
				grss_feed_channel_set_pubsubhub (feed, href);

			g_free (relation);
			g_free (href);
			return TRUE;
		}
	}

	return FALSE;
}

/*
	TODO	Look at
		http://www.dmoz.org/Computers/Internet/On_the_Web/Syndication_and_Feeds/RSS/Specifications/RSS_1.0_Modules/
		and
		http://www.dmoz.org/Computers/Internet/On_the_Web/Syndication_and_Feeds/RSS/Specifications/RSS_2.0_Modules/
		for more modules to be implemented
*/
static void
ns_handler_init (NSHandler *node)
{
	InternalNsHandler *nsh;

	node->priv = NS_HANDLER_GET_PRIVATE (node);
	memset (node->priv, 0, sizeof (NSHandlerPrivate));

	node->priv->href_handlers = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, g_free);
	node->priv->prefix_handlers = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, g_free);

	nsh = g_new0 (InternalNsHandler, 1);
	nsh->handle_channel = ns_admin_channel;
	nsh->handle_item = NULL;
	g_hash_table_insert (node->priv->prefix_handlers, "admin", nsh);
	g_hash_table_insert (node->priv->href_handlers, "http://webns.net/mvcb/", nsh);

	/*

	nsh = g_new0 (InternalNsHandler, 1);
	nsh->handle_channel = NULL;
	nsh->handle_item = ns_aggregation_item;
	g_hash_table_insert (node->priv->prefix_handlers, "ag", nsh);
	g_hash_table_insert (node->priv->href_handlers, "http://purl.org/rss/1.0/modules/aggregation/", nsh);

	nsh = g_new0 (InternalNsHandler, 1);
	nsh->handle_channel = ns_blog_channel;
	nsh->handle_item = NULL;
	g_hash_table_insert (node->priv->prefix_handlers, "blogChannel", nsh);
	g_hash_table_insert (node->priv->href_handlers, "http://backend.userland.com/blogChannelModule", nsh);

	nsh = g_new0 (InternalNsHandler, 1);
	nsh->handle_channel = ns_creativecommons_channel;
	nsh->handle_item = ns_creativecommons_item;
	g_hash_table_insert (node->priv->prefix_handlers, "cc", nsh);
	g_hash_table_insert (node->priv->prefix_handlers, "creativeCommons", nsh);
	g_hash_table_insert (node->priv->href_handlers, "http://web.resource.org/cc/", nsh);
	g_hash_table_insert (node->priv->href_handlers, "http://backend.userland.com/creativeCommonsRssModule", nsh);

	*/

	nsh = g_new0 (InternalNsHandler, 1);
	nsh->handle_channel = NULL;
	nsh->handle_item = ns_content_item;
	g_hash_table_insert (node->priv->prefix_handlers, "content", nsh);
	g_hash_table_insert (node->priv->href_handlers, "http://purl.org/rss/1.0/modules/content/", nsh);

	nsh = g_new0 (InternalNsHandler, 1);
	nsh->handle_channel = ns_dc_channel;
	nsh->handle_item = ns_dc_item;
	g_hash_table_insert (node->priv->prefix_handlers, "dc", nsh);
	g_hash_table_insert (node->priv->href_handlers, "http://purl.org/dc/elements/1.1/", nsh);
	g_hash_table_insert (node->priv->href_handlers, "http://purl.org/dc/elements/1.0/", nsh);

	nsh = g_new0 (InternalNsHandler, 1);
	nsh->handle_channel = NULL;
	nsh->handle_item = ns_georss_item;
	g_hash_table_insert (node->priv->prefix_handlers, "georss", nsh);
	g_hash_table_insert (node->priv->href_handlers, "http://www.georss.org/georss", nsh);

	nsh = g_new0 (InternalNsHandler, 1);
	nsh->handle_channel = NULL;
	nsh->handle_item = ns_geo_item;
	g_hash_table_insert (node->priv->prefix_handlers, "geo", nsh);
	g_hash_table_insert (node->priv->href_handlers, "http://www.w3.org/2003/01/geo/wgs84_pos#", nsh);

	nsh = g_new0 (InternalNsHandler, 1);
	nsh->handle_channel = ns_itunes_channel;
	nsh->handle_item = ns_itunes_item;
	g_hash_table_insert (node->priv->prefix_handlers, "itunes", nsh);
	g_hash_table_insert (node->priv->href_handlers, "http://www.itunes.com/dtds/podcast-1.0.dtd", nsh);

	nsh = g_new0 (InternalNsHandler, 1);
	nsh->handle_channel = NULL;
	nsh->handle_item = ns_media_item;
	g_hash_table_insert (node->priv->prefix_handlers, "media", nsh);
	g_hash_table_insert (node->priv->href_handlers, "http://search.yahoo.com/mrss", nsh);

	/*

	nsh = g_new0 (InternalNsHandler, 1);
	nsh->handle_channel = NULL;
	nsh->handle_item = ns_photo_item;
	g_hash_table_insert (node->priv->prefix_handlers, "photo", nsh);
	g_hash_table_insert (node->priv->href_handlers, "http://www.pheed.com/pheed/", nsh);

	nsh = g_new0 (InternalNsHandler, 1);
	nsh->handle_channel = NULL;
	nsh->handle_item = ns_slash_item;
	g_hash_table_insert (node->priv->prefix_handlers, "slash", nsh);
	g_hash_table_insert (node->priv->href_handlers, "http://purl.org/rss/1.0/modules/slash/", nsh);

	*/

	nsh = g_new0 (InternalNsHandler, 1);
	nsh->handle_channel = ns_syn_channel;
	nsh->handle_item = NULL;
	g_hash_table_insert (node->priv->prefix_handlers, "syn", nsh);
	g_hash_table_insert (node->priv->href_handlers, "http://purl.org/rss/1.0/modules/syndication/", nsh);

	nsh = g_new0 (InternalNsHandler, 1);
	nsh->handle_channel = NULL;
	nsh->handle_item = ns_trackback_item;
	g_hash_table_insert (node->priv->prefix_handlers, "trackback", nsh);
	g_hash_table_insert (node->priv->href_handlers, "http://madskills.com/public/xml/rss/module/trackback/", nsh);

	nsh = g_new0 (InternalNsHandler, 1);
	nsh->handle_channel = NULL;
	nsh->handle_item = ns_wfw_item;
	g_hash_table_insert (node->priv->prefix_handlers, "wfw", nsh);
	g_hash_table_insert (node->priv->href_handlers, "http://wellformedweb.org/CommentAPI", nsh);

	nsh = g_new0 (InternalNsHandler, 1);
	nsh->handle_channel = ns_atom10_channel;
	nsh->handle_item = NULL;
	g_hash_table_insert (node->priv->prefix_handlers, "atom10", nsh);
	g_hash_table_insert (node->priv->href_handlers, "http://www.w3.org/2005/Atom", nsh);
}

NSHandler*
ns_handler_new ()
{
	return g_object_new (NS_HANDLER_TYPE, NULL);
}

static InternalNsHandler*
retrieve_internal_handler (NSHandler *handler, xmlNodePtr cur)
{
	InternalNsHandler *nsh;

	nsh = NULL;

	if (cur->ns->href)
		nsh = (InternalNsHandler*) g_hash_table_lookup (handler->priv->href_handlers, (gpointer) cur->ns->href);

	if (nsh == NULL && cur->ns->prefix)
		nsh = (InternalNsHandler*) g_hash_table_lookup (handler->priv->prefix_handlers, (gpointer) cur->ns->prefix);

	return nsh;
}

gboolean
ns_handler_channel (NSHandler *handler, GrssFeedChannel *feed, xmlNodePtr cur)
{
	InternalNsHandler *nsh;

	nsh = retrieve_internal_handler (handler, cur);

	if (nsh != NULL && nsh->handle_channel != NULL)
		return nsh->handle_channel (feed, cur);
	else
		return FALSE;
}

gboolean
ns_handler_item (NSHandler *handler, GrssFeedItem *item, xmlNodePtr cur)
{
	InternalNsHandler *nsh;

	nsh = retrieve_internal_handler (handler, cur);

	if (nsh != NULL && nsh->handle_item != NULL) {
		nsh->handle_item (item, cur);
		return TRUE;
	}
	else {
		return FALSE;
	}
}
