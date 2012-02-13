/*
 * Copyright (C) 2011-2012, Roberto Guido <rguido@src.gnome.org>
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

#ifndef __GRSS_FEEDS_SUBSCRIBER_HANDLER_H__
#define __GRSS_FEEDS_SUBSCRIBER_HANDLER_H__

#include "libgrss.h"

#define GRSS_FEEDS_SUBSCRIBER_HANDLER_TYPE			(grss_feeds_subscriber_handler_get_type ())
#define GRSS_FEEDS_SUBSCRIBER_HANDLER(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GRSS_FEEDS_SUBSCRIBER_HANDLER_TYPE, GrssFeedsSubscriberHandler))
#define GRSS_IS_FEEDS_SUBSCRIBER_HANDLER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRSS_FEEDS_SUBSCRIBER_HANDLER_TYPE))
#define GRSS_FEEDS_SUBSCRIBER_HANDLER_GET_INTERFACE(inst)	(G_TYPE_INSTANCE_GET_INTERFACE ((inst), GRSS_FEEDS_SUBSCRIBER_HANDLER_TYPE, GrssFeedsSubscriberHandlerInterface))

typedef struct _GrssFeedsSubscriberHandler		GrssFeedsSubscriberHandler;
typedef struct _GrssFeedsSubscriberHandlerInterface	GrssFeedsSubscriberHandlerInterface;

typedef enum {
	FEED_SUBSCRIPTION_IDLE,
	FEED_SUBSCRIPTION_SUBSCRIBING,
	FEED_SUBSCRIPTION_SUBSCRIBED,
	FEED_SUBSCRIPTION_UNSUBSCRIBING,
} FEED_SUBSCRIPTION_STATUS;

struct _GrssFeedsSubscriberHandlerInterface {
	GTypeInterface parent_iface;

	void (*set_parent) (GrssFeedsSubscriberHandler *handler, GrssFeedsSubscriber *parent);
	gboolean (*check_format) (GrssFeedsSubscriberHandler *handler, GrssFeedChannel *channel);
	void (*subscribe) (GrssFeedsSubscriberHandler *handler, GrssFeedChannel *channel, gchar *assigned_url);
	GList* (*handle_message) (GrssFeedsSubscriberHandler *handler, GrssFeedChannel *channel, FEED_SUBSCRIPTION_STATUS *status, SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query, SoupClientContext *client);
	void (*unsubscribe) (GrssFeedsSubscriberHandler *handler, GrssFeedChannel *channel, gchar *assigned_url);
};

GType			grss_feeds_subscriber_handler_get_type			();

void			grss_feeds_subscriber_handler_set_parent		(GrssFeedsSubscriberHandler *handler, GrssFeedsSubscriber *parent);
gboolean		grss_feeds_subscriber_handler_check_format		(GrssFeedsSubscriberHandler *handler, GrssFeedChannel *channel);
void			grss_feeds_subscriber_handler_subscribe			(GrssFeedsSubscriberHandler *handler, GrssFeedChannel *channel, gchar *assigned_url);
GList*			grss_feeds_subscriber_handler_handle_incoming_message	(GrssFeedsSubscriberHandler *handler, GrssFeedChannel *channel, FEED_SUBSCRIPTION_STATUS *status, SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query, SoupClientContext *client);
void			grss_feeds_subscriber_handler_unsubscribe		(GrssFeedsSubscriberHandler *handler, GrssFeedChannel *channel, gchar *assigned_url);

#endif /* __GRSS_FEEDS_SUBSCRIBER_HANDLER_H__ */
