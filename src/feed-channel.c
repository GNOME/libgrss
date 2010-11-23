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

#include "utils.h"
#include "feed-channel.h"
#include "feed-parser.h"

#define FEED_CHANNEL_GET_PRIVATE(obj)     (G_TYPE_INSTANCE_GET_PRIVATE ((obj), FEED_CHANNEL_TYPE, GrssFeedChannelPrivate))

/**
 * SECTION: feed-channel
 * @short_description: a feed
 *
 * #GrssFeedChannel rappresents a single feed which may be fetched and parsed
 */

#define FEEDS_CHANNEL_ERROR		feeds_channel_error_quark()

typedef struct {
	gchar	*hub;
	gchar	*self;
} PubSub;

struct _GrssFeedChannelPrivate {
	gchar	*source;

	gchar	*title;
	gchar	*homepage;
	gchar	*description;
	gchar	*image;
	gchar	*icon;
	gchar	*language;
	gchar	*category;
	PubSub	pubsub;

	gchar	*copyright;
	gchar	*editor;
	GList	*contributors;
	gchar	*webmaster;
	gchar	*generator;

	time_t	pub_time;
	time_t	update_time;
	int	update_interval;
};

enum {
	FEEDS_CHANNEL_FETCH_ERROR,
};

G_DEFINE_TYPE (GrssFeedChannel, grss_feed_channel, G_TYPE_OBJECT);

static GQuark
feeds_channel_error_quark ()
{
	return g_quark_from_static_string ("feeds_channel_error");
}

static void
grss_feed_channel_finalize (GObject *obj)
{
	GList *iter;
	GrssFeedChannel *chan;

	chan = FEED_CHANNEL (obj);
	FREE_STRING (chan->priv->title);
	FREE_STRING (chan->priv->homepage);
	FREE_STRING (chan->priv->description);
	FREE_STRING (chan->priv->image);
	FREE_STRING (chan->priv->icon);
	FREE_STRING (chan->priv->language);
	FREE_STRING (chan->priv->category);
	FREE_STRING (chan->priv->pubsub.hub);
	FREE_STRING (chan->priv->pubsub.self);
	FREE_STRING (chan->priv->copyright);
	FREE_STRING (chan->priv->editor);
	FREE_STRING (chan->priv->webmaster);
	FREE_STRING (chan->priv->generator);

	if (chan->priv->contributors != NULL) {
		for (iter = chan->priv->contributors; iter; iter = g_list_next (iter))
			g_free (iter->data);
		g_list_free (chan->priv->contributors);
	}
}

static void
grss_feed_channel_class_init (GrssFeedChannelClass *klass)
{
	GObjectClass *gobject_class;

	g_type_class_add_private (klass, sizeof (GrssFeedChannelPrivate));

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->finalize = grss_feed_channel_finalize;
}

static void
grss_feed_channel_init (GrssFeedChannel *node)
{
	node->priv = FEED_CHANNEL_GET_PRIVATE (node);
	memset (node->priv, 0, sizeof (GrssFeedChannelPrivate));
}

/**
 * grss_feed_channel_new:
 *
 * Allocates a new #GrssFeedChannel
 *
 * Return value: a #GrssFeedChannel
 */
GrssFeedChannel*
grss_feed_channel_new ()
{
	return g_object_new (FEED_CHANNEL_TYPE, NULL);
}

/**
 * grss_feed_channel_new_from_file:
 * @path: path of the file to parse
 *
 * Allocates a new #GrssFeedChannel and init it with contents found in specified
 * file
 *
 * Return value: a #GrssFeedChannel, or NULL if the file in @path is not a valid
 * document
 */
GrssFeedChannel*
grss_feed_channel_new_from_file (const gchar *path)
{
	GList *items;
	GList *iter;
	xmlDocPtr doc;
	GrssFeedParser *parser;
	GrssFeedChannel *ret;

	/*
		TODO	This function is quite inefficent because parses all
			the feed with a GrssFeedParser and then trash obtained
			GrssFeedItems. Perhaps a more aimed function in
			GrssFeedParser would help...
	*/

	ret = NULL;
	doc = file_to_xml (path);

	if (doc != NULL) {
		ret = g_object_new (FEED_CHANNEL_TYPE, NULL);
		parser = grss_feed_parser_new ();
		items = grss_feed_parser_parse (parser, ret, doc, NULL);

		if (items != NULL) {
			for (iter = items; iter; iter = g_list_next (iter))
				g_object_unref (iter->data);
			g_list_free (items);
		}

		g_object_unref (parser);
		xmlFreeDoc (doc);
	}

	return ret;
}

