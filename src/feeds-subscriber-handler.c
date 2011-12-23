/*
 * Copyright (C) 2011, Roberto Guido <rguido@src.gnome.org>
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
#include "feeds-subscriber-handler.h"

static void
grss_feeds_subscriber_handler_base_init (gpointer g_class)
{
}

GType
grss_feeds_subscriber_handler_get_type ()
{
	static GType iface_type = 0;

	if (iface_type == 0) {
		static const GTypeInfo info = {
			sizeof (GrssFeedsSubscriberHandlerInterface),
			grss_feeds_subscriber_handler_base_init,
			NULL,
		};

		iface_type = g_type_register_static (G_TYPE_INTERFACE, "GrssFeedsSubscriberHandler", &info, 0);
	}

	return iface_type;
}

void
grss_feeds_subscriber_handler_set_parent (GrssFeedsSubscriberHandler *handler,
                                          GrssFeedsSubscriber *parent)
{
	if (GRSS_IS_FEEDS_SUBSCRIBER_HANDLER (handler) == FALSE)
		return;

	return GRSS_FEEDS_SUBSCRIBER_HANDLER_GET_INTERFACE (handler)->set_parent (handler, parent);
}

gboolean
grss_feeds_subscriber_handler_check_format (GrssFeedsSubscriberHandler *handler,
                                            GrssFeedChannel *channel)
{
	if (GRSS_IS_FEEDS_SUBSCRIBER_HANDLER (handler) == FALSE)
		return FALSE;

	return GRSS_FEEDS_SUBSCRIBER_HANDLER_GET_INTERFACE (handler)->check_format (handler, channel);
}

void
grss_feeds_subscriber_handler_subscribe (GrssFeedsSubscriberHandler *handler,
                                         GrssFeedChannel *channel,
                                         gchar *assigned_url)
{
	if (GRSS_IS_FEEDS_SUBSCRIBER_HANDLER (handler) == FALSE)
		return;

	GRSS_FEEDS_SUBSCRIBER_HANDLER_GET_INTERFACE (handler)->subscribe (handler, channel, assigned_url);
}

GList*
grss_feeds_subscriber_handler_handle_incoming_message (GrssFeedsSubscriberHandler *handler,
                                                       GrssFeedChannel *channel,
                                                       FEED_SUBSCRIPTION_STATUS *status,
						       SoupServer *server,
                                                       SoupMessage *msg,
                                                       const char *path,
                                                       GHashTable *query,
                                                       SoupClientContext *client)
{
	if (GRSS_IS_FEEDS_SUBSCRIBER_HANDLER (handler) == FALSE)
		return NULL;

	return GRSS_FEEDS_SUBSCRIBER_HANDLER_GET_INTERFACE (handler)->handle_message (handler, channel, status, server, msg, path, query, client);
}

void
grss_feeds_subscriber_handler_unsubscribe (GrssFeedsSubscriberHandler *handler,
                                           GrssFeedChannel *channel,
                                           gchar *assigned_url)
{
	if (GRSS_IS_FEEDS_SUBSCRIBER_HANDLER (handler) == FALSE)
		return;

	GRSS_FEEDS_SUBSCRIBER_HANDLER_GET_INTERFACE (handler)->unsubscribe (handler, channel, assigned_url);
}
