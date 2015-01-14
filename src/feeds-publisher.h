/*
 * Copyright (C) 2010-2015, Roberto Guido <rguido@src.gnome.org>
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

#ifndef __FEEDS_PUBLISHER_H__
#define __FEEDS_PUBLISHER_H__

#include "libgrss.h"

#define FEEDS_PUBLISHER_TYPE		(grss_feeds_publisher_get_type())
#define FEEDS_PUBLISHER(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), FEEDS_PUBLISHER_TYPE, GrssFeedsPublisher))
#define FEEDS_PUBLISHER_CLASS(c)	(G_TYPE_CHECK_CLASS_CAST ((c), FEEDS_PUBLISHER_TYPE, GrssFeedsPublisherClass))
#define IS_FEEDS_PUBLISHER(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), FEEDS_PUBLISHER_TYPE))
#define IS_FEEDS_PUBLISHER_CLASS(c)	(G_TYPE_CHECK_CLASS_TYPE ((c),  FEEDS_PUBLISHER_TYPE))
#define FEEDS_PUBLISHER_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), FEEDS_PUBLISHER_TYPE, GrssFeedsPublisherClass))

typedef struct _GrssFeedsPublisher		GrssFeedsPublisher;
typedef struct _GrssFeedsPublisherPrivate	GrssFeedsPublisherPrivate;

struct _GrssFeedsPublisher {
	GObject parent;
	GrssFeedsPublisherPrivate *priv;
};

typedef struct {
	GObjectClass parent;

	void (*new_subscription) (GrssFeedsPublisher *pub, GrssFeedChannel *topic, gchar *callback);
	void (*delete_subscription) (GrssFeedsPublisher *pub, GrssFeedChannel *topic, gchar *callback);
} GrssFeedsPublisherClass;

GType			grss_feeds_publisher_get_type		() G_GNUC_CONST;

GrssFeedsPublisher*	grss_feeds_publisher_new		();

gchar*			grss_feeds_publisher_format_content	(GrssFeedsPublisher *pub, GrssFeedChannel *channel, GList *items, GError **error);
gboolean		grss_feeds_publisher_publish_web	(GrssFeedsPublisher *pub, GrssFeedChannel *channel, GList *items, const gchar *id, GError **error);
gboolean		grss_feeds_publisher_publish_file	(GrssFeedsPublisher *pub, GrssFeedChannel *channel, GList *items, const gchar *uri, GError **error);

void			grss_feeds_publisher_hub_set_port	(GrssFeedsPublisher *pub, int port);
void			grss_feeds_publisher_hub_set_topics	(GrssFeedsPublisher *pub, GList *topics);
void			grss_feeds_publisher_hub_switch		(GrssFeedsPublisher *pub, gboolean run);

#endif /* __FEEDS_PUBLISHER_H__ */
