/*
 * Copyright (C) 2011-2015, Roberto Guido <rguido@src.gnome.org>
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

#ifndef __FEEDS_PUBSUBHUBBUB_SUBSCRIBER_H__
#define __FEEDS_PUBSUBHUBBUB_SUBSCRIBER_H__

#include "libgrss.h"

#define GRSS_FEEDS_PUBSUBHUBBUB_SUBSCRIBER_TYPE		(grss_feeds_pubsubhubbub_subscriber_get_type())
#define GRSS_FEEDS_PUBSUBHUBBUB_SUBSCRIBER(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), GRSS_FEEDS_PUBSUBHUBBUB_SUBSCRIBER_TYPE, GrssFeedsPubsubhubbubSubscriber))
#define GRSS_FEEDS_PUBSUBHUBBUB_SUBSCRIBER_CLASS(c)	(G_TYPE_CHECK_CLASS_CAST ((c), GRSS_FEEDS_PUBSUBHUBBUB_SUBSCRIBER_TYPE, GrssFeedsPubsubhubbubSubscriberClass))
#define GRSS_IS_FEEDS_PUBSUBHUBBUB_SUBSCRIBER(o)	(G_TYPE_CHECK_INSTANCE_TYPE ((o), GRSS_FEEDS_PUBSUBHUBBUB_SUBSCRIBER_TYPE))
#define GRSS_IS_FEEDS_PUBSUBHUBBUB_SUBSCRIBER_CLASS(c)	(G_TYPE_CHECK_CLASS_TYPE ((c),  GRSS_FEEDS_PUBSUBHUBBUB_SUBSCRIBER_TYPE))
#define GRSS_PUBSUBHUBBUB_SUBSCRIBER_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), GRSS_FEEDS_PUBSUBHUBBUB_SUBSCRIBER_TYPE, GrssFeedsPubsubhubbubSubscriberClass))

typedef struct _GrssFeedsPubsubhubbubSubscriber		GrssFeedsPubsubhubbubSubscriber;
typedef struct _GrssFeedsPubsubhubbubSubscriberPrivate	GrssFeedsPubsubhubbubSubscriberPrivate;

struct _GrssFeedsPubsubhubbubSubscriber {
	GObject parent;
	GrssFeedsPubsubhubbubSubscriberPrivate *priv;
};

typedef struct {
	GObjectClass parent;
} GrssFeedsPubsubhubbubSubscriberClass;

GType					grss_feeds_pubsubhubbub_subscriber_get_type	() G_GNUC_CONST;

GrssFeedsPubsubhubbubSubscriber*	grss_feeds_pubsubhubbub_subscriber_new		();

#endif /* __FEEDS_PUBSUBHUBBUB_SUBSCRIBER_H__ */
