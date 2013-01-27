/** 
 * xmr-indicator-plugin.c
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
#include <glib/gi18n.h>
#include <libpeas-gtk/peas-gtk.h>

#include "xmrplugin.h"
#include "xmrwindow.h"
#include "xmrplayer.h"

#ifdef HAVE_APP_INDICATOR
#include "xmr-app-indicator.h"
#else
#include "xmr-tray-icon.h"
#endif

#define XMR_TYPE_INDICATOR_PLUGIN			(xmr_indicator_plugin_get_type())
#define XMR_INDICATOR_PLUGIN(o)				(G_TYPE_CHECK_INSTANCE_CAST((o), XMR_TYPE_INDICATOR_PLUGIN, XmrIndicatorPlugin))
#define XMR_INDICATOR_PLUGIN_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), XMR_TYPE_INDICATOR_PLUGIN, XmrIndicatorPluginClass))
#define XMR_IS_INDICATOR_PLUGIN(o)	        (G_TYPE_CHECK_INSTANCE_TYPE((o), XMR_TYPE_INDICATOR_PLUGIN))
#define XMR_IS_INDICATOR_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE((k), XMR_TYPE_INDICATOR_PLUGIN))
#define XMR_INDICATOR_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS((o),  XMR_TYPE_INDICATOR_PLUGIN, XmrIndicatorPluginClass))

typedef struct
{
	PeasExtensionBase parent;

	XmrWindow *window;

	GSettings *settings;
	gboolean hide_on_exit;
#ifdef HAVE_APP_INDICATOR
	XmrAppIndicator *indicator;
#else
	GtkWidget *indicator;
#endif
	GtkWidget *popup_menu;

	GtkWidget *menu_item_play;
	GtkWidget *menu_item_pause;

}XmrIndicatorPlugin;

typedef struct
{
	PeasExtensionBaseClass parent_class;
}XmrIndicatorPluginClass;

static GtkWidget *
impl_create_configure_widget(PeasGtkConfigurable *plugin);

static void
peas_gtk_configurable_iface_init(PeasGtkConfigurableInterface *iface);

XMR_DEFINE_PLUGIN(XMR_TYPE_INDICATOR_PLUGIN,
			XmrIndicatorPlugin,
			xmr_indicator_plugin,
			(G_IMPLEMENT_INTERFACE_DYNAMIC(PEAS_GTK_TYPE_CONFIGURABLE,
						peas_gtk_configurable_iface_init))
			)


static gboolean
on_xmr_window_delete(XmrWindow *window,
			GdkEvent  *event,
			XmrIndicatorPlugin *plugin)
{
	if (plugin->hide_on_exit)
	{
		return gtk_widget_hide_on_delete(GTK_WIDGET(window));
	}

	return FALSE;
}

static void
on_menu_item_activate(GtkMenuItem *item,
			XmrIndicatorPlugin *plugin)
{
	const gchar *menu = gtk_menu_item_get_label(item);
	
	if(g_strcmp0(menu, GTK_STOCK_QUIT) == 0)
	{
		xmr_window_quit(plugin->window);
	}
	else if(g_strcmp0(menu, GTK_STOCK_MEDIA_PLAY) == 0)
	{
		xmr_window_play(plugin->window);
		gtk_widget_hide(GTK_WIDGET(item));
		gtk_widget_show(plugin->menu_item_pause);
	}
	else if(g_strcmp0(menu, GTK_STOCK_MEDIA_PAUSE) == 0)
	{
		xmr_window_pause(plugin->window);
		gtk_widget_hide(GTK_WIDGET(item));
		gtk_widget_show(plugin->menu_item_play);
	}
	else if(g_strcmp0(menu, GTK_STOCK_MEDIA_NEXT) == 0)
	{
		xmr_window_play_next(plugin->window);
	}
	else if(g_strcmp0(menu, _("Show")) == 0)
	{
		gtk_widget_show(GTK_WIDGET(plugin->window));
        gtk_window_present(GTK_WINDOW(plugin->window));
	}
	else if(g_strcmp0(menu, _("Love")) == 0)
	{
		xmr_window_love(plugin->window);
	}
	else if(g_strcmp0(menu, _("Hate")) == 0)
	{
		xmr_window_hate(plugin->window);
	}
}

static void
player_state_changed(XmrPlayer *player,
			gint old_state,
			gint new_state,
			XmrIndicatorPlugin *plugin)
{
	if (new_state == GST_STATE_PLAYING)
	{
		gtk_widget_hide(plugin->menu_item_play);
		gtk_widget_show(plugin->menu_item_pause);
	}
	else if(new_state == GST_STATE_PAUSED)
	{
		gtk_widget_hide(plugin->menu_item_pause);
		gtk_widget_show(plugin->menu_item_play);
	}
}

static GtkWidget *
impl_create_configure_widget(PeasGtkConfigurable *plugin)
{
	GtkWidget *box = NULL;
	GtkWidget *widget;
	XmrIndicatorPlugin *indicator = XMR_INDICATOR_PLUGIN(plugin);

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	widget = gtk_check_button_new_with_label(_("Hide on exit when playing"));
	gtk_box_pack_start(GTK_BOX(box), widget, TRUE, FALSE, 0);

	g_settings_bind(indicator->settings,
				"hide-on-exit",
				widget, "active",
				G_SETTINGS_BIND_DEFAULT);

	return box;
}

static void
peas_gtk_configurable_iface_init(PeasGtkConfigurableInterface *iface)
{
	iface->create_configure_widget = impl_create_configure_widget;
}

static void
on_settings_changed(GSettings *settings,
					const char *key,
					XmrIndicatorPlugin *plugin)
{
	if (g_strcmp0(key, "hide-on-exit") == 0){
		plugin->hide_on_exit = g_settings_get_boolean(settings, key);
	}
}

static void
track_changed(XmrWindow *window,
			SongInfo *new_track,
			XmrIndicatorPlugin *plugin)
{
	gchar *tooltips;

	tooltips = g_strdup_printf("%s - %s",
				new_track->artist_name,
				new_track->song_name);

#ifdef HAVE_APP_INDICATOR
//	app_indicator_set_title(APP_INDICATOR(plugin->indicator), tooltips);
#else
	xmr_tray_icon_set_tooltips(XMR_TRAY_ICON(plugin->indicator), tooltips);
#endif

	g_free(tooltips);
}

static void
impl_activate(PeasActivatable *activatable)
{
	XmrIndicatorPlugin *plugin;
	XmrWindow *window = NULL;
	XmrPlayer *player;

	plugin = XMR_INDICATOR_PLUGIN(activatable);
	g_object_get(plugin, "object", &window, NULL);

	if (window)
	{
		plugin->window = window;
		if (!plugin->indicator)
		{
#if HAVE_APP_INDICATOR
			plugin->indicator = xmr_app_indicator_new(plugin->popup_menu);
#else
			plugin->indicator = xmr_tray_icon_new(GTK_WIDGET(window), plugin->popup_menu);
#endif
		}
		g_signal_connect(window, "delete-event",
				G_CALLBACK(on_xmr_window_delete), plugin);

		g_signal_connect(window, "track-changed",
				G_CALLBACK(track_changed), plugin);
		
		g_object_get(window, "player", &player, NULL);
		if (player)
		{
			g_signal_connect(player, "state-changed", G_CALLBACK(player_state_changed), plugin);
			g_object_unref(player);
		}
	}

	plugin->hide_on_exit  = g_settings_get_boolean(plugin->settings, "hide-on-exit");
	g_signal_connect(plugin->settings,
				"changed",
				G_CALLBACK(on_settings_changed), plugin);

	if (xmr_window_playing(plugin->window)){
		gtk_widget_hide(plugin->menu_item_play);
	} else {
		gtk_widget_hide(plugin->menu_item_pause);
	}
}

static void
impl_deactivate(PeasActivatable *activatable)
{
	XmrIndicatorPlugin *plugin;
	XmrWindow *window = NULL;
	XmrPlayer *player = NULL;

	plugin = XMR_INDICATOR_PLUGIN(activatable);

	if (window)
	{
		g_object_get(window, "player", &player, NULL);
		g_signal_handlers_disconnect_by_func(player,
					player_state_changed, plugin);
		g_object_unref(player);

		g_signal_handlers_disconnect_by_func(plugin->window,
					on_xmr_window_delete, plugin);

		g_signal_handlers_disconnect_by_func(window,
					track_changed, plugin);

		g_object_unref(plugin->window);
		plugin->window = NULL;
	}

	if (plugin->settings != NULL)
	{
		g_object_unref(plugin->settings);
		plugin->settings = NULL;
	}

	if (plugin->indicator)
	{
		g_object_unref(plugin->indicator);
		plugin->indicator = NULL;
	}
}

static void
xmr_indicator_plugin_init(XmrIndicatorPlugin *plugin)
{
	GtkWidget *item;

	plugin->settings = g_settings_new("com.timxx.xmradio.plugins.indicator");

	plugin->hide_on_exit  = g_settings_get_boolean(plugin->settings, "hide-on-exit");

	plugin->indicator = NULL;
	plugin->popup_menu = gtk_menu_new();

	item = gtk_image_menu_item_new_with_label(_("Show"));
	gtk_menu_shell_append(GTK_MENU_SHELL(plugin->popup_menu), item);
	g_signal_connect(item, "activate", G_CALLBACK(on_menu_item_activate), plugin);

	item = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(plugin->popup_menu), item);

	item = gtk_image_menu_item_new_from_stock(GTK_STOCK_MEDIA_PLAY, NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(plugin->popup_menu), item);
	g_signal_connect(item, "activate", G_CALLBACK(on_menu_item_activate), plugin);
	gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(item), TRUE);
	plugin->menu_item_play = item;

	item = gtk_image_menu_item_new_from_stock(GTK_STOCK_MEDIA_PAUSE, NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(plugin->popup_menu), item);
	g_signal_connect(item, "activate", G_CALLBACK(on_menu_item_activate), plugin);
	gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(item), TRUE);
	plugin->menu_item_pause = item;

	item = gtk_image_menu_item_new_from_stock(GTK_STOCK_MEDIA_NEXT, NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(plugin->popup_menu), item);
	g_signal_connect(item, "activate", G_CALLBACK(on_menu_item_activate), plugin);
	gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(item), TRUE);

	item = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(plugin->popup_menu), item);

	item = gtk_image_menu_item_new_with_label(_("Love"));
	gtk_menu_shell_append(GTK_MENU_SHELL(plugin->popup_menu), item);
	g_signal_connect(item, "activate", G_CALLBACK(on_menu_item_activate), plugin);

	item = gtk_image_menu_item_new_with_label(_("Hate"));
	gtk_menu_shell_append(GTK_MENU_SHELL(plugin->popup_menu), item);
	g_signal_connect(item, "activate", G_CALLBACK(on_menu_item_activate), plugin);

	item = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(plugin->popup_menu), item);

	item = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(plugin->popup_menu), item);
	g_signal_connect(item, "activate", G_CALLBACK(on_menu_item_activate), plugin);

	gtk_widget_show_all(plugin->popup_menu);
}

G_MODULE_EXPORT void
peas_register_types(PeasObjectModule *module)
{
	xmr_indicator_plugin_register_type(G_TYPE_MODULE(module));

	peas_object_module_register_extension_type(module,
						    PEAS_TYPE_ACTIVATABLE,
						    XMR_TYPE_INDICATOR_PLUGIN);

	peas_object_module_register_extension_type(module,
						    PEAS_GTK_TYPE_CONFIGURABLE,
						    XMR_TYPE_INDICATOR_PLUGIN);
}