/**
 * grss_feed_channel_set_source:
 * @channel: a #GrssFeedChannel
 * @source: URL of the feed
 *
 * To assign the URL where to fetch the feed
 */
void
grss_feed_channel_set_source (GrssFeedChannel *channel, gchar *source)
{
	FREE_STRING (channel->priv->source);
	channel->priv->source = g_strdup (source);
}

/**
 * grss_feed_channel_get_source:
 * @channel: a #GrssFeedChannel
 *
 * Retrieves URL where to fetch the @channel
 *
 * Return value: URL of the channel
 */
const gchar*
grss_feed_channel_get_source (GrssFeedChannel *channel)
{
	return (const gchar*) channel->priv->source;
}

/**
 * grss_feed_channel_set_title:
 * @channel: a #GrssFeedChannel
 * @title: title of the feed
 *
 * To set a title to the @channel
 */
void
grss_feed_channel_set_title (GrssFeedChannel *channel, gchar *title)
{
	FREE_STRING (channel->priv->title);
	channel->priv->title = g_strdup (title);
}

/**
 * grss_feed_channel_get_title:
 * @channel: a #GrssFeedChannel
 *
 * Retrieves title of the @channel
 *
 * Return value: title of the feed, or NULL
 */
const gchar*
grss_feed_channel_get_title (GrssFeedChannel *channel)
{
	return (const gchar*) channel->priv->title;
}

/**
 * grss_feed_channel_set_homepage:
 * @channel: a #GrssFeedChannel
 * @homepage: homepage for the main website
 *
 * To set the homepage of the site the @channel belongs
 */
void
grss_feed_channel_set_homepage (GrssFeedChannel *channel, gchar *homepage)
{
	FREE_STRING (channel->priv->homepage);
	channel->priv->homepage = g_strdup (homepage);
}

/**
 * grss_feed_channel_get_homepage:
 * @channel: a #GrssFeedChannel
 *
 * Retrieves the homepage of the site for which @channel is the feed
 *
 * Return value: reference homepage of the feed, or NULL
 */
const gchar*
grss_feed_channel_get_homepage (GrssFeedChannel *channel)
{
	return (const gchar*) channel->priv->homepage;
}

/**
 * grss_feed_channel_set_description:
 * @channel: a #GrssFeedChannel
 * @description: description of the feed
 *
 * To set the description of @channel
 */
void
grss_feed_channel_set_description (GrssFeedChannel *channel, gchar *description)
{
	FREE_STRING (channel->priv->description);
	channel->priv->description = g_strdup (description);
}

/**
 * grss_feed_channel_get_description:
 * @channel: a #GrssFeedChannel
 *
 * Retrieves the description of @channel
 *
 * Return value: description of the feed, or NULL
 */
const gchar*
grss_feed_channel_get_description (GrssFeedChannel *channel)
{
	return (const gchar*) channel->priv->description;
}

/**
 * grss_feed_channel_set_image:
 * @channel: a #GrssFeedChannel
 * @image: URL of the image
 *
 * To set a rappresentative image to @channel
 */
void
grss_feed_channel_set_image (GrssFeedChannel *channel, gchar *image)
{
	FREE_STRING (channel->priv->image);
	channel->priv->image = g_strdup (image);
}

/**
 * grss_feed_channel_get_image:
 * @channel: a #GrssFeedChannel
 *
 * Retrieves the URL of the image assigned to the channel
 *
 * Return value: URL of the image, or NULL
 */
const gchar*
grss_feed_channel_get_image (GrssFeedChannel *channel)
{
	return (const gchar*) channel->priv->image;
}

/**
 * grss_feed_channel_set_icon:
 * @channel: a #GrssFeedChannel
 * @icon: URL where to retrieve the favicon
 *
 * To set the URL of the icon rappresenting @channel
 */
void
grss_feed_channel_set_icon (GrssFeedChannel *channel, gchar *icon)
{
	FREE_STRING (channel->priv->icon);
	channel->priv->icon = g_strdup (icon);
}

/**
 * grss_feed_channel_get_icon:
 * @channel: a #GrssFeedChannel
 *
 * Retrieves URL of the favicon of the channel (and/or the website for which
 * this is the feed)
 *
 * Return value: URL of the favicon, or NULL
 */
