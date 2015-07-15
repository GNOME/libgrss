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

#include "utils.h"
#include "feed-channel.h"
#include "feed-parser.h"

#define FEED_CHANNEL_GET_PRIVATE(obj)	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), GRSS_FEED_CHANNEL_TYPE, GrssFeedChannelPrivate))
#define FEED_CHANNEL_ERROR		feed_channel_error_quark()

/**
 * SECTION: feed-channel
 * @short_description: a feed
 *
 * #GrssFeedChannel rappresents a single feed which may be fetched and parsed.
 */

typedef struct {
	gchar	*hub;
} PubSub;

typedef struct {
	gchar	*path;
	gchar	*protocol;
} RSSCloud;

struct _GrssFeedChannelPrivate {
	gchar		*format;
	gchar		*source;

	gchar		*title;
	gchar		*homepage;
	gchar		*description;
	gchar		*image;
	gchar		*icon;
	gchar		*language;
	gchar		*category;
	PubSub		pubsub;
	RSSCloud	rsscloud;

	gchar		*copyright;
	gchar		*editor;
	GList		*contributors;
	SoupCookieJar   *jar;
	gchar		*webmaster;
	gchar		*generator;
	gboolean	gzip;

	time_t		pub_time;
	time_t		update_time;
	int		update_interval;

	GCancellable	*fetchcancel;
};

enum {
	FEED_CHANNEL_FETCH_ERROR,
	FEED_CHANNEL_PARSE_ERROR,
	FEED_CHANNEL_FILE_ERROR,
};

G_DEFINE_TYPE (GrssFeedChannel, grss_feed_channel, G_TYPE_OBJECT);

static GQuark
feed_channel_error_quark ()
{
	return g_quark_from_static_string ("feed_channel_error");
}

