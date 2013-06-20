/** 
 * xmr-auto-collect-plugin.c
 *
 * Copyright (C) 2013  Weitian Leung (weitianleung@gmail.com)

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
#include <libpeas-gtk/peas-gtk.h>

#include "xmrplugin.h"
#include "xmrwindow.h"
#include "lib/xmrservice.h"
#include "lib/songinfo.h"
#include "xmrplayer.h"
#include "config.h"
#include "xmrdebug.h"
#include "xmrutil.h"

#define XMR_TYPE_AUTOCOLLECT_PLUGIN			(xmr_autocollect_plugin_get_type())
#define XMR_AUTOCOLLECT_PLUGIN(o)			(G_TYPE_CHECK_INSTANCE_CAST((o), XMR_TYPE_AUTOCOLLECT_PLUGIN, XmrAutoCollectPlugin))
#define XMR_AUTOCOLLECT_PLUGIN_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), XMR_TYPE_AUTOCOLLECT_PLUGIN, XmrAutoCollectPluginClass))
#define XMR_IS_AUTOCOLLECT_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_TYPE((o), XMR_TYPE_AUTOCOLLECT_PLUGIN))
#define XMR_IS_AUTOCOLLECT_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE((k), XMR_TYPE_AUTOCOLLECT_PLUGIN))
#define XMR_AUTOCOLLECT_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS((o),  XMR_TYPE_AUTOCOLLECT_PLUGIN, XmrAutoCollectPluginClass))

typedef struct
{
	PeasExtensionBase parent;

	gint like_percent;
	gint hate_percent;
	gboolean auto_like;
	gboolean auto_hate;
	
	gboolean already_set;
	gint play_percent;
	
	GSettings *settings;
	GtkBuilder *builder;
	
	gint64 last_duration;
}XmrAutoCollectPlugin;

typedef struct
{
	PeasExtensionBaseClass parent_class;
}XmrAutoCollectPluginClass;

static void
peas_gtk_configurable_iface_init(PeasGtkConfigurableInterface *iface);

XMR_DEFINE_PLUGIN(XMR_TYPE_AUTOCOLLECT_PLUGIN,
			XmrAutoCollectPlugin,
			xmr_autocollect_plugin,
			(G_IMPLEMENT_INTERFACE_DYNAMIC(PEAS_GTK_TYPE_CONFIGURABLE,
						peas_gtk_configurable_iface_init))
			)

static void
on_auto_like_toggled(GtkToggleButton *button,
					 GtkBuilder *builder)
{
	gboolean active = gtk_toggle_button_get_active(button);
	GObject *obj = gtk_builder_get_object(builder, "like_percent");
	gtk_widget_set_sensitive(GTK_WIDGET(obj), active);
}

static void
on_auto_hate_toggled(GtkToggleButton *button,
					 GtkBuilder *builder)
{
	gboolean active = gtk_toggle_button_get_active(button);
	GObject *obj = gtk_builder_get_object(builder, "hate_percent");
	gtk_widget_set_sensitive(GTK_WIDGET(obj), active);
}

static GtkWidget *
impl_create_configure_widget(PeasGtkConfigurable *cfg)
{
	GObject *obj;
	gchar *filePath;
	XmrAutoCollectPlugin *plugin = XMR_AUTOCOLLECT_PLUGIN(cfg);
	
	plugin->builder = gtk_builder_new();
	filePath = g_build_filename(xmr_app_dir(), "plugins/xmr-auto-collect.ui", NULL);

	if (gtk_builder_add_from_file(plugin->builder, filePath, NULL) == 0)
		gtk_builder_add_from_file(plugin->builder, PLUGIN_DATA_DIR "/autocollect/xmr-auto-collect.ui", NULL);
	
	g_free(filePath);

	obj = gtk_builder_get_object(plugin->builder, "auto_like");
	g_settings_bind(plugin->settings, "auto-like", obj, "active", G_SETTINGS_BIND_DEFAULT);
	
	obj = gtk_builder_get_object(plugin->builder, "auto_hate");
	g_settings_bind(plugin->settings, "auto-hate", obj, "active", G_SETTINGS_BIND_DEFAULT);
	
	obj = gtk_builder_get_object(plugin->builder, "adjustment1");
	g_settings_bind(plugin->settings, "like-percent", obj, "value", G_SETTINGS_BIND_DEFAULT);
	
	obj = gtk_builder_get_object(plugin->builder, "adjustment2");
	g_settings_bind(plugin->settings, "hate-percent", obj, "value", G_SETTINGS_BIND_DEFAULT);
	
	obj = gtk_builder_get_object(plugin->builder, "auto_like");	
	on_auto_like_toggled(GTK_TOGGLE_BUTTON(obj), plugin->builder);
	g_signal_connect(obj, "toggled", G_CALLBACK(on_auto_like_toggled), plugin->builder);

	obj = gtk_builder_get_object(plugin->builder, "auto_hate");
	on_auto_hate_toggled(GTK_TOGGLE_BUTTON(obj), plugin->builder);
	g_signal_connect(obj, "toggled", G_CALLBACK(on_auto_hate_toggled), plugin->builder);

	return GTK_WIDGET(gtk_builder_get_object(plugin->builder, "main_layout"));
}

static void
peas_gtk_configurable_iface_init(PeasGtkConfigurableInterface *iface)
{
	iface->create_configure_widget = impl_create_configure_widget;
}

static void
on_settings_changed(GSettings *settings,
					const char *key,
					XmrAutoCollectPlugin *plugin)
{
	if (g_strcmp0(key, "auto-like") == 0) {
		plugin->auto_like = g_settings_get_boolean(settings, key);
	} else if (g_strcmp0(key, "auto-hate") == 0) {
		plugin->auto_hate = g_settings_get_boolean(settings, key);
	} else if (g_strcmp0(key, "like-percent") == 0) {
		plugin->like_percent = g_settings_get_int(settings, key);
	} else if (g_strcmp0(key, "hate-percent") == 0) {
		plugin->hate_percent = g_settings_get_int(settings, key);
	}
}

static void
track_changed(XmrWindow *window,
			SongInfo *new_track,
			XmrAutoCollectPlugin *plugin)
{
	plugin->already_set = FALSE;
	plugin->play_percent = 0;
	plugin->last_duration = 0;
}

static void
player_tick(XmrPlayer *player,
			gint64 elapsed,
			gint64 duration,
			XmrAutoCollectPlugin *plugin)
{
	// avoid miss setting, due to the buffering size
	if (plugin->last_duration != duration)
	{
		plugin->last_duration = duration;
		return ;
	}

	if (duration > 0 && !plugin->already_set && (plugin->auto_like || plugin->auto_hate))
	{
		plugin->play_percent = elapsed * 100 / duration;
		
		if ((plugin->auto_like && plugin->play_percent >= plugin->like_percent) ||
			(plugin->auto_hate && plugin->play_percent < plugin->hate_percent))
		{
			XmrService *service = NULL;
			XmrWindow *window = NULL;
			g_object_get(plugin, "object", &window, NULL);
			g_object_get(window, "service", &service, NULL);
			if (!xmr_service_is_logged_in(service))
			{
				xmr_debug("Please sign in before use this plugin");
			}
			else
			{
				if (plugin->auto_like && plugin->play_percent >= plugin->like_percent)
					if (!xmr_window_is_current_song_marked(window))
						xmr_window_love(window);
			
				if (plugin->auto_hate && plugin->play_percent < plugin->hate_percent)
					xmr_window_hate(window);
				
				plugin->already_set = TRUE;
			}
			
			g_object_unref(service);
			g_object_unref(window);
		}
	}
}

static void
impl_activate(PeasActivatable *activatable)
{
	XmrAutoCollectPlugin *plugin;
	XmrWindow *window = NULL;

	plugin = XMR_AUTOCOLLECT_PLUGIN(activatable);
	g_object_get(plugin, "object", &window, NULL);

	if (window)
	{
		XmrPlayer *player = NULL;

		g_signal_connect(window, "track-changed", G_CALLBACK(track_changed), plugin);

		g_object_get(window, "player", &player, NULL);
		g_signal_connect(player, "tick", G_CALLBACK(player_tick), plugin);

		g_object_unref(window);
		g_object_unref(player);
	}
	
	g_signal_connect(plugin->settings, "changed", G_CALLBACK(on_settings_changed), plugin);
	
	plugin->like_percent = g_settings_get_int(plugin->settings, "like-percent");
	plugin->hate_percent = g_settings_get_int(plugin->settings, "hate-percent");
	
	plugin->auto_like = g_settings_get_boolean(plugin->settings, "auto-like");
	plugin->auto_hate = g_settings_get_boolean(plugin->settings, "auto-hate");

	plugin->already_set = FALSE;
	plugin->play_percent = 0;
	plugin->last_duration = 0;
}

static void
impl_deactivate(PeasActivatable *activatable)
{
	XmrAutoCollectPlugin *plugin;
	XmrWindow *window = NULL;

	plugin = XMR_AUTOCOLLECT_PLUGIN(activatable);
	g_object_get(plugin, "object", &window, NULL);

	if (window)
	{
		XmrPlayer *player = NULL;
		g_signal_handlers_disconnect_by_func(window, track_changed, plugin);
		
		g_object_get(window, "player", &player, NULL);
		g_signal_handlers_disconnect_by_func(player, player_tick, plugin);

		g_object_unref(window);
		g_object_unref(player);
	}
	
	if (plugin->settings != NULL)
	{
		g_signal_handlers_disconnect_by_func(plugin->settings, G_CALLBACK(on_settings_changed), plugin);
		g_object_unref(plugin->settings);
		plugin->settings = NULL;
	}
	
	if (plugin->builder)
	{
		g_object_unref(plugin->builder);
		plugin->builder = NULL;
	}
}

static void
xmr_autocollect_plugin_init(XmrAutoCollectPlugin *plugin)
{
	plugin->settings = g_settings_new("com.timxx.xmradio.plugins.autocollect");
}

G_MODULE_EXPORT void
peas_register_types(PeasObjectModule *module)
{
	xmr_autocollect_plugin_register_type(G_TYPE_MODULE(module));
	peas_object_module_register_extension_type(module,
						    PEAS_TYPE_ACTIVATABLE,
						    XMR_TYPE_AUTOCOLLECT_PLUGIN);
	
	peas_object_module_register_extension_type(module,
						    PEAS_GTK_TYPE_CONFIGURABLE,
						    XMR_TYPE_AUTOCOLLECT_PLUGIN);
}