const gchar*
grss_feed_channel_get_icon (GrssFeedChannel *channel)
{
	return (const gchar*) channel->priv->icon;
}

/**
 * grss_feed_channel_set_language:
 * @channel: a #GrssFeedChannel
 * @language: string holding the language of the feed
 *
 * To set the language of @channel
 */
void
grss_feed_channel_set_language (GrssFeedChannel *channel, gchar *language)
{
	FREE_STRING (channel->priv->language);
	channel->priv->language = g_strdup (language);
}

/**
 * grss_feed_channel_get_language:
 * @channel: a #GrssFeedChannel
 *
 * Retrieves the language of the @channel
 *
 * Return value: string rappresenting the language of channel
 */
const gchar*
grss_feed_channel_get_language (GrssFeedChannel *channel)
{
	return (const gchar*) channel->priv->language;
}

/**
 * grss_feed_channel_set_category:
 * @channel: a #GrssFeedChannel
 * @category: category of the feed
 *
 * To set the category of the @channel
 */
void
grss_feed_channel_set_category (GrssFeedChannel *channel, gchar *category)
{
	FREE_STRING (channel->priv->category);
	channel->priv->category = g_strdup (category);
}

/**
 * grss_feed_channel_get_category:
 * @channel: a #GrssFeedChannel
 *
 * Retrieves category of the @channel
 *
 * Return value: category of the feed, or NULL
 */
const gchar*
grss_feed_channel_get_category (GrssFeedChannel *channel)
{
	return (const gchar*) channel->priv->category;
}

/**
 * grss_feed_channel_set_pubsubhub:
 * @channel: a #GrssFeedChannel
 * @hub: hub for the feed, or NULL
 * @self: target referencing the feed, or NULL
 *
 * To set information about PubSubHubbub for the channel. Options can be set
 * alternatively, only with hub != %NULL or self != %NULL, and are saved
 * internally to the object: the hub is considered valid
 * (grss_feed_channel_get_pubsubhub() returns %TRUE) only when both parameters has
 * been set. To unset the hub, pass %NULL for both parameters
 */
void
grss_feed_channel_set_pubsubhub (GrssFeedChannel *channel, gchar *hub, gchar *self)
{
	if (hub == NULL && self == NULL) {
		FREE_STRING (channel->priv->pubsub.hub);
		FREE_STRING (channel->priv->pubsub.self);
	}
	else {
		if (hub != NULL) {
			FREE_STRING (channel->priv->pubsub.hub);
			channel->priv->pubsub.hub = g_strdup (hub);
		}
		if (self != NULL) {
			FREE_STRING (channel->priv->pubsub.self);
			channel->priv->pubsub.self = g_strdup (self);
		}
	}
}

/**
 * grss_feed_channel_get_pubsubhub:
 * @channel: a #GrssFeedChannel
 * @hub: location for the hub string, or NULL
 * @self: location for the reference to the feed, or NULL
 *
 * Retrieves information about the PubSubHubbub hub of the channel
 *
 * Return value: %TRUE if a valid PubSubHubbub hub has been set for the
 * @channel, %FALSE otherwise
 */
gboolean
grss_feed_channel_get_pubsubhub (GrssFeedChannel *channel, gchar **hub, gchar **self)
{
	if (hub != NULL)
		*hub = channel->priv->pubsub.hub;
	if (self != NULL)
		*self = channel->priv->pubsub.self;

	return (channel->priv->pubsub.hub != NULL && channel->priv->pubsub.self != NULL);
}

/**
 * grss_feed_channel_set_copyright:
 * @channel: a #GrssFeedChannel
 * @copyright: copyright of the channel
 *
 * To set the copyright of the feed
 */
void
grss_feed_channel_set_copyright (GrssFeedChannel *channel, gchar *copyright)
{
	FREE_STRING (channel->priv->copyright);
	channel->priv->copyright = g_strdup (copyright);
}

/**
 * grss_feed_channel_get_copyright:
 * @channel: a #GrssFeedChannel
 *
 * Retrieves indications about the copyright
 *
 * Return value: copyright of the @channel, or NULL
 */
const gchar*
grss_feed_channel_get_copyright (GrssFeedChannel *channel)
{
	return (const gchar*) channel->priv->copyright;
}

/**
 * grss_feed_channel_set_editor:
 * @channel: a #GrssFeedChannel
 * @editor: editor of the feed
 *
 * To set the editor of the @channel
 */
void
grss_feed_channel_set_editor (GrssFeedChannel *channel, gchar *editor)
{
	FREE_STRING (channel->priv->editor);
	channel->priv->editor = g_strdup (editor);
}

