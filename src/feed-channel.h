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

#ifndef __FEED_CHANNEL_H__
#define __FEED_CHANNEL_H__

#include "libgrss.h"

#define GRSS_FEED_CHANNEL_TYPE		(grss_feed_channel_get_type())
#define GRSS_FEED_CHANNEL(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), GRSS_FEED_CHANNEL_TYPE, GrssFeedChannel))
#define FEED_CHANNEL_CLASS(c)		(G_TYPE_CHECK_CLASS_CAST ((c), GRSS_FEED_CHANNEL_TYPE, GrssFeedChannelClass))
#define GRSS_IS_FEED_CHANNEL(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), GRSS_FEED_CHANNEL_TYPE))
#define GRSS_IS_FEED_CHANNEL_CLASS(c)	(G_TYPE_CHECK_CLASS_TYPE ((c),  GRSS_FEED_CHANNEL_TYPE))
#define GRSS_FEED_CHANNEL_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), GRSS_FEED_CHANNEL_TYPE, GrssFeedChannelClass))

typedef struct _GrssFeedChannel		GrssFeedChannel;
typedef struct _GrssFeedChannelPrivate	GrssFeedChannelPrivate;

struct _GrssFeedChannel {
	GObject parent;
	GrssFeedChannelPrivate *priv;
};

typedef struct {
	GObjectClass parent;
} GrssFeedChannelClass;

GType			grss_feed_channel_get_type		(void) G_GNUC_CONST;

GrssFeedChannel*	grss_feed_channel_new			();
GrssFeedChannel*	grss_feed_channel_new_from_source	(gchar *source);
GrssFeedChannel*	grss_feed_channel_new_from_file		(const gchar *path);

void			grss_feed_channel_set_format		(GrssFeedChannel *channel, gchar *format);
const gchar*		grss_feed_channel_get_format		(GrssFeedChannel *channel);
void			grss_feed_channel_set_source		(GrssFeedChannel *channel, gchar *source);
const gchar*		grss_feed_channel_get_source		(GrssFeedChannel *channel);
void			grss_feed_channel_set_title		(GrssFeedChannel *channel, gchar *title);
const gchar*		grss_feed_channel_get_title		(GrssFeedChannel *channel);
void			grss_feed_channel_set_homepage		(GrssFeedChannel *channel, gchar *homepage);
const gchar*		grss_feed_channel_get_homepage		(GrssFeedChannel *channel);
void			grss_feed_channel_set_description	(GrssFeedChannel *channel, gchar *description);
const gchar*		grss_feed_channel_get_description	(GrssFeedChannel *channel);
void			grss_feed_channel_set_image		(GrssFeedChannel *channel, gchar *image);
const gchar*		grss_feed_channel_get_image		(GrssFeedChannel *channel);
void			grss_feed_channel_set_icon		(GrssFeedChannel *channel, gchar *icon);
const gchar*		grss_feed_channel_get_icon		(GrssFeedChannel *channel);
void			grss_feed_channel_set_language		(GrssFeedChannel *channel, gchar *language);
const gchar*		grss_feed_channel_get_language		(GrssFeedChannel *channel);
void			grss_feed_channel_set_category		(GrssFeedChannel *channel, gchar *category);
const gchar*		grss_feed_channel_get_category		(GrssFeedChannel *channel);
void			grss_feed_channel_set_pubsubhub		(GrssFeedChannel *channel, gchar *hub);
gboolean		grss_feed_channel_get_pubsubhub		(GrssFeedChannel *channel, gchar **hub);
void			grss_feed_channel_set_rsscloud		(GrssFeedChannel *channel, gchar *path, gchar *protocol);
gboolean		grss_feed_channel_get_rsscloud		(GrssFeedChannel *channel, gchar **path, gchar **protocol);

void			grss_feed_channel_set_copyright		(GrssFeedChannel *channel, gchar *copyright);
const gchar*		grss_feed_channel_get_copyright		(GrssFeedChannel *channel);
void			grss_feed_channel_set_editor		(GrssFeedChannel *channel, gchar *editor);
const gchar*		grss_feed_channel_get_editor		(GrssFeedChannel *channel);
void			grss_feed_channel_add_contributor	(GrssFeedChannel *channel, gchar *contributor);
const GList*		grss_feed_channel_get_contributors	(GrssFeedChannel *channel);
void			grss_feed_channel_set_webmaster		(GrssFeedChannel *channel, gchar *webmaster);
const gchar*		grss_feed_channel_get_webmaster		(GrssFeedChannel *channel);
void			grss_feed_channel_set_generator		(GrssFeedChannel *channel, gchar *generator);
const gchar*		grss_feed_channel_get_generator		(GrssFeedChannel *channel);

void			grss_feed_channel_set_publish_time	(GrssFeedChannel *channel, time_t publish);
time_t			grss_feed_channel_get_publish_time	(GrssFeedChannel *channel);
void			grss_feed_channel_set_update_time	(GrssFeedChannel *channel, time_t update);
time_t			grss_feed_channel_get_update_time	(GrssFeedChannel *channel);
void			grss_feed_channel_set_update_interval	(GrssFeedChannel *channel, int minutes);
int			grss_feed_channel_get_update_interval	(GrssFeedChannel *channel);

gboolean		grss_feed_channel_fetch			(GrssFeedChannel *channel);
void			grss_feed_channel_fetch_async		(GrssFeedChannel *channel, GAsyncReadyCallback callback, gpointer user_data);
gboolean		grss_feed_channel_fetch_finish		(GrssFeedChannel *channel, GAsyncResult *res, GError **error);
GList*			grss_feed_channel_fetch_all		(GrssFeedChannel *channel);
void			grss_feed_channel_fetch_all_async	(GrssFeedChannel *channel, GAsyncReadyCallback callback, gpointer user_data);
GList*			grss_feed_channel_fetch_all_finish	(GrssFeedChannel *channel, GAsyncResult *res, GError **error);

#endif /* __FEED_CHANNEL_H__ */
