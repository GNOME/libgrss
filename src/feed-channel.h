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

#include "common.h"

#define FEED_CHANNEL_TYPE		(feed_channel_get_type())
#define FEED_CHANNEL(o)			(G_TYPE_CHECK_INSTANCE_CAST ((o), FEED_CHANNEL_TYPE, FeedChannel))
#define FEED_CHANNEL_CLASS(c)		(G_TYPE_CHECK_CLASS_CAST ((c), FEED_CHANNEL_TYPE, FeedChannelClass))
#define IS_FEED_CHANNEL(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), FEED_CHANNEL_TYPE))
#define IS_FEED_CHANNEL_CLASS(c)	(G_TYPE_CHECK_CLASS_TYPE ((c),  FEED_CHANNEL_TYPE))
#define FEED_CHANNEL_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), FEED_CHANNEL_TYPE, FeedChannelClass))

typedef struct _FeedChannel		FeedChannel;
typedef struct _FeedChannelPrivate	FeedChannelPrivate;

struct _FeedChannel {
	GObject parent;
	FeedChannelPrivate *priv;
};

typedef struct {
	GObjectClass parent;
} FeedChannelClass;

GType		feed_channel_get_type			(void) G_GNUC_CONST;

FeedChannel*	feed_channel_new			();

void		feed_channel_set_source			(FeedChannel *channel, gchar *source);
const gchar*	feed_channel_get_source			(FeedChannel *channel);
void		feed_channel_set_title			(FeedChannel *channel, gchar *title);
const gchar*	feed_channel_get_title			(FeedChannel *channel);
void		feed_channel_set_homepage		(FeedChannel *channel, gchar *homepage);
const gchar*	feed_channel_get_homepage		(FeedChannel *channel);
void		feed_channel_set_description		(FeedChannel *channel, gchar *description);
const gchar*	feed_channel_get_description		(FeedChannel *channel);
void		feed_channel_set_image			(FeedChannel *channel, gchar *image);
const gchar*	feed_channel_get_image			(FeedChannel *channel);
void		feed_channel_set_icon			(FeedChannel *channel, gchar *icon);
const gchar*	feed_channel_get_icon			(FeedChannel *channel);
void		feed_channel_set_language		(FeedChannel *channel, gchar *language);
const gchar*	feed_channel_get_language		(FeedChannel *channel);
void		feed_channel_set_category		(FeedChannel *channel, gchar *category);
const gchar*	feed_channel_get_category		(FeedChannel *channel);
void		feed_channel_set_pubsubhub		(FeedChannel *channel, gchar *hub, gchar *self);
gboolean	feed_channel_get_pubsubhub		(FeedChannel *channel, gchar **hub, gchar **self);

void		feed_channel_set_copyright		(FeedChannel *channel, gchar *copyright);
const gchar*	feed_channel_get_copyright		(FeedChannel *channel);
void		feed_channel_set_editor			(FeedChannel *channel, gchar *editor);
const gchar*	feed_channel_get_editor			(FeedChannel *channel);
void		feed_channel_add_contributor		(FeedChannel *channel, gchar *contributor);
const GList*	feed_channel_get_contributors		(FeedChannel *channel);
void		feed_channel_set_webmaster		(FeedChannel *channel, gchar *webmaster);
const gchar*	feed_channel_get_webmaster		(FeedChannel *channel);
void		feed_channel_set_generator		(FeedChannel *channel, gchar *generator);
const gchar*	feed_channel_get_generator		(FeedChannel *channel);

void		feed_channel_set_publish_time		(FeedChannel *channel, time_t publish);
time_t		feed_channel_get_publish_time		(FeedChannel *channel);
void		feed_channel_set_update_time		(FeedChannel *channel, time_t update);
time_t		feed_channel_get_update_time		(FeedChannel *channel);
void		feed_channel_set_update_interval	(FeedChannel *channel, int minutes);
int		feed_channel_get_update_interval	(FeedChannel *channel);

gboolean	feed_channel_fetch			(FeedChannel *channel);

#endif /* __FEED_CHANNEL_H__ */
