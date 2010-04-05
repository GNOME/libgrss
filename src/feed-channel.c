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

#define FEED_CHANNEL_GET_PRIVATE(obj)     (G_TYPE_INSTANCE_GET_PRIVATE ((obj), FEED_CHANNEL_TYPE, FeedChannelPrivate))

/**
 * SECTION: feed-channel
 * @short_description: a feed
 *
 * #FeedChannel rappresents a single feed which may be fetched and parsed
 */

typedef struct {
	gchar	*hub;
	gchar	*self;
} PubSub;

struct _FeedChannelPrivate {
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

G_DEFINE_TYPE (FeedChannel, feed_channel, G_TYPE_OBJECT);

static void
feed_channel_finalize (GObject *obj)
{
	GList *iter;
	FeedChannel *chan;

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
feed_channel_class_init (FeedChannelClass *klass)
{
	GObjectClass *gobject_class;

	g_type_class_add_private (klass, sizeof (FeedChannelPrivate));

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->finalize = feed_channel_finalize;
}

static void
feed_channel_init (FeedChannel *node)
{
	node->priv = FEED_CHANNEL_GET_PRIVATE (node);
	memset (node->priv, 0, sizeof (FeedChannelPrivate));
}

/**
 * feed_channel_new:
 *
 * Allocates a new #FeedChannel
 *
 * Return value: a #FeedChannel
 */
FeedChannel*
feed_channel_new ()
{
	return g_object_new (FEED_CHANNEL_TYPE, NULL);
}

/**
 * feed_channel_set_source:
 * @channel: a #FeedChannel
 * @source: URL of the feed
 *
 * To assign the URL where to fetch the feed
 */
void
feed_channel_set_source (FeedChannel *channel, gchar *source)
{
	FREE_STRING (channel->priv->source);
	channel->priv->source = g_strdup (source);
}

/**
 * feed_channel_get_source:
 * @channel: a #FeedChannel
 *
 * Retrieves URL where to fetch the @channel
 *
 * Return value: URL of the channel
 */
const gchar*
feed_channel_get_source (FeedChannel *channel)
{
	return (const gchar*) channel->priv->source;
}

/**
 * feed_channel_set_title:
 * @channel: a #FeedChannel
 * @title: title of the feed
 *
 * To set a title to the @channel
 */
void
feed_channel_set_title (FeedChannel *channel, gchar *title)
{
	FREE_STRING (channel->priv->title);
	channel->priv->title = g_strdup (title);
}

/**
 * feed_channel_get_title:
 * @channel: a #FeedChannel
 *
 * Retrieves title of the @channel
 *
 * Return value: title of the feed, or NULL
 */
const gchar*
feed_channel_get_title (FeedChannel *channel)
{
	return (const gchar*) channel->priv->title;
}

/**
 * feed_channel_set_homepage:
 * @channel: a #FeedChannel
 * @homepage: homepage for the main website
 *
 * To set the homepage of the site the @channel belongs
 */
void
feed_channel_set_homepage (FeedChannel *channel, gchar *homepage)
{
	FREE_STRING (channel->priv->homepage);
	channel->priv->homepage = g_strdup (homepage);
}

/**
 * feed_channel_get_homepage:
 * @channel: a #FeedChannel
 *
 * Retrieves the homepage of the site for which @channel is the feed
 *
 * Return value: reference homepage of the feed, or NULL
 */
const gchar*
feed_channel_get_homepage (FeedChannel *channel)
{
	return (const gchar*) channel->priv->homepage;
}

/**
 * feed_channel_set_description:
 * @channel: a #FeedChannel
 * @description: description of the feed
 *
 * To set the description of @channel
 */
void
feed_channel_set_description (FeedChannel *channel, gchar *description)
{
	FREE_STRING (channel->priv->description);
	channel->priv->description = g_strdup (description);
}

/**
 * feed_channel_get_description:
 * @channel: a #FeedChannel
 *
 * Retrieves the description of @channel
 *
 * Return value: description of the feed, or NULL
 */
const gchar*
feed_channel_get_description (FeedChannel *channel)
{
	return (const gchar*) channel->priv->description;
}

/**
 * feed_channel_set_image:
 * @channel: a #FeedChannel
 * @image: URL of the image
 *
 * To set a rappresentative image to @channel
 */
void
feed_channel_set_image (FeedChannel *channel, gchar *image)
{
	FREE_STRING (channel->priv->image);
	channel->priv->image = g_strdup (image);
}

/**
 * feed_channel_get_image:
 * @channel: a #FeedChannel
 *
 * Retrieves the URL of the image assigned to the channel
 *
 * Return value: URL of the image, or NULL
 */
const gchar*
feed_channel_get_image (FeedChannel *channel)
{
	return (const gchar*) channel->priv->image;
}

/**
 * feed_channel_set_icon:
 * @channel: a #FeedChannel
 * @icon: URL where to retrieve the favicon
 *
 * To set the URL of the icon rappresenting @channel
 */
void
feed_channel_set_icon (FeedChannel *channel, gchar *icon)
{
	FREE_STRING (channel->priv->icon);
	channel->priv->icon = g_strdup (icon);
}

/**
 * feed_channel_get_icon:
 * @channel: a #FeedChannel
 *
 * Retrieves URL of the favicon of the channel (and/or the website for which
 * this is the feed)
 *
 * Return value: URL of the favicon, or NULL
 */
const gchar*
feed_channel_get_icon (FeedChannel *channel)
{
	return (const gchar*) channel->priv->icon;
}

/**
 * feed_channel_set_language:
 * @channel: a #FeedChannel
 * @language: string holding the language of the feed
 *
 * To set the language of @channel
 */
void
feed_channel_set_language (FeedChannel *channel, gchar *language)
{
	FREE_STRING (channel->priv->language);
	channel->priv->language = g_strdup (language);
}

/**
 * feed_channel_get_language:
 * @channel: a #FeedChannel
 *
 * Retrieves the language of the @channel
 *
 * Return value: string rappresenting the language of channel
 */
const gchar*
feed_channel_get_language (FeedChannel *channel)
{
	return (const gchar*) channel->priv->language;
}

/**
 * feed_channel_set_category:
 * @channel: a #FeedChannel
 * @category: category of the feed
 *
 * To set the category of the @channel
 */
void
feed_channel_set_category (FeedChannel *channel, gchar *category)
{
	FREE_STRING (channel->priv->category);
	channel->priv->category = g_strdup (category);
}

/**
 * feed_channel_get_category:
 * @channel: a #FeedChannel
 *
 * Retrieves category of the @channel
 *
 * Return value: category of the feed, or NULL
 */
const gchar*
feed_channel_get_category (FeedChannel *channel)
{
	return (const gchar*) channel->priv->category;
}

/**
 * feed_channel_set_pubsubhub:
 * @channel: a #FeedChannel
 * @hub: hub for the feed, or NULL
 * @self: target referencing the feed, or NULL
 *
 * To set information about PubSubHub for the channel. Options can be set
 * alternatively, only with hub != %NULL or self != %NULL, and are saved
 * internally to the object: the hub is considered valid
 * (feed_channel_get_pubsubhub() returns %TRUE) only when both parameters has
 * been set. To unset the hub, pass %NULL for both parameters
 */
void
feed_channel_set_pubsubhub (FeedChannel *channel, gchar *hub, gchar *self)
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
 * feed_channel_get_pubsubhub:
 * @channel: a #FeedChannel
 * @hub: location for the hub string, or NULL
 * @self: location for the reference to the feed, or NULL
 *
 * Retrieves information about the PubSub hub of the channel
 *
 * Return value: %TRUE if a valid PubSub hub has been set for the @channel,
 * %FALSE otherwise
 */
gboolean
feed_channel_get_pubsubhub (FeedChannel *channel, gchar **hub, gchar **self)
{
	if (hub != NULL)
		*hub = channel->priv->pubsub.hub;
	if (self != NULL)
		*self = channel->priv->pubsub.self;

	return (channel->priv->pubsub.hub != NULL && channel->priv->pubsub.self != NULL);
}

/**
 * feed_channel_set_copyright:
 * @channel: a #FeedChannel
 * @copyright: copyright of the channel
 *
 * To set the copyright of the feed
 */
void
feed_channel_set_copyright (FeedChannel *channel, gchar *copyright)
{
	FREE_STRING (channel->priv->copyright);
	channel->priv->copyright = g_strdup (copyright);
}

/**
 * feed_channel_get_copyright:
 * @channel: a #FeedChannel
 *
 * Retrieves indications about the copyright
 *
 * Return value: copyright of the @channel, or NULL
 */
const gchar*
feed_channel_get_copyright (FeedChannel *channel)
{
	return (const gchar*) channel->priv->copyright;
}

/**
 * feed_channel_set_editor:
 * @channel: a #FeedChannel
 * @editor: editor of the feed
 *
 * To set the editor of the @channel
 */
void
feed_channel_set_editor (FeedChannel *channel, gchar *editor)
{
	FREE_STRING (channel->priv->editor);
	channel->priv->editor = g_strdup (editor);
}

/**
 * feed_channel_get_editor:
 * @channel: a #FeedChannel
 *
 * Retrieves reference to the editor or the @channel
 *
 * Return value: editor of the feed, or NULL
 */
const gchar*
feed_channel_get_editor (FeedChannel *channel)
{
	return (const gchar*) channel->priv->editor;
}

/**
 * feed_channel_add_contributor:
 * @channel: a #FeedChannel
 * @contributor: contributor of the feed
 *
 * To add a contributor to the @channel
 */
void
feed_channel_add_contributor (FeedChannel *channel, gchar *contributor)
{
	gchar *con;

	con = g_strdup (contributor);

	if (channel->priv->contributors == NULL)
		channel->priv->contributors = g_list_prepend (channel->priv->contributors, con);
	else
		channel->priv->contributors = g_list_append (channel->priv->contributors, con);
}

/**
 * feed_channel_get_contributors:
 * @channel: a #FeedChannel
 *
 * Retrieves reference to the contributors of the @channel
 *
 * Return value: list of contributors to the channel, or NULL
 */
const GList*
feed_channel_get_contributor (FeedChannel *channel)
{
	return (const GList*) channel->priv->contributors;
}

/**
 * feed_channel_set_webmaster:
 * @channel: a #FeedChannel
 * @webmaster: webmaster of the feed
 *
 * To assign a webmaster to the @channel
 */
void
feed_channel_set_webmaster (FeedChannel *channel, gchar *webmaster)
{
	FREE_STRING (channel->priv->webmaster);
	channel->priv->webmaster = g_strdup (webmaster);
}

/**
 * feed_channel_get_webmaster:
 * @channel: a #FeedChannel
 *
 * Retrieves reference to the webmaster of the feed
 *
 * Return value: webmaster of @channel, or NULL
 */
const gchar*
feed_channel_get_webmaster (FeedChannel *channel)
{
	return (const gchar*) channel->priv->webmaster;
}

/**
 * feed_channel_set_generator:
 * @channel: a #FeedChannel
 * @generator: software generator of the feed
 *
 * To set information about the software generator of @channel
 */
void
feed_channel_set_generator (FeedChannel *channel, gchar *generator)
{
	FREE_STRING (channel->priv->generator);
	channel->priv->generator = g_strdup (generator);
}

/**
 * feed_channel_get_generator:
 * @channel: a #FeedChannel
 *
 * Retrieves information about the feed's software generator
 *
 * Return value: generator of @channel, or NULL
 */
const gchar*
feed_channel_get_generator (FeedChannel *channel)
{
	return (const gchar*) channel->priv->generator;
}

/**
 * feed_channel_set_publish_time:
 * @channel: a #FeedChannel
 * @publish: timestamp of publishing
 *
 * To set the time of publishing for the feed
 */
void
feed_channel_set_publish_time (FeedChannel *channel, time_t publish)
{
	channel->priv->pub_time = publish;
}

/**
 * feed_channel_get_publish_time:
 * @channel: a #FeedChannel
 *
 * Retrieves the publishing time of @channel
 *
 * Return value: time of feed's publish
 */
time_t
feed_channel_get_publish_time (FeedChannel *channel)
{
	return channel->priv->pub_time;
}

/**
 * feed_channel_set_update_time:
 * @channel: a #FeedChannel
 * @update: update time of the feed
 *
 * To set the latest update time of @channel
 */
void
feed_channel_set_update_time (FeedChannel *channel, time_t update)
{
	channel->priv->update_time = update;
}

/**
 * feed_channel_get_update_time:
 * @channel: a #FeedChannel
 *
 * Retrieves the update time of @channel
 *
 * Return value: time of the feed's latest update. If this value was not set
 * (with feed_channel_set_update_time()) returns
 * feed_channel_get_publish_time()
 */
time_t
feed_channel_get_update_time (FeedChannel *channel)
{
	return channel->priv->update_time;
}

/**
 * feed_channel_set_update_interval:
 * @channel: a #FeedChannel
 * @minutes: update interval, in minutes
 *
 * To set the update interval for @channel
 */
void
feed_channel_set_update_interval (FeedChannel *channel, int minutes)
{
	channel->priv->update_interval = minutes;
}

/**
 * feed_channel_get_update_interval:
 * @channel: a #FeedChannel
 *
 * Retrieves the update interval for the feed. Pay attention to the fact the
 * value can be unset, and the function returns 0: in this case the caller
 * must manually set a default update interval with
 * feed_channel_set_update_interval()
 *
 * Return value: update interval for the @channel, in minutes
 */
int
feed_channel_get_update_interval (FeedChannel *channel)
{
	return channel->priv->update_interval;
}

/**
 * feed_channel_fetch:
 * @channel: a #FeedChannel
 *
 * Utility to fetch and populate a #FeedChannel for the first time, and init
 * all his internal values. Only the source URL has to be set in @channel
 * (with feed_channel_set_source()). Be aware this function is sync, do not
 * returns until the feed isn't downloaded and parsed
 *
 * Return value: %TRUE if the feed is correctly fetched and parsed, %FALSE
 * otherwise
 */
gboolean
feed_channel_fetch (FeedChannel *channel)
{
	gboolean ret;
	guint status;
	GList *items;
	GList *iter;
	xmlDocPtr doc;
	SoupMessage *msg;
	SoupSession *session;
	FeedParser *parser;

	/*
		TODO	This function is quite inefficent because parses all
			the feed with a FeedParser and them waste obtained
			FeedItems. Perhaps a more aimed function in
			FeedParser would help...
	*/

	ret = FALSE;
	session = soup_session_sync_new ();
	msg = soup_message_new ("GET", feed_channel_get_source (channel));
	status = soup_session_send_message (session, msg);

	if (status >= 200 && status <= 299) {
		doc = content_to_xml (msg->response_body->data, msg->response_body->length);

		if (doc != NULL) {
			parser = feed_parser_new ();
			items = feed_parser_parse (parser, channel, doc, NULL);

			for (iter = items; iter; iter = g_list_next (iter))
				g_object_unref (iter->data);
			g_list_free (items);

			g_object_unref (parser);
			xmlFreeDoc (doc);
			ret = TRUE;
		}
	}
	else {
		g_warning ("Unable to fetch feed from %s: %s", feed_channel_get_source (channel), soup_status_get_phrase (status));
	}

	g_object_unref (session);
	g_object_unref (msg);
	return ret;
}
