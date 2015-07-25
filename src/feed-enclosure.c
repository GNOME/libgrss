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
#include "feed-enclosure.h"

#define FEED_ENCLOSURE_GET_PRIVATE(obj)	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), GRSS_FEED_ENCLOSURE_TYPE, GrssFeedEnclosurePrivate))
#define FEED_ENCLOSURE_ERROR		feed_enclosure_error_quark()

/**
 * SECTION: feed-enclosure
 * @short_description: a component attached to an item
 *
 * #GrssFeedEnclosure describes an external element embedded into a
 * #GrssFeedItem: it may be an image, a video of other kind of file to be
 * presented with the parent item.
 */

struct _GrssFeedEnclosurePrivate {
	gchar	*url;
	gchar	*type;
	gsize	length;
};

enum {
	FEED_ENCLOSURE_FETCH_ERROR,
	FEED_ENCLOSURE_FILE_ERROR,
};

G_DEFINE_TYPE (GrssFeedEnclosure, grss_feed_enclosure, G_TYPE_OBJECT);

static GQuark
feed_enclosure_error_quark ()
{
	return g_quark_from_static_string ("feed_enclosure_error");
}

static void
grss_feed_enclosure_finalize (GObject *obj)
{
	GrssFeedEnclosure *enclosure;

	enclosure = GRSS_FEED_ENCLOSURE (obj);
	FREE_STRING (enclosure->priv->url);
	FREE_STRING (enclosure->priv->type);
}

static void
grss_feed_enclosure_class_init (GrssFeedEnclosureClass *klass)
{
	GObjectClass *gobject_class;

	g_type_class_add_private (klass, sizeof (GrssFeedEnclosurePrivate));

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->finalize = grss_feed_enclosure_finalize;
}

static void
grss_feed_enclosure_init (GrssFeedEnclosure *node)
{
	node->priv = FEED_ENCLOSURE_GET_PRIVATE (node);
	memset (node->priv, 0, sizeof (GrssFeedEnclosurePrivate));
}

/**
 * grss_feed_enclosure_new:
 * @url: URL of the external element.
 *
 * Allocates a new #GrssFeedEnclosure, to be downloaded separately.
 *
 * Returns: a new #GrssFeedEnclosure.
 */
GrssFeedEnclosure*
grss_feed_enclosure_new (gchar *url)
{
	GrssFeedEnclosure *ret;

	ret = g_object_new (GRSS_FEED_ENCLOSURE_TYPE, NULL);
	ret->priv->url = g_strdup (url);
	return ret;
}

/**
 * grss_feed_enclosure_get_url:
 * @enclosure: a #GrssFeedEnclosure.
 *
 * Retrieves the URL of the @enclosure.
 *
 * Returns: the URL where the enclosure may be found.
 */
const gchar*
grss_feed_enclosure_get_url (GrssFeedEnclosure *enclosure)
{
	return (const gchar*) enclosure->priv->url;
}

/**
 * grss_feed_enclosure_set_format:
 * @enclosure: a #GrssFeedEnclosure.
 * @type: type of content.
 *
 * To set the type of the external file.
 */
void
grss_feed_enclosure_set_format (GrssFeedEnclosure *enclosure, gchar *type)
{
	FREE_STRING (enclosure->priv->type);
	enclosure->priv->type = g_strdup (type);
}

/**
 * grss_feed_enclosure_get_format:
 * @enclosure: a #GrssFeedEnclosure.
 *
 * Retrieves the format of the enclosed file.
 *
 * Returns: type of @enclosure.
 */
const gchar*
grss_feed_enclosure_get_format (GrssFeedEnclosure *enclosure)
{
	return (const gchar*) enclosure->priv->type;
}

/**
 * grss_feed_enclosure_set_length:
 * @enclosure: a #GrssFeedEnclosure.
 * @length: size of the enclosure, in bytes.
 *
 * To set the size of the embedded @enclosure.
 */
void
grss_feed_enclosure_set_length (GrssFeedEnclosure *enclosure, gsize length)
{
	enclosure->priv->length = length;
}

/**
 * grss_feed_enclosure_get_length:
 * @enclosure: a #GrssFeedEnclosure.
 *
 * Retrieves the size of the embedded file.
 *
 * Returns: size of the @enclosure, in bytes.
 */
gsize
grss_feed_enclosure_get_length (GrssFeedEnclosure *enclosure)
{
	return enclosure->priv->length;
}