static void
grss_feed_channel_finalize (GObject *obj)
{
	GList *iter;
	GrssFeedChannel *chan;

	chan = GRSS_FEED_CHANNEL (obj);
	FREE_STRING (chan->priv->title);
	FREE_STRING (chan->priv->homepage);
	FREE_STRING (chan->priv->description);
	FREE_STRING (chan->priv->image);
	FREE_STRING (chan->priv->icon);
	FREE_STRING (chan->priv->language);
	FREE_STRING (chan->priv->category);
	FREE_STRING (chan->priv->pubsub.hub);
	FREE_STRING (chan->priv->rsscloud.path);
	FREE_STRING (chan->priv->rsscloud.protocol);
	FREE_STRING (chan->priv->copyright);
	FREE_STRING (chan->priv->editor);
	FREE_STRING (chan->priv->webmaster);
	FREE_STRING (chan->priv->generator);

	if (chan->priv->contributors != NULL) {
		for (iter = chan->priv->contributors; iter; iter = g_list_next (iter))
			g_free (iter->data);
		g_list_free (chan->priv->contributors);
	}

	if (chan->priv->jar != NULL)
		g_free (chan->priv->jar);
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
 * Allocates a new #GrssFeedChannel.
 *
 * Returns: a #GrssFeedChannel.
 */
GrssFeedChannel*
grss_feed_channel_new ()
{
	return g_object_new (GRSS_FEED_CHANNEL_TYPE, NULL);
}

/**
 * grss_feed_channel_new_with_source:
 * @source: URL of the feed.
 *
 * Allocates a new #GrssFeedChannel and assign it the given remote source.
 *
 * Returns: a #GrssFeedChannel.
 */
GrssFeedChannel*
grss_feed_channel_new_with_source (gchar *source)
{
	GrssFeedChannel *ret;

	ret = grss_feed_channel_new ();
	grss_feed_channel_set_source (ret, source);
	return ret;
}

/**
 * grss_feed_channel_new_from_xml:
 * @doc: an XML document previously parsed with libxml2.
 * @error: if an error occurred, %NULL is returned and this is filled with the
 *         message.
 *
 * Allocates a new #GrssFeedChannel and init it with contents found in specified
 * XML document.
 *
 * Returns: a #GrssFeedChannel, or %NULL if an error occurs.
 */
GrssFeedChannel*
grss_feed_channel_new_from_xml (xmlDocPtr doc, GError **error)
{
	GrssFeedParser *parser;
	GrssFeedChannel *ret;
	GError *myerror;

	ret = g_object_new (GRSS_FEED_CHANNEL_TYPE, NULL);
	parser = grss_feed_parser_new ();

	myerror = NULL;
	grss_feed_parser_parse_channel (parser, ret, doc, &myerror);
	if (myerror != NULL) {
		g_propagate_error (error, myerror);
		g_object_unref (ret);
		ret = NULL;
	}

	g_object_unref (parser);
	return ret;
}

/**
 * grss_feed_channel_new_from_memory:
 * @data: string to parse.
 * @error: if an error occurred, %NULL is returned and this is filled with the
 *         message.
 *
 * Allocates a new #GrssFeedChannel and init it with contents found in specified
 * memory block.
 *
 * Returns: a #GrssFeedChannel, or %NULL if an error occurs.
 */
GrssFeedChannel*
grss_feed_channel_new_from_memory (const gchar *data, GError **error)
{
	xmlDocPtr doc;

	doc = content_to_xml (data, strlen (data));
	if (doc == NULL) {
		g_set_error (error, FEED_CHANNEL_ERROR, FEED_CHANNEL_PARSE_ERROR, "Unable to parse data");
		return NULL;
	}

	return grss_feed_channel_new_from_xml (doc, error);
}

/**
 * grss_feed_channel_new_from_file:
 * @path: path of the file to parse.
 * @error: if an error occurred, %NULL is returned and this is filled with the
 *         message.
 *
 * Allocates a new #GrssFeedChannel and init it with contents found in specified
 * file.
 *
 * Returns: a #GrssFeedChannel, or %NULL if the file in @path is not a
 * valid document.
 */
GrssFeedChannel*
grss_feed_channel_new_from_file (const gchar *path, GError **error)
{
	struct stat sbuf;
	xmlDocPtr doc;
	GrssFeedChannel *ret;

	ret = NULL;

	if (stat (path, &sbuf) == -1) {
		g_set_error (error, FEED_CHANNEL_ERROR, FEED_CHANNEL_FILE_ERROR, "Unable to open file: %s", strerror (errno));
		return NULL;
	}

	doc = file_to_xml (path);
	if (doc == NULL) {
		g_set_error (error, FEED_CHANNEL_ERROR, FEED_CHANNEL_PARSE_ERROR, "Unable to parse file");
		return NULL;
	}

	ret = grss_feed_channel_new_from_xml (doc, error);

	xmlFreeDoc (doc);
	return ret;
}

/**
 * grss_feed_channel_set_format:
 * @channel: a #GrssFeedChannel.
 * @format: format of the file, such as "application/atom+xml" or
 * "application/rss+xml".
 *
 * To assign a file format to the feed.
 */
void
grss_feed_channel_set_format (GrssFeedChannel *channel, gchar *format)
{
	FREE_STRING (channel->priv->format);
	channel->priv->format = g_strdup (format);
}

/**
 * grss_feed_channel_get_format:
 * @channel: a #GrssFeedChannel.
 *
 * Retrieves the file format of @channel.
 *
 * Returns: file format of channel.
 */
const gchar*
grss_feed_channel_get_format (GrssFeedChannel *channel)
{
	return (const gchar*) channel->priv->format;
}

/**
 * grss_feed_channel_set_source:
 * @channel: a #GrssFeedChannel.
 * @source: URL of the feed.
 *
 * To assign the URL where to fetch the feed.
 * 
 * Returns: %TRUE if @source is a valid URL, %FALSE otherwise
 */
gboolean
grss_feed_channel_set_source (GrssFeedChannel *channel, gchar *source)
{
	FREE_STRING (channel->priv->source);

	if (test_url ((const gchar*) source) == TRUE) {
		channel->priv->source = SET_STRING (source);
		return TRUE;
	}
	else {
		return FALSE;
	}
}

/**
 * grss_feed_channel_get_source:
 * @channel: a #GrssFeedChannel.
 *
 * Retrieves URL where to fetch the @channel.
 *
 * Returns: URL of the channel.
 */
const gchar*
grss_feed_channel_get_source (GrssFeedChannel *channel)
{
	return (const gchar*) channel->priv->source;
}

/**
 * grss_feed_channel_set_title:
 * @channel: a #GrssFeedChannel.
 * @title: title of the feed.
 *
 * To set a title to the @channel.
 */
void
grss_feed_channel_set_title (GrssFeedChannel *channel, gchar *title)
{
	FREE_STRING (channel->priv->title);
	channel->priv->title = g_strdup (title);
}

/**
 * grss_feed_channel_get_title:
 * @channel: a #GrssFeedChannel.
 *
 * Retrieves title of the @channel.
 *
 * Returns: title of the feed, or %NULL.
 */
const gchar*
grss_feed_channel_get_title (GrssFeedChannel *channel)
{
	return (const gchar*) channel->priv->title;
}

/**
 * grss_feed_channel_set_homepage:
 * @channel: a #GrssFeedChannel.
 * @homepage: homepage for the main website.
 *
 * To set the homepage of the site the @channel belongs.
 * 
 * Returns: %TRUE if @homepage is a valid URL, %FALSE otherwise
 */
gboolean
grss_feed_channel_set_homepage (GrssFeedChannel *channel, gchar *homepage)
{
	FREE_STRING (channel->priv->homepage);

	if (test_url ((const gchar*) homepage) == TRUE) {
		channel->priv->homepage = SET_STRING (homepage);
		return TRUE;
	}
	else {
		return FALSE;
	}
}

/**
 * grss_feed_channel_get_homepage:
 * @channel: a #GrssFeedChannel.
 *
 * Retrieves the homepage of the site for which @channel is the feed.
 *
 * Returns: reference homepage of the feed, or %NULL.
 */
const gchar*
grss_feed_channel_get_homepage (GrssFeedChannel *channel)
{
	return (const gchar*) channel->priv->homepage;
}

/**
 * grss_feed_channel_set_description:
 * @channel: a #GrssFeedChannel.
 * @description: description of the feed.
 *
 * To set the description of @channel.
 */
void
grss_feed_channel_set_description (GrssFeedChannel *channel, gchar *description)
{
	FREE_STRING (channel->priv->description);
	channel->priv->description = g_strdup (description);
}

/**
 * grss_feed_channel_get_description:
 * @channel: a #GrssFeedChannel.
 *
 * Retrieves the description of @channel.
 *
 * Returns: description of the feed, or %NULL.
 */
const gchar*
grss_feed_channel_get_description (GrssFeedChannel *channel)
{
	return (const gchar*) channel->priv->description;
}

/**
 * grss_feed_channel_set_image:
 * @channel: a #GrssFeedChannel.
 * @image: URL of the image.
 *
 * To set a rappresentative image to @channel.
 * 
 * Returns: %TRUE if @image is a valid URL, %FALSE otherwise
 */
gboolean
grss_feed_channel_set_image (GrssFeedChannel *channel, gchar *image)
{
	FREE_STRING (channel->priv->image);

	if (test_url ((const gchar*) image) == TRUE) {
		channel->priv->image = SET_STRING (image);
		return TRUE;
	}
	else {
		return FALSE;
	}
}

/**
 * grss_feed_channel_get_image:
 * @channel: a #GrssFeedChannel.
 *
 * Retrieves the URL of the image assigned to the channel.
 *
 * Returns: URL of the image, or %NULL.
 */
const gchar*
grss_feed_channel_get_image (GrssFeedChannel *channel)
{
	return (const gchar*) channel->priv->image;
}

/**
 * grss_feed_channel_set_icon:
 * @channel: a #GrssFeedChannel.
 * @icon: URL where to retrieve the favicon.
 *
 * To set the URL of the icon rappresenting @channel.
 * 
 * Returns: %TRUE if @icon is a valid URL, %FALSE otherwise
 */
gboolean
grss_feed_channel_set_icon (GrssFeedChannel *channel, gchar *icon)
{
	FREE_STRING (channel->priv->icon);

	if (test_url ((const gchar*) icon) == TRUE) {
		channel->priv->icon = SET_STRING (icon);
		return TRUE;
	}
	else {
		return FALSE;
	}
}

/**
 * grss_feed_channel_get_icon:
 * @channel: a #GrssFeedChannel.
 *
 * Retrieves URL of the favicon of the channel (and/or the website for which
 * this is the feed).
 *
 * Returns: URL of the favicon, or %NULL.
 */
const gchar*
grss_feed_channel_get_icon (GrssFeedChannel *channel)
{
	return (const gchar*) channel->priv->icon;
}

/**
 * grss_feed_channel_set_language:
 * @channel: a #GrssFeedChannel.
 * @language: string holding the language of the feed.
 *
 * To set the language of @channel.
 */
void
grss_feed_channel_set_language (GrssFeedChannel *channel, gchar *language)
{
	FREE_STRING (channel->priv->language);
	channel->priv->language = g_strdup (language);
}

/**
 * grss_feed_channel_get_language:
 * @channel: a #GrssFeedChannel.
 *
 * Retrieves the language of the @channel.
 *
 * Returns: string rappresenting the language of channel, or %NULL.
 */
const gchar*
grss_feed_channel_get_language (GrssFeedChannel *channel)
{
	return (const gchar*) channel->priv->language;
}

/**
 * grss_feed_channel_set_category:
 * @channel: a #GrssFeedChannel.
 * @category: category of the feed.
 *
 * To set the category of the @channel.
 */
void
grss_feed_channel_set_category (GrssFeedChannel *channel, gchar *category)
{
	FREE_STRING (channel->priv->category);
	channel->priv->category = g_strdup (category);
}

/**
 * grss_feed_channel_get_category:
 * @channel: a #GrssFeedChannel.
 *
 * Retrieves category of the @channel.
 *
 * Returns: category of the feed, or %NULL.
 */
const gchar*
grss_feed_channel_get_category (GrssFeedChannel *channel)
{
	return (const gchar*) channel->priv->category;
}

/**
 * grss_feed_channel_set_pubsubhub:
 * @channel: a #GrssFeedChannel.
 * @hub: hub for the feed, or %NULL.
 *
 * To set information about PubSubHubbub for the channel. To unset the hub,
 * pass %NULL as parameter.
 * 
 * Returns: %TRUE if @hub is a valid URL, %FALSE otherwise
 */
gboolean
grss_feed_channel_set_pubsubhub (GrssFeedChannel *channel, gchar *hub)
{
	FREE_STRING (channel->priv->pubsub.hub);

	if (test_url ((const gchar*) hub) == TRUE) {
		channel->priv->pubsub.hub = SET_STRING (hub);
		return TRUE;
	}
	else {
		return FALSE;
	}
}

/**
 * grss_feed_channel_get_pubsubhub:
 * @channel: a #GrssFeedChannel.
 * @hub: location for the hub string, or %NULL.
 *
 * Retrieves information about the PubSubHubbub hub of the channel.
 *
 * Returns: %TRUE if a valid PubSubHubbub hub has been set for the
 * @channel, %FALSE otherwise.
 */
gboolean
grss_feed_channel_get_pubsubhub (GrssFeedChannel *channel, gchar **hub)
{
	if (hub != NULL)
		*hub = channel->priv->pubsub.hub;

	return (channel->priv->pubsub.hub != NULL);
}

/**
 * grss_feed_channel_set_rsscloud:
 * @channel: a #GrssFeedChannel.
 * @path: complete references of the URL where to register subscription, e.g.
 *        http://example.com/rsscloudNotify .
 * @protocol: type of protocol used for notifications.
 *
 * To set information about RSSCloud notifications for the channel.
 */
void
grss_feed_channel_set_rsscloud (GrssFeedChannel *channel, gchar *path, gchar *protocol)
{
	FREE_STRING (channel->priv->rsscloud.path);
	FREE_STRING (channel->priv->rsscloud.protocol);

	if (path != NULL && protocol != NULL) {
		channel->priv->rsscloud.path = g_strdup (path);
		channel->priv->rsscloud.protocol = g_strdup (protocol);
	}
}

/**
 * grss_feed_channel_get_rsscloud:
 * @channel: a #GrssFeedChannel.
 * @path: location for the path string, or %NULL.
 * @protocol: location for the protocol string, or %NULL.
 *
 * Retrieves information about the RSSCloud coordinates of the channel.
 *
 * Returns: %TRUE if a valid RSSCloud path has been set for the
 * @channel, %FALSE otherwise.
 */
gboolean
grss_feed_channel_get_rsscloud (GrssFeedChannel *channel, gchar **path, gchar **protocol)
{
	if (path != NULL)
		*path = channel->priv->rsscloud.path;
	if (protocol != NULL)
		*protocol = channel->priv->rsscloud.protocol;

	return (channel->priv->rsscloud.path != NULL && channel->priv->rsscloud.protocol != NULL);
}

/**
 * grss_feed_channel_set_copyright:
 * @channel: a #GrssFeedChannel.
 * @copyright: copyright of the channel.
 *
 * To set the copyright of the feed.
 */
void
grss_feed_channel_set_copyright (GrssFeedChannel *channel, gchar *copyright)
{
	FREE_STRING (channel->priv->copyright);
	channel->priv->copyright = g_strdup (copyright);
}

/**
 * grss_feed_channel_get_copyright:
 * @channel: a #GrssFeedChannel.
 *
 * Retrieves indications about the copyright.
 *
 * Returns: copyright of the @channel, or %NULL.
 */
const gchar*
grss_feed_channel_get_copyright (GrssFeedChannel *channel)
{
	return (const gchar*) channel->priv->copyright;
}

/**
 * grss_feed_channel_set_editor:
 * @channel: a #GrssFeedChannel.
 * @editor: editor of the feed.
 *
 * To set the editor of the @channel.
 */
void
grss_feed_channel_set_editor (GrssFeedChannel *channel, gchar *editor)
{
	FREE_STRING (channel->priv->editor);
	channel->priv->editor = g_strdup (editor);
}

/**
 * grss_feed_channel_get_editor:
 * @channel: a #GrssFeedChannel.
 *
 * Retrieves reference to the editor or the @channel.
 *
 * Returns: editor of the feed, or %NULL.
 */
const gchar*
grss_feed_channel_get_editor (GrssFeedChannel *channel)
{
	return (const gchar*) channel->priv->editor;
}

/**
 * grss_feed_channel_add_contributor:
 * @channel: a #GrssFeedChannel.
 * @contributor: contributor of the feed.
 *
 * To add a contributor to the @channel.
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
 * @channel: a #GrssFeedChannel.
 *
 * Retrieves reference to the contributors of the @channel.
 *
 * Returns: (element-type utf8) (transfer none): list of contributors to
 * the channel, or %NULL.
 */
const GList*
grss_feed_channel_get_contributors (GrssFeedChannel *channel)
{
	return (const GList*) channel->priv->contributors;
}

/**
 * grss_feed_channel_add_cookie:
 * @channel: a #GrssFeedChannel.
 * @cookie: HTML cookie to add to the #GrssFeedChannel's cookie jar
 * 
 * To add a cookie related to the @channel, will be involved in HTTP sessions
 * while fetching it. More cookie can be added to every #GrssFeedChannel
 */
void
grss_feed_channel_add_cookie (GrssFeedChannel *channel, SoupCookie *cookie)
{
	if (cookie != NULL) {
		if (channel->priv->jar == NULL)
			channel->priv->jar = soup_cookie_jar_new ();
		soup_cookie_jar_add_cookie (channel->priv->jar, cookie);
	}
}

/**
 * grss_feed_channel_get_cookies:
 * @channel: a #GrssFeedChannel.
 *
 * Retrieves reference to the HTML cookies of the @channel.
 * The list and the individual cookies should all be freed after use.
 * You can use soup_cookies_free.
 *
 * Returns: (element-type SoupCookie) (transfer full): list of cookies to
 * the channel, or %NULL.
 */
GSList*
grss_feed_channel_get_cookies (GrssFeedChannel *channel)
{
	if (channel->priv->jar != NULL)
		return soup_cookie_jar_all_cookies(channel->priv->jar);

	return NULL;
}

/**
 * grss_feed_channel_set_webmaster:
 * @channel: a #GrssFeedChannel.
 * @webmaster: webmaster of the feed.
 *
 * To assign a webmaster to the @channel.
 */
void
grss_feed_channel_set_webmaster (GrssFeedChannel *channel, gchar *webmaster)
{
	FREE_STRING (channel->priv->webmaster);
	channel->priv->webmaster = g_strdup (webmaster);
}

/**
 * grss_feed_channel_get_webmaster:
 * @channel: a #GrssFeedChannel.
 *
 * Retrieves reference to the webmaster of the feed.
 *
 * Returns: webmaster of @channel, or %NULL.
 */
const gchar*
grss_feed_channel_get_webmaster (GrssFeedChannel *channel)
{
	return (const gchar*) channel->priv->webmaster;
}

/**
 * grss_feed_channel_set_generator:
 * @channel: a #GrssFeedChannel.
 * @generator: software generator of the feed.
 *
 * To set information about the software generator of @channel.
 */
void
grss_feed_channel_set_generator (GrssFeedChannel *channel, gchar *generator)
{
	FREE_STRING (channel->priv->generator);
	channel->priv->generator = g_strdup (generator);
}

/**
 * grss_feed_channel_get_generator:
 * @channel: a #GrssFeedChannel.
 *
 * Retrieves information about the feed's software generator.
 *
 * Returns: generator of @channel, or %NULL.
 */
const gchar*
grss_feed_channel_get_generator (GrssFeedChannel *channel)
{
	return (const gchar*) channel->priv->generator;
}

/**
 * grss_feed_channel_set_gzip_compression:
 * @channel: a #GrssFeedChannel.
 * @value: %TRUE to enable GZIP compression when fetching the channel
 * 
 * Set the GZIP compression for the channel to on or off.
 */
void
grss_feed_channel_set_gzip_compression(GrssFeedChannel *channel, gboolean value)
{
	channel->priv->gzip = value;
}

/**
 * grss_feed_channel_get_gzip_compression:
 * @channel: a #GrssFeedChannel.
 *
 * GZIP compression of the channel is either on or off.
 *
 * Returns: %TRUE if @channel has GZIP compression on.
 */
gboolean
grss_feed_channel_get_gzip_compression (GrssFeedChannel *channel)
{
	return channel->priv->gzip;
}

/**
 * grss_feed_channel_set_publish_time:
 * @channel: a #GrssFeedChannel.
 * @publish: timestamp of publishing.
 *
 * To set the time of publishing for the feed.
 */
void
grss_feed_channel_set_publish_time (GrssFeedChannel *channel, time_t publish)
{
	channel->priv->pub_time = publish;
}

/**
 * grss_feed_channel_get_publish_time:
 * @channel: a #GrssFeedChannel.
 *
 * Retrieves the publishing time of @channel.
 *
 * Returns: time of feed's publish.
 */
time_t
grss_feed_channel_get_publish_time (GrssFeedChannel *channel)
{
	return channel->priv->pub_time;
}

/**
 * grss_feed_channel_set_update_time:
 * @channel: a #GrssFeedChannel.
 * @update: update time of the feed.
 *
 * To set the latest update time of @channel.
 */
void
grss_feed_channel_set_update_time (GrssFeedChannel *channel, time_t update)
{
	channel->priv->update_time = update;
}

/**
 * grss_feed_channel_get_update_time:
 * @channel: a #GrssFeedChannel.
 *
 * Retrieves the update time of @channel.
 *
 * Returns: time of the feed's latest update. If this value was not set
 * (with grss_feed_channel_set_update_time()) returns
 * grss_feed_channel_get_publish_time().
 */
time_t
grss_feed_channel_get_update_time (GrssFeedChannel *channel)
{
	return channel->priv->update_time;
}

/**
 * grss_feed_channel_set_update_interval:
 * @channel: a #GrssFeedChannel.
 * @minutes: update interval, in minutes.
 *
 * To set the update interval for @channel.
 */
void
grss_feed_channel_set_update_interval (GrssFeedChannel *channel, int minutes)
{
	channel->priv->update_interval = minutes;
}

/**
 * grss_feed_channel_get_update_interval:
 * @channel: a #GrssFeedChannel.
 *
 * Retrieves the update interval for the feed. Pay attention to the fact the
 * value can be unset, and the function returns 0: in this case the caller
 * must manually set a default update interval with
 * grss_feed_channel_set_update_interval().
 *
 * Returns: update interval for the @channel, in minutes.
 */
int
grss_feed_channel_get_update_interval (GrssFeedChannel *channel)
{
	return channel->priv->update_interval;
}

static gboolean
quick_and_dirty_parse (GrssFeedChannel *channel, SoupMessage *msg, GList **save_items)
{
	GList *items;
	GList *iter;
	xmlDocPtr doc;
	GrssFeedParser *parser;

	doc = content_to_xml (msg->response_body->data, msg->response_body->length);

	if (doc != NULL) {
		parser = grss_feed_parser_new ();

		if (save_items == NULL) {
			grss_feed_parser_parse_channel (parser, channel, doc, NULL);
		}
		else {
			items = grss_feed_parser_parse (parser, channel, doc, NULL);
			*save_items = items;
		}

		g_object_unref (parser);
		xmlFreeDoc (doc);
		return TRUE;
	}
	else {
		return FALSE;
	}
}

static void
init_soup_session (SoupSession *session, GrssFeedChannel *channel)
{
	if (channel->priv->jar != NULL)
		soup_session_add_feature (session, SOUP_SESSION_FEATURE (channel->priv->jar));
	if (channel->priv->gzip == TRUE)
		soup_session_add_feature_by_type (session, SOUP_TYPE_CONTENT_DECODER);
}

static void
init_soup_message (SoupMessage* msg, GrssFeedChannel *channel)
{
	if (channel->priv->gzip == TRUE)
		soup_message_headers_append (msg->request_headers, "Accept-encoding", "gzip");
}

/**
 * grss_feed_channel_fetch:
 * @channel: a #GrssFeedChannel.
 * @error: if an error occurred, %FALSE is returned and this is filled with the
 *         message.
 *
 * Utility to fetch and populate a #GrssFeedChannel for the first time, and init
 * all his internal values. Only the source URL has to be set in @channel
 * (with grss_feed_channel_set_source()). Be aware this function is sync, do not
 * returns until the feed isn't downloaded and parsed.
 *
 * Returns: %TRUE if the feed is correctly fetched and parsed, %FALSE
 * otherwise.
 */
gboolean
grss_feed_channel_fetch (GrssFeedChannel *channel, GError **error)
{
	gboolean ret;
	guint status;
	SoupMessage *msg;
	SoupSession *session;

	ret = FALSE;

	session = soup_session_sync_new ();
	init_soup_session (session, channel);

	msg = soup_message_new ("GET", grss_feed_channel_get_source (channel));
	init_soup_message (msg, channel);

	status = soup_session_send_message (session, msg);

	if (status >= 200 && status <= 299) {
		ret = quick_and_dirty_parse (channel, msg, NULL);
		if (ret == FALSE)
			g_set_error (error, FEED_CHANNEL_ERROR, FEED_CHANNEL_PARSE_ERROR, "Unable to parse file");
	}
	else {
		g_set_error (error, FEED_CHANNEL_ERROR, FEED_CHANNEL_FETCH_ERROR,
		             "Unable to download from %s", grss_feed_channel_get_source (channel));
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
	channel = GRSS_FEED_CHANNEL (g_async_result_get_source_object (G_ASYNC_RESULT (result)));
	g_object_get (msg, "status-code", &status, NULL);

	if (status >= 200 && status <= 299) {
		if (quick_and_dirty_parse (channel, msg, NULL) == FALSE)
			g_simple_async_result_set_error (result, FEED_CHANNEL_ERROR, FEED_CHANNEL_PARSE_ERROR,
						 "Unable to parse feed from %s", grss_feed_channel_get_source (channel));
	}
	else {
		g_simple_async_result_set_error (result, FEED_CHANNEL_ERROR, FEED_CHANNEL_FETCH_ERROR,
						 "Unable to download from %s", grss_feed_channel_get_source (channel));
	}

	g_simple_async_result_complete_in_idle (result);
	g_object_unref (channel->priv->fetchcancel);
	g_object_unref (result);
}

/**
 * grss_feed_channel_fetch_finish:
 * @channel: a #GrssFeedChannel.
 * @res: the #GAsyncResult passed to the callback.
 * @error: if an error occurred, %FALSE is returned and this is filled with the
 *         message.
 *
 * Finalizes an asyncronous operation started with
 * grss_feed_channel_fetch_async().
 *
 * Returns: %TRUE if @channel informations have been successfully fetched,
 * %FALSE otherwise.
 */
gboolean
grss_feed_channel_fetch_finish (GrssFeedChannel *channel, GAsyncResult *res, GError **error)
{
	if (g_simple_async_result_propagate_error (G_SIMPLE_ASYNC_RESULT (res), error))
		return FALSE;
	else
		return TRUE;
}

static void
do_prefetch (GrssFeedChannel *channel)
{
	grss_feed_channel_fetch_cancel (channel);
	channel->priv->fetchcancel = g_cancellable_new ();
}

/**
 * grss_feed_channel_fetch_async:
 * @channel: a #GrssFeedChannel.
 * @callback: function to invoke at the end of the download.
 * @user_data: data passed to the callback.
 *
 * Similar to grss_feed_channel_fetch(), but asyncronous.
 */
void
grss_feed_channel_fetch_async (GrssFeedChannel *channel, GAsyncReadyCallback callback, gpointer user_data)
{
	GSimpleAsyncResult *result;
	SoupMessage *msg;
	SoupSession *session;

	/*
		TODO: if the source is not valid, call anyway the callback with an error
	*/

	do_prefetch (channel);
	result = g_simple_async_result_new (G_OBJECT (channel), callback, user_data, grss_feed_channel_fetch_async);
	g_simple_async_result_set_check_cancellable (result, channel->priv->fetchcancel);

	session = soup_session_async_new ();
	init_soup_session (session, channel);

	msg = soup_message_new ("GET", grss_feed_channel_get_source (channel));
	init_soup_message (msg, channel);

	soup_session_queue_message (session, msg, feed_downloaded, result);
}

/**
 * grss_feed_channel_fetch_all:
 * @channel: a #GrssFeedChannel.
 * @error: if an error occurred, %NULL is returned and this is filled with the
 *         message.
 *
 * Utility to fetch and populate a #GrssFeedChannel, and retrieve all its
 * items.
 *
 * Returns: (element-type GrssFeedItem) (transfer full): a GList
 * of #GrssFeedItem, to be completely unreferenced and freed when no
 * longer in use, or %NULL if an error occurs.
 */
GList*
grss_feed_channel_fetch_all (GrssFeedChannel *channel, GError **error)
{
	guint status;
	GList *items;
	SoupMessage *msg;
	SoupSession *session;

	session = soup_session_sync_new ();
	init_soup_session (session, channel);

	msg = soup_message_new ("GET", grss_feed_channel_get_source (channel));
	init_soup_message (msg, channel);

	status = soup_session_send_message (session, msg);
	items = NULL;

	if (status >= 200 && status <= 299) {
		if (quick_and_dirty_parse (channel, msg, &items) == FALSE)
			g_set_error (error, FEED_CHANNEL_ERROR, FEED_CHANNEL_PARSE_ERROR, "Unable to parse file");
	}
	else {
		g_set_error (error, FEED_CHANNEL_ERROR, FEED_CHANNEL_FETCH_ERROR,
		             "Unable to download from %s", grss_feed_channel_get_source (channel));
	}

	g_object_unref (session);
	g_object_unref (msg);
	return items;
}

static void
free_items_list (gpointer list)
{
	GList *items;
	GList *iter;

	items = list;

	for (iter = items; iter; iter = g_list_next (iter))
		g_object_unref (iter->data);

	g_list_free (items);
}

static void
feed_downloaded_return_items (SoupSession *session, SoupMessage *msg, gpointer user_data)
{
	guint status;
	GList *items;
	GSimpleAsyncResult *result;
	GrssFeedChannel *channel;

	result = user_data;
	channel = GRSS_FEED_CHANNEL (g_async_result_get_source_object (G_ASYNC_RESULT (result)));
	g_object_get (msg, "status-code", &status, NULL);

	if (status >= 200 && status <= 299) {
		items = NULL;

		if (quick_and_dirty_parse (channel, msg, &items) == TRUE)
			g_simple_async_result_set_op_res_gpointer (result, items, free_items_list);
		else
			g_simple_async_result_set_error (result, FEED_CHANNEL_ERROR, FEED_CHANNEL_PARSE_ERROR,
						 "Unable to parse feed from %s", grss_feed_channel_get_source (channel));
	}
	else {
		g_simple_async_result_set_error (result, FEED_CHANNEL_ERROR, FEED_CHANNEL_FETCH_ERROR,
						 "Unable to download from %s", grss_feed_channel_get_source (channel));
	}

	g_simple_async_result_complete_in_idle (result);
	g_object_unref (channel->priv->fetchcancel);
	g_object_unref (result);
}

/**
 * grss_feed_channel_fetch_all_async:
 * @channel: a #GrssFeedChannel.
 * @callback: function to invoke at the end of the download.
 * @user_data: data passed to the callback.
 *
 * Similar to grss_feed_channel_fetch_all(), but asyncronous.
 */
void
grss_feed_channel_fetch_all_async (GrssFeedChannel *channel, GAsyncReadyCallback callback, gpointer user_data)
{
	GSimpleAsyncResult *result;
	SoupMessage *msg;
	SoupSession *session;

	do_prefetch (channel);
	result = g_simple_async_result_new (G_OBJECT (channel), callback, user_data, grss_feed_channel_fetch_async);
	g_simple_async_result_set_check_cancellable (result, channel->priv->fetchcancel);

	session = soup_session_async_new ();
	init_soup_session (session, channel);

	msg = soup_message_new ("GET", grss_feed_channel_get_source (channel));
	init_soup_message (msg, channel);

	soup_session_queue_message (session, msg, feed_downloaded_return_items, result);
}

/**
 * grss_feed_channel_fetch_all_finish:
 * @channel: a #GrssFeedChannel.
 * @res: the #GAsyncResult passed to the callback.
 * @error: if an error occurred, %NULL is returned and this is filled with the
 *         message.
 *
 * Finalizes an asyncronous operation started with
 * grss_feed_channel_fetch_all_async().
 *
 * Returns: (element-type GrssFeedItem) (transfer none): list of
 * items fetched from the #GrssFeedChannel, or %NULL if @error is
 * set. The list (and contained items) is freed at the end of the
 * callback
 */
GList*
grss_feed_channel_fetch_all_finish (GrssFeedChannel *channel, GAsyncResult *res, GError **error)
{
	if (g_simple_async_result_propagate_error (G_SIMPLE_ASYNC_RESULT (res), error))
		return NULL;
	else
		return (GList*) g_simple_async_result_get_op_res_gpointer (G_SIMPLE_ASYNC_RESULT (res));
}

/**
 * grss_feed_channel_fetch_cancel:
 * @channel: a #GrssFeedChannel.
 *
 * If a fetch operation was scheduled with grss_feed_channel_fetch_async() or
 * grss_feed_channel_fetch_all_async(), cancel it.
 * 
 * Returns: %TRUE if a fetch was scheduled (and now cancelled), %FALSE if
 * this function had nothing to do
 */
gboolean
grss_feed_channel_fetch_cancel (GrssFeedChannel *channel)
{
	if (channel->priv->fetchcancel != NULL) {
		g_cancellable_cancel (channel->priv->fetchcancel);
		g_object_unref (channel->priv->fetchcancel);
		return TRUE;
	}
	else {
		return FALSE;
	}
}

