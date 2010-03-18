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

#ifndef __FEEDS_SUBSCRIBER_H__
#define __FEEDS_SUBSCRIBER_H__

#include "common.h"
#include "feed-channel.h"
#include "feed-item.h"

#define FEEDS_SUBSCRIBER_TYPE		(feeds_subscriber_get_type())
#define FEEDS_SUBSCRIBER(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), FEEDS_SUBSCRIBER_TYPE, FeedsSubscriber))
#define FEEDS_SUBSCRIBER_CLASS(c)	(G_TYPE_CHECK_CLASS_CAST ((c), FEEDS_SUBSCRIBER_TYPE, FeedsSubscriberClass))
#define IS_FEEDS_SUBSCRIBER(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), FEEDS_SUBSCRIBER_TYPE))
#define IS_FEEDS_SUBSCRIBER_CLASS(c)	(G_TYPE_CHECK_CLASS_TYPE ((c),  FEEDS_SUBSCRIBER_TYPE))
#define FEEDS_SUBSCRIBER_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), FEEDS_SUBSCRIBER_TYPE, FeedsSubscriberClass))

typedef struct _FeedsSubscriber		FeedsSubscriber;
typedef struct _FeedsSubscriberPrivate	FeedsSubscriberPrivate;

struct _FeedsSubscriber {
	GObject parent;
	FeedsSubscriberPrivate *priv;
};

typedef struct {
	GObjectClass parent;

	void (*notification_received) (FeedsSubscriber *sub, FeedChannel *feed, FeedItem *item);
} FeedsSubscriberClass;

GType			feeds_subscriber_get_type	() G_GNUC_CONST;

FeedsSubscriber*	feeds_subscriber_new		();

gboolean		feeds_subscriber_listen		(FeedsSubscriber *sub, GList *feeds);
GList*			feeds_subscriber_get_listened	(FeedsSubscriber *sub);
void			feeds_subscriber_set_port	(FeedsSubscriber *sub, int port);
void			feeds_subscriber_set_hub	(FeedsSubscriber *sub, gchar *hub);
void			feeds_subscriber_switch		(FeedsSubscriber *sub, gboolean run);

#endif /* __FEEDS_SUBSCRIBER_H__ */
