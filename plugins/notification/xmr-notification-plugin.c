/** 
 * xmr-notification-plugin.c
 *
 * Copyright (C) 2012-2013, 2015  Weitian Leung (weitianleung@gmail.com)

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
#define XMR_NOTIFICATION_PLUGIN_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), XMR_TYPE_NOTIFICATION_PLUGIN, XmrNotificationPluginClass))
#define XMR_IS_NOTIFICATION_PLUGIN(o)	        (G_TYPE_CHECK_INSTANCE_TYPE((o), XMR_TYPE_NOTIFICATION_PLUGIN))
#define XMR_IS_NOTIFICATION_PLUGIN_CLASS(k)		(G_TYPE_CHECK_CLASS_TYPE((k), XMR_TYPE_NOTIFICATION_PLUGIN))
#define XMR_NOTIFICATION_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS((o),  XMR_TYPE_NOTIFICATION_PLUGIN, XmrNotificationPluginClass))

typedef struct
{
	PeasExtensionBase parent;

	NotifyNotification *notification;

	gchar *summary;
	gchar *body;
	gboolean have_cover;

}XmrNotificationPlugin;

typedef struct
{
	PeasExtensionBaseClass parent_class;
}XmrNotificationPluginClass;

XMR_DEFINE_PLUGIN(XMR_TYPE_NOTIFICATION_PLUGIN, XmrNotificationPlugin, xmr_notification_plugin,)

#define NOTIFY_TIMEOUT 3000

static gboolean
delay_show_notify(XmrNotificationPlugin *plugin)
{
	notify_notification_update(plugin->notification,
							   plugin->summary, plugin->body,
							   plugin->have_cover ? NULL : "xmradio");
	notify_notification_show(plugin->notification, NULL);
	return FALSE;
}

static void
track_notification(XmrNotificationPlugin *plugin,
			SongInfo *info,
			gint timeout)
{
	g_return_if_fail(info != NULL && plugin->notification != NULL);

	g_free(plugin->summary);
	g_free(plugin->body);

	plugin->summary = g_strdup(info->song_name);
	plugin->body = g_strdup(info->artist_name);

	notify_notification_clear_hints(plugin->notification);
	notify_notification_set_timeout(plugin->notification, timeout);

	// wait 500ms for cover image
	g_timeout_add(500, (GSourceFunc)delay_show_notify, plugin);
}

static void
track_changed(XmrWindow *window,
			SongInfo *new_track,
			XmrNotificationPlugin *plugin)
{
	plugin->have_cover = FALSE;
	track_notification(plugin, new_track, NOTIFY_TIMEOUT);
}

static void
cover_image_changed(XmrWindow *window,
					GdkPixbuf *pixbuf,
					XmrNotificationPlugin *plugin)
{
	g_return_if_fail(plugin->notification != NULL);

	plugin->have_cover = TRUE;
	notify_notification_set_image_from_pixbuf(plugin->notification, pixbuf);
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
		g_signal_connect(window, "track-changed", G_CALLBACK(track_changed), plugin);
		g_signal_connect(window, "fetch-cover-finish", G_CALLBACK(cover_image_changed), plugin);

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
				G_CALLBACK(track_changed), plugin);

		g_signal_handlers_disconnect_by_func(window,
				G_CALLBACK(cover_image_changed), plugin);

		g_object_unref(window);
	}

	g_free(plugin->summary);
	g_free(plugin->body);
	if (plugin->notification)
		notify_notification_close(plugin->notification, NULL);
}

static void
xmr_notification_plugin_init (XmrNotificationPlugin *plugin)
{
	notify_init("xmrario-notification-plugin");

	plugin->notification = notify_notification_new("", "", "xmradio");
	plugin->summary = NULL;
	plugin->body = NULL;
	plugin->have_cover = FALSE;
}

G_MODULE_EXPORT void
peas_register_types(PeasObjectModule *module)
{
	xmr_notification_plugin_register_type(G_TYPE_MODULE(module));
	peas_object_module_register_extension_type (module,
						    PEAS_TYPE_ACTIVATABLE,
						    XMR_TYPE_NOTIFICATION_PLUGIN);
}