/**
 * grss_feed_channel_get_editor:
 * @channel: a #GrssFeedChannel
 *
 * Retrieves reference to the editor or the @channel
 *
 * Return value: editor of the feed, or NULL
 */
const gchar*
grss_feed_channel_get_editor (GrssFeedChannel *channel)
{
	return (const gchar*) channel->priv->editor;
}

/**
 * grss_feed_channel_add_contributor:
 * @channel: a #GrssFeedChannel
 * @contributor: contributor of the feed
 *
 * To add a contributor to the @channel
 */
void
grss_feed_channel_add_contributor (GrssFeedChannel *channel, gchar *contributor)
{
	gchar *con;

	con = g_strdup (contributor);

	if (channel->priv->contributors == NULL)
		channel->priv->contributors = g_list_prepend (channel->priv->contributors, con);
	else
		channel->priv->contributors = g_list_append (channel->priv->contributors, con);
}

/**
 * grss_feed_channel_get_contributors:
 * @channel: a #GrssFeedChannel
 *
 * Retrieves reference to the contributors of the @channel
 *
 * Return value: list of contributors to the channel, or NULL
 */
const GList*
grss_feed_channel_get_contributors (GrssFeedChannel *channel)
{
	return (const GList*) channel->priv->contributors;
}

/**
 * grss_feed_channel_set_webmaster:
 * @channel: a #GrssFeedChannel
 * @webmaster: webmaster of the feed
 *
 * To assign a webmaster to the @channel
 */
void
grss_feed_channel_set_webmaster (GrssFeedChannel *channel, gchar *webmaster)
{
	FREE_STRING (channel->priv->webmaster);
	channel->priv->webmaster = g_strdup (webmaster);
}

/**
 * grss_feed_channel_get_webmaster:
 * @channel: a #GrssFeedChannel
 *
 * Retrieves reference to the webmaster of the feed
 *
 * Return value: webmaster of @channel, or NULL
 */
const gchar*
grss_feed_channel_get_webmaster (GrssFeedChannel *channel)
{
	return (const gchar*) channel->priv->webmaster;
}

/**
 * grss_feed_channel_set_generator:
 * @channel: a #GrssFeedChannel
 * @generator: software generator of the feed
 *
 * To set information about the software generator of @channel
 */
void
grss_feed_channel_set_generator (GrssFeedChannel *channel, gchar *generator)
{
	FREE_STRING (channel->priv->generator);
	channel->priv->generator = g_strdup (generator);
}

/**
 * grss_feed_channel_get_generator:
 * @channel: a #GrssFeedChannel
 *
 * Retrieves information about the feed's software generator
 *
 * Return value: generator of @channel, or NULL
 */
const gchar*
grss_feed_channel_get_generator (GrssFeedChannel *channel)
{
	return (const gchar*) channel->priv->generator;
}

/**
 * grss_feed_channel_set_publish_time:
 * @channel: a #GrssFeedChannel
 * @publish: timestamp of publishing
 *
 * To set the time of publishing for the feed
 */
void
grss_feed_channel_set_publish_time (GrssFeedChannel *channel, time_t publish)
{
	channel->priv->pub_time = publish;
}

/**
 * grss_feed_channel_get_publish_time:
 * @channel: a #GrssFeedChannel
 *
 * Retrieves the publishing time of @channel
 *
 * Return value: time of feed's publish
 */
time_t
grss_feed_channel_get_publish_time (GrssFeedChannel *channel)
{
	return channel->priv->pub_time;
}

/**
 * grss_feed_channel_set_update_time:
 * @channel: a #GrssFeedChannel
 * @update: update time of the feed
 *
 * To set the latest update time of @channel
 */
void
grss_feed_channel_set_update_time (GrssFeedChannel *channel, time_t update)
{
	channel->priv->update_time = update;
}

/**
 * grss_feed_channel_get_update_time:
 * @channel: a #GrssFeedChannel
 *
 * Retrieves the update time of @channel
 *
 * Return value: time of the feed's latest update. If this value was not set
 * (with grss_feed_channel_set_update_time()) returns
 * grss_feed_channel_get_publish_time()
 */
time_t
grss_feed_channel_get_update_time (GrssFeedChannel *channel)
{
	return channel->priv->update_time;
}

/**
 * grss_feed_channel_set_update_interval:
 * @channel: a #GrssFeedChannel
 * @minutes: update interval, in minutes
 *
 * To set the update interval for @channel
 */
