/** 
 * notification.c
 *
 * Copyright (C) 2012  Weitian Leung (weitianleung@gmail.com)

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>
#include <libnotify/notify.h>

#include "xmrplugin.h"
#include "xmrwindow.h"
#include "lib/songinfo.h"

#define XMR_TYPE_NOTIFICATION_PLUGIN			(xmr_notification_plugin_get_type())
#define XMR_NOTIFICATION_PLUGIN(o)				(G_TYPE_CHECK_INSTANCE_CAST((o), XMR_TYPE_NOTIFICATION_PLUGIN, XmrNotificationPlugin))
#define XMR_NOTIFICATION_PLUGIN_CLASS(k)		G_TYPE_CHECK_CLASS_CAST((k), XMR_TYPE_NOTIFICATION_PLUGIN, XmrNotificationPluginClass))
#define XMR_IS_NOTIFICATION_PLUGIN(o)	        (G_TYPE_CHECK_INSTANCE_TYPE((o), XMR_TYPE_NOTIFICATION_PLUGIN))
#define XMR_IS_NOTIFICATION_PLUGIN_CLASS(k)		(G_TYPE_CHECK_CLASS_TYPE((k), XMR_TYPE_NOTIFICATION_PLUGIN))
#define XMR_NOTIFICATION_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS((o),  XMR_TYPE_NOTIFICATION_PLUGIN, XmrNotificationPluginClass))

typedef struct
{
	PeasExtensionBase parent;

}XmrNotificationPlugin;

typedef struct
{
	PeasExtensionBaseClass parent_class;
}XmrNotificationPluginClass;

XMR_DEFINE_PLUGIN(XMR_TYPE_NOTIFICATION_PLUGIN, XmrNotificationPlugin, xmr_notification_plugin)

static void
track_notification(SongInfo *info,
			gint timeout)
{
	NotifyNotification *notification;
	gchar *summary, *body;

	g_return_if_fail(info != NULL);

	summary = info->song_name;
	body = info->artist_name;

	notification = notify_notification_new(summary, body, "xmradio");
	notify_notification_set_timeout(notification, timeout);

	notify_notification_show(notification, NULL);

	g_object_unref(notification);
}

static void
track_changed(XmrWindow *window,
			SongInfo *new_track,
			XmrNotificationPlugin *plugin)
{
	track_notification(new_track, NOTIFY_EXPIRES_DEFAULT);
}

static void
impl_activate(PeasActivatable *activatable)
{
	XmrNotificationPlugin *plugin;
	XmrWindow *window = NULL;

	plugin = XMR_NOTIFICATION_PLUGIN(activatable);
	g_object_get(plugin, "object", &window, NULL);

	if (window)
	{
		g_signal_connect(window, "track-changed",
				G_CALLBACK(track_changed), plugin);

		g_object_unref(window);
	}
}

static void
impl_deactivate(PeasActivatable *activatable)
{
	XmrNotificationPlugin *plugin;
	XmrWindow *window = NULL;

	plugin = XMR_NOTIFICATION_PLUGIN(activatable);
	g_object_get(plugin, "object", &window, NULL);

	if (window)
	{
		g_signal_handlers_disconnect_by_func(window,
					track_changed, plugin);

		g_object_unref(window);
	}
}

static void
xmr_notification_plugin_init (XmrNotificationPlugin *plugin)
{
	notify_init("xmrario-notification-plugin");
}

G_MODULE_EXPORT void
peas_register_types(PeasObjectModule *module)
{
	xmr_notification_plugin_register_type(G_TYPE_MODULE(module));
	peas_object_module_register_extension_type (module,
						    PEAS_TYPE_ACTIVATABLE,
						    XMR_TYPE_NOTIFICATION_PLUGIN);
}