static GFile*
msg_to_internal_file (GrssFeedEnclosure *enclosure, SoupMessage *msg, GError **error)
{
	gboolean test;
	GFile *ret;
	GFileIOStream *stream;
	GOutputStream *stream_out;

	ret = g_file_new_tmp ("enclosure_XXXXXX", &stream, error);

	if (ret != NULL) {
		stream_out = g_io_stream_get_output_stream (G_IO_STREAM (stream));
		test = g_output_stream_write_all (stream_out, msg->response_body->data, msg->response_body->length,
		                                  NULL, NULL, error);

		if (test == FALSE) {
			g_object_unref (ret);
			ret = NULL;
		}

		g_object_unref (stream_out);
		g_object_unref (stream);
	}

	return ret;
}

/**
 * grss_feed_enclosure_fetch:
 * @enclosure: a #GrssFeedEnclosure.
 * @error: if an error occurred, %NULL is returned and this is filled with the
 *         message.
 *
 * Utility to fetch a #GrssFeedEnclosure. Contents are stored in a temporary
 * #GFile, which is suggested to move on a permanent location to keep it over
 * time.
 *
 * Returns: (transfer full): temporary file where the contents have been
 * written, or %NULL if an error occours.
 */
GFile*
grss_feed_enclosure_fetch (GrssFeedEnclosure *enclosure, GError **error)
{
	const gchar *url;
	guint status;
	GFile *ret;
	SoupMessage *msg;
	SoupSession *session;

	ret = NULL;
	url = grss_feed_enclosure_get_url (enclosure);

	session = soup_session_sync_new ();
	msg = soup_message_new ("GET", url);
	status = soup_session_send_message (session, msg);

	if (status >= 200 && status <= 299)
		ret = msg_to_internal_file (enclosure, msg, error);
	else
		g_set_error (error, FEED_ENCLOSURE_ERROR, FEED_ENCLOSURE_FETCH_ERROR,
		             "Unable to download from %s", url);

	g_object_unref (session);
	g_object_unref (msg);
	return ret;
}

static void
enclosure_downloaded (SoupSession *session, SoupMessage *msg, gpointer user_data) {
	guint status;
	const gchar *url;
	GFile *file;
	GTask *task;
	GrssFeedEnclosure *enclosure;
	GError *error;

	task = user_data;
	enclosure = GRSS_FEED_ENCLOSURE (g_task_get_source_object (task));
	url = grss_feed_enclosure_get_url (enclosure);
	g_object_get (msg, "status-code", &status, NULL);

	if (status >= 200 && status <= 299) {
		error = NULL;
		file = msg_to_internal_file (enclosure, msg, &error);

		if (file != NULL)
			g_task_return_pointer (task, file, g_object_unref);
		else
			g_task_return_error (task, error);
	}
	else {
		g_task_return_new_error (task, FEED_ENCLOSURE_ERROR, FEED_ENCLOSURE_FETCH_ERROR,
						 "Unable to download from %s", url);
	}

	g_object_unref (task);
}

/**
 * grss_feed_enclosure_fetch_async:
 * @enclosure: a #GrssFeedEnclosure.
 * @callback: function to invoke at the end of the download.
 * @user_data: data passed to the callback.
 *
 * Similar to grss_feed_enclosure_fetch(), but asyncronous.
 */
void
grss_feed_enclosure_fetch_async (GrssFeedEnclosure *enclosure, GAsyncReadyCallback callback, gpointer user_data)
{
	GTask *task;
	SoupMessage *msg;
	SoupSession *session;

	task = g_task_new (enclosure, NULL, callback, user_data);
	session = soup_session_async_new ();
	msg = soup_message_new ("GET", grss_feed_enclosure_get_url (enclosure));
	soup_session_queue_message (session, msg, enclosure_downloaded, task);
}

/**
 * grss_feed_enclosure_fetch_finish:
 * @enclosure: a #GrssFeedEnclosure.
 * @res: the #GAsyncResult passed to the callback.
 * @error: if an error occurred, %NULL is returned and this is filled with the
 *         message.
 *
 * Finalizes an asyncronous operation started with
 * grss_feed_enclosure_fetch_async().
 *
 * Returns: (transfer full): temporary file where the contents have been
 * written, or %NULL if an error occours.
 */
GFile*
grss_feed_enclosure_fetch_finish (GrssFeedEnclosure *enclosure, GAsyncResult *res, GError **error)
{
	return g_task_propagate_pointer (G_TASK (res), error);
}