void
grss_feed_channel_set_update_interval (GrssFeedChannel *channel, int minutes)
{
	channel->priv->update_interval = minutes;
}

/**
 * grss_feed_channel_get_update_interval:
 * @channel: a #GrssFeedChannel
 *
 * Retrieves the update interval for the feed. Pay attention to the fact the
 * value can be unset, and the function returns 0: in this case the caller
 * must manually set a default update interval with
 * grss_feed_channel_set_update_interval()
 *
 * Return value: update interval for the @channel, in minutes
 */
int
grss_feed_channel_get_update_interval (GrssFeedChannel *channel)
{
	return channel->priv->update_interval;
}

static gboolean
quick_and_dirty_parse (GrssFeedChannel *channel, SoupMessage *msg)
{
	GList *items;
	GList *iter;
	xmlDocPtr doc;
	GrssFeedParser *parser;

	/*
		TODO	This function is quite inefficent because parses all
			the feed with a GrssFeedParser and them waste obtained
			GrssFeedItems. Perhaps a more aimed function in
			GrssFeedParser would help...
	*/

	doc = content_to_xml (msg->response_body->data, msg->response_body->length);

	if (doc != NULL) {
		parser = grss_feed_parser_new ();
		items = grss_feed_parser_parse (parser, channel, doc, NULL);

		if (items != NULL) {
			for (iter = items; iter; iter = g_list_next (iter))
				g_object_unref (iter->data);
			g_list_free (items);
		}

		g_object_unref (parser);
		xmlFreeDoc (doc);
		return TRUE;
	}
	else {
		return FALSE;
	}
}

/**
 * grss_feed_channel_fetch:
 * @channel: a #GrssFeedChannel
 *
 * Utility to fetch and populate a #GrssFeedChannel for the first time, and init
 * all his internal values. Only the source URL has to be set in @channel
 * (with grss_feed_channel_set_source()). Be aware this function is sync, do not
 * returns until the feed isn't downloaded and parsed
 *
 * Return value: %TRUE if the feed is correctly fetched and parsed, %FALSE
 * otherwise
 */
gboolean
grss_feed_channel_fetch (GrssFeedChannel *channel)
{
	gboolean ret;
	guint status;
	SoupMessage *msg;
	SoupSession *session;

	session = soup_session_sync_new ();
	msg = soup_message_new ("GET", grss_feed_channel_get_source (channel));
	status = soup_session_send_message (session, msg);

	if (status >= 200 && status <= 299) {
		ret = quick_and_dirty_parse (channel, msg);
	}
	else {
		g_warning ("Unable to fetch feed from %s: %s", grss_feed_channel_get_source (channel), soup_status_get_phrase (status));
		ret = FALSE;
	}

	g_object_unref (session);
	g_object_unref (msg);
	return ret;
}

static void
feed_downloaded (SoupSession *session, SoupMessage *msg, gpointer user_data) {
	guint status;
	GSimpleAsyncResult *result;
	GrssFeedChannel *channel;

	result = user_data;
	channel = FEED_CHANNEL (g_async_result_get_source_object (G_ASYNC_RESULT (result)));
	g_object_get (msg, "status-code", &status, NULL);

	if (status >= 200 && status <= 299) {
		quick_and_dirty_parse (channel, msg);
	}
	else {
		g_simple_async_result_set_error (result, FEEDS_CHANNEL_ERROR, FEEDS_CHANNEL_FETCH_ERROR,
						 "Unable to download from %s", grss_feed_channel_get_source (channel));
	}

	g_simple_async_result_complete_in_idle (result);
	g_object_unref (result);
}

/**
 * grss_feed_channel_fetch_async:
 * @channel: a #GrssFeedChannel
 * @callback: function to invoke at the end of the download
 * @user_data: data passed to the callback
 *
 * Similar to grss_feed_channel_fetch(), but asyncronous
 */
void
grss_feed_channel_fetch_async (GrssFeedChannel *channel, GAsyncReadyCallback callback, gpointer user_data)
{
	GSimpleAsyncResult *result;
	SoupMessage *msg;
	SoupSession *session;

	result = g_simple_async_result_new (G_OBJECT (channel), callback, user_data, grss_feed_channel_fetch_async);

	session = soup_session_async_new ();
	msg = soup_message_new ("GET", grss_feed_channel_get_source (channel));
	soup_session_queue_message (session, msg, feed_downloaded, result);
}
