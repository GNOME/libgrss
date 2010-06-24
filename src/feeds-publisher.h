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

#ifndef __FEEDS_PUBLISHER_H__
#define __FEEDS_PUBLISHER_H__

#include "libgrss.h"

#define FEEDS_PUBLISHER_TYPE		(feeds_publisher_get_type())
#define FEEDS_PUBLISHER(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), FEEDS_PUBLISHER_TYPE, FeedsPublisher))
#define FEEDS_PUBLISHER_CLASS(c)	(G_TYPE_CHECK_CLASS_CAST ((c), FEEDS_PUBLISHER_TYPE, FeedsPublisherClass))
#define IS_FEEDS_PUBLISHER(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), FEEDS_PUBLISHER_TYPE))
#define IS_FEEDS_PUBLISHER_CLASS(c)	(G_TYPE_CHECK_CLASS_TYPE ((c),  FEEDS_PUBLISHER_TYPE))
#define FEEDS_PUBLISHER_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), FEEDS_PUBLISHER_TYPE, FeedsPublisherClass))

typedef struct _FeedsPublisher		FeedsPublisher;
typedef struct _FeedsPublisherPrivate	FeedsPublisherPrivate;

struct _FeedsPublisher {
	GObject parent;
	FeedsPublisherPrivate *priv;
};

typedef struct {
	GObjectClass parent;

	void (*new_subscription) (FeedsPublisher *pub, FeedChannel *topic, gchar *callback);
	void (*delete_subscription) (FeedsPublisher *pub, FeedChannel *topic, gchar *callback);
} FeedsPublisherClass;

GType			feeds_publisher_get_type	() G_GNUC_CONST;

FeedsPublisher*		feeds_publisher_new		();

void			feeds_publisher_set_port	(FeedsPublisher *pub, int port);
void			feeds_publisher_set_topics	(FeedsPublisher *pub, GList *topics);
void			feeds_publisher_switch		(FeedsPublisher *pub, gboolean run);
void			feeds_publisher_publish		(FeedsPublisher *pub, FeedItem *item);

#endif /* __FEEDS_PUBLISHER_H__ */
