/** 
 * xmr-mpris-plugin.c
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

/**
 * Source refer to rhythmbox plugin of mpris
 * Copyright (C) 2010  Jonathan Matthew  <jonathan@d14n.org>
 */

#include <gio/gio.h>

#include "xmrplugin.h"
#include "xmrwindow.h"
#include "xmrplayer.h"
#include "lib/songinfo.h"

#include "mpris-spec.h"

#define XMR_TYPE_MPRIS_PLUGIN			(xmr_mpris_plugin_get_type())
#define XMR_MPRIS_PLUGIN(o)				(G_TYPE_CHECK_INSTANCE_CAST((o), XMR_TYPE_MPRIS_PLUGIN, XmrMprisPlugin))
#define XMR_MPRIS_PLUGIN_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), XMR_TYPE_MPRIS_PLUGIN, XmrMprisPluginClass))
#define XMR_IS_MPRIS_PLUGIN(o)	        (G_TYPE_CHECK_INSTANCE_TYPE((o), XMR_TYPE_MPRIS_PLUGIN))
#define XMR_IS_MPRIS_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE((k), XMR_TYPE_MPRIS_PLUGIN))
#define XMR_MPRIS_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS((o),  XMR_TYPE_MPRIS_PLUGIN, XmrMprisPluginClass))

typedef struct
{
	PeasExtensionBase parent;

	XmrWindow *window;
	XmrPlayer *player;

	GDBusConnection *connection;
	GDBusNodeInfo *node_info;
	guint name_own_id;
	guint root_id;
	guint player_id;

	gint64 duration;
	gint64 elapsed;
	SongInfo *current_song;

}XmrMprisPlugin;

typedef struct
{
	PeasExtensionBaseClass parent_class;
}XmrMprisPluginClass;

XMR_DEFINE_PLUGIN(XMR_TYPE_MPRIS_PLUGIN, XmrMprisPlugin, xmr_mpris_plugin,)

static void
track_changed(XmrWindow *window,
			SongInfo *new_track,
			XmrMprisPlugin *plugin)
{
	plugin->current_song = new_track;
}

static void
player_tick(XmrPlayer	*player,
			gint64		 elapsed,
			gint64		 duration,
			XmrMprisPlugin *plugin)
{
	GError *error = NULL;

	plugin->duration = duration;
	plugin->elapsed = elapsed;

	g_dbus_connection_emit_signal(plugin->connection,
				       NULL,
				       MPRIS_OBJECT_NAME,
				       MPRIS_PLAYER_INTERFACE,
				       "Seeked",
				       g_variant_new("(x)", elapsed / 1000),
				       &error);
	if (error != NULL)
	{
		g_warning("Unable to set MPRIS Seeked signal: %s", error->message);
		g_clear_error(&error);
	}
}

static void
handle_root_method_call(GDBusConnection *connection,
			 const char *sender,
			 const char *object_path,
			 const char *interface_name,
			 const char *method_name,
			 GVariant *parameters,
			 GDBusMethodInvocation *invocation,
			 XmrMprisPlugin *plugin)
{
	XmrWindow *window = NULL;

	if (g_strcmp0(object_path, MPRIS_OBJECT_NAME) != 0 ||
	    g_strcmp0(interface_name, MPRIS_ROOT_INTERFACE) != 0)
	{
		g_dbus_method_invocation_return_error(invocation,
						       G_DBUS_ERROR,
						       G_DBUS_ERROR_NOT_SUPPORTED,
						       "Method %s.%s not supported",
						       interface_name,
						       method_name);
		return;
	}

	g_object_get(plugin, "object", &window, NULL);

	if (g_strcmp0(method_name, "Raise") == 0)
	{
		gtk_widget_show(GTK_WIDGET(window));
        gtk_window_present(GTK_WINDOW(window));

		g_dbus_method_invocation_return_value(invocation, NULL);
	}
	else if (g_strcmp0(method_name, "Quit") == 0)
	{
		xmr_window_quit(window);
		g_dbus_method_invocation_return_value(invocation, NULL);
	}
	else
	{
		g_dbus_method_invocation_return_error(invocation,
						       G_DBUS_ERROR,
						       G_DBUS_ERROR_NOT_SUPPORTED,
						       "Method %s.%s not supported",
						       interface_name,
						       method_name);
	}

	g_object_unref(window);
}

static GVariant *
get_root_property(GDBusConnection *connection,
		   const char *sender,
		   const char *object_path,
		   const char *interface_name,
		   const char *property_name,
		   GError **error,
		   XmrMprisPlugin *plugin)
{
	if (g_strcmp0(object_path, MPRIS_OBJECT_NAME) != 0 ||
	    g_strcmp0(interface_name, MPRIS_ROOT_INTERFACE) != 0)
	{
		g_set_error(error,
					G_DBUS_ERROR,
					G_DBUS_ERROR_NOT_SUPPORTED,
					"Property %s.%s not supported",
					interface_name,
					property_name);
		return NULL;
	}

	if (g_strcmp0(property_name, "CanQuit") == 0)
	{
		return g_variant_new_boolean(TRUE);
	}
	else if(g_strcmp0(property_name, "CanRaise") == 0)
	{
		return g_variant_new_boolean(TRUE);
	}
	else if(g_strcmp0(property_name, "HasTrackList") == 0)
	{
		return g_variant_new_boolean(TRUE);
	}
	else if (g_strcmp0(property_name, "Identity") == 0)
	{
		return g_variant_new_string("xmradio");
	}
	else if (g_strcmp0(property_name, "SupportedUriSchemes") == 0)
	{
		const char *fake_supported_schemes[] = { "http", NULL };
		return g_variant_new_strv(fake_supported_schemes, -1);
	}
	else if (g_strcmp0(property_name, "SupportedMimeTypes") == 0)
	{
		const char *fake_supported_mimetypes[] =
		{
			"audio/mpeg", NULL
		};
		return g_variant_new_strv(fake_supported_mimetypes, -1);
	}

	g_set_error(error,
		     G_DBUS_ERROR,
		     G_DBUS_ERROR_NOT_SUPPORTED,
		     "Property %s.%s not supported",
		     interface_name,
		     property_name);
	return NULL;
}

static const GDBusInterfaceVTable root_vtable =
{
	(GDBusInterfaceMethodCallFunc) handle_root_method_call,
	(GDBusInterfaceGetPropertyFunc) get_root_property,
	NULL
};

/* MPRIS player interface */

static void
handle_result(GDBusMethodInvocation *invocation,
			gboolean ret,
			GError *error)
{
	if (ret)
	{
		g_dbus_method_invocation_return_value(invocation, NULL);
	}
	else
	{
		if (error != NULL)
		{
			g_print("mpris: returning error: %s\n", error->message);
			g_dbus_method_invocation_return_gerror(invocation, error);
			g_error_free(error);
		}
		else
		{
			g_print("mpris: returning unknown error\n");
			g_dbus_method_invocation_return_error_literal(invocation,
						G_DBUS_ERROR,
						G_DBUS_ERROR_FAILED,
						"Unknown error");
		}
	}
}

static void
handle_player_method_call(GDBusConnection *connection,
			const char *sender,
			const char *object_path,
			const char *interface_name,
			const char *method_name,
			GVariant *parameters,
			GDBusMethodInvocation *invocation,
			XmrMprisPlugin *plugin)

{
	if(g_strcmp0(object_path, MPRIS_OBJECT_NAME) != 0 ||
	    g_strcmp0(interface_name, MPRIS_PLAYER_INTERFACE) != 0)
	{
		g_dbus_method_invocation_return_error(invocation,
						       G_DBUS_ERROR,
						       G_DBUS_ERROR_NOT_SUPPORTED,
						       "Method %s.%s not supported",
						       interface_name,
						       method_name);
		return;
	}

	if (g_strcmp0(method_name, "Next") == 0)
	{
		xmr_window_play_next(plugin->window);
		handle_result(invocation, TRUE, NULL);
	}
	else if (g_strcmp0(method_name, "Pause") == 0)
	{
		xmr_window_pause(plugin->window);
		handle_result(invocation, TRUE, NULL);
	}
	else if (g_strcmp0 (method_name, "Play") == 0)
	{
		xmr_window_play(plugin->window);
		handle_result(invocation, TRUE, NULL);
	}
	else
	{
		g_dbus_method_invocation_return_error(invocation,
					G_DBUS_ERROR,
					G_DBUS_ERROR_NOT_SUPPORTED,
					"Method %s.%s not supported",
					interface_name,
					method_name);
	}
}

static GVariant *
get_playback_status(XmrMprisPlugin *plugin)
{
	GVariant *v = NULL;

	if (xmr_player_playing(plugin->player))
	{
		v = g_variant_new_string ("Playing");
	}
	else
	{
		v = g_variant_new_string ("Paused");
	}

	return v;
}

static GVariant *
variant_for_metadata(const char *value, gboolean as_list)
{
	if (as_list)
	{
		const char *strv[] = {
			value, NULL
		};
		return g_variant_new_strv(strv, -1);
	}
	else 
	{
		return g_variant_new_string(value);
	}
}

static GVariant *
build_metadata(XmrMprisPlugin *plugin)
{
	GVariantBuilder *builder;
	GVariant *v = NULL;

	if (plugin->current_song == NULL)
		return NULL;

	builder = g_variant_builder_new (G_VARIANT_TYPE ("a{sv}"));

	g_variant_builder_add(builder, "{sv}", "mpris:trackid",
				g_variant_new ("o", "/org/mpris/MediaPlayer2/Track/track0"));

    g_variant_builder_add(builder, "{sv}", "mpris:length",
				g_variant_new("x", plugin->duration / 1000));

	if (plugin->current_song->album_name)
		g_variant_builder_add(builder, "{sv}", "xesam:album",
					g_variant_new("s", plugin->current_song->album_name));

	if (plugin->current_song->artist_name)
		g_variant_builder_add(builder, "{sv}", "xesam:artist",
					variant_for_metadata(plugin->current_song->artist_name, TRUE));

	if (plugin->current_song->song_name)
		g_variant_builder_add(builder, "{sv}", "xesam:title",
					g_variant_new("s", plugin->current_song->song_name));

	if (plugin->current_song->location)
	  g_variant_builder_add(builder, "{sv}", "xesam:url",
					g_variant_new("s", plugin->current_song->location));

	v = g_variant_builder_end (builder);
	g_variant_builder_unref (builder);

	return v;
}

static GVariant *
get_player_property(GDBusConnection *connection,
		     const char *sender,
		     const char *object_path,
		     const char *interface_name,
		     const char *property_name,
		     GError **error,
		     XmrMprisPlugin *plugin)
{
	if (g_strcmp0(object_path, MPRIS_OBJECT_NAME) != 0 ||
	    g_strcmp0(interface_name, MPRIS_PLAYER_INTERFACE) != 0)
	{
		g_set_error(error,
					G_DBUS_ERROR,
					G_DBUS_ERROR_NOT_SUPPORTED,
					"Property %s.%s not supported",
					interface_name,
					property_name);
		return NULL;
	}

	if (g_strcmp0 (property_name, "PlaybackStatus") == 0)
	{
		return get_playback_status(plugin);
	}
	else if (g_strcmp0 (property_name, "Metadata") == 0)
	{
		return build_metadata(plugin);
	}
	else if (g_strcmp0 (property_name, "Rate") == 0)
	{
		return g_variant_new_double(1.0);
	}
	else if (g_strcmp0(property_name, "Volume") == 0)
	{
		return g_variant_new_double(xmr_player_get_volume(plugin->player));
	}
	else if (g_strcmp0(property_name, "Position") == 0)
	{
		return g_variant_new_int64(plugin->elapsed / 1000);
	}
	else if (g_strcmp0 (property_name, "MinimumRate") == 0)
	{
		return g_variant_new_double (1.0);
	}
	else if (g_strcmp0 (property_name, "MaximumRate") == 0)
	{
		return g_variant_new_double (1.0);
	}
	else if (g_strcmp0 (property_name, "CanGoNext") == 0)
	{
		return g_variant_new_boolean(TRUE);
	}
	else if (g_strcmp0 (property_name, "CanGoPrevious") == 0)
	{
		return g_variant_new_boolean(FALSE);
	}
	else if (g_strcmp0 (property_name, "CanPlay") == 0)
	{
		return g_variant_new_boolean (TRUE);
	}
	else if (g_strcmp0 (property_name, "CanPause") == 0)
	{
		return g_variant_new_boolean(TRUE);
	}
	else if (g_strcmp0 (property_name, "CanSeek") == 0)
	{
		g_variant_new_boolean(FALSE);
	}
	else if (g_strcmp0(property_name, "CanControl") == 0)
	{
		return g_variant_new_boolean(TRUE);
	}

	g_set_error(error,
		     G_DBUS_ERROR,
		     G_DBUS_ERROR_NOT_SUPPORTED,
		     "Property %s.%s not supported",
		     interface_name,
		     property_name);
	return NULL;
}

static gboolean
set_player_property (GDBusConnection *connection,
		     const char *sender,
		     const char *object_path,
		     const char *interface_name,
		     const char *property_name,
		     GVariant *value,
		     GError **error,
		     XmrMprisPlugin *plugin)
{
	if (g_strcmp0 (object_path, MPRIS_OBJECT_NAME) != 0 ||
	    g_strcmp0 (interface_name, MPRIS_PLAYER_INTERFACE) != 0)
	{
		g_set_error(error,
			     G_DBUS_ERROR,
			     G_DBUS_ERROR_NOT_SUPPORTED,
			     "%s:%s not supported",
			     object_path,
			     interface_name);
		return FALSE;
	}

	if (g_strcmp0 (property_name, "Volume") == 0)
	{
		xmr_window_set_volume(plugin->window, g_variant_get_double(value));
		return TRUE;
	}

	g_set_error(error,
				G_DBUS_ERROR,
				G_DBUS_ERROR_NOT_SUPPORTED,
				"Property %s.%s not supported",
				interface_name,
				property_name);
	return FALSE;
}

static const GDBusInterfaceVTable player_vtable =
{
	(GDBusInterfaceMethodCallFunc) handle_player_method_call,
	(GDBusInterfaceGetPropertyFunc) get_player_property,
	(GDBusInterfaceSetPropertyFunc) set_player_property,
};

static void
name_acquired_cb(GDBusConnection *connection, const char *name, XmrMprisPlugin *plugin)
{
	g_print("successfully acquired dbus name %s\n", name);
}

static void
name_lost_cb (GDBusConnection *connection, const char *name, XmrMprisPlugin *plugin)
{
	g_print("lost dbus name %s\n", name);
}

static void
impl_activate(PeasActivatable *activatable)
{
	XmrMprisPlugin *plugin;
	GError *error = NULL;

	GDBusInterfaceInfo *ifaceinfo;

	plugin = XMR_MPRIS_PLUGIN(activatable);
	g_object_get(plugin, "object", &plugin->window, NULL);
	g_object_get(plugin->window, "player", &plugin->player, NULL);

	g_signal_connect(plugin->window, "track-changed",
				G_CALLBACK(track_changed), plugin);
	g_signal_connect(plugin->player, "tick", G_CALLBACK(player_tick), plugin);

	plugin->connection = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
	if (error != NULL)
	{
		g_warning("Unable to connect to D-Bus session bus: %s", error->message);
		return ;
	}

	/* parse introspection data */
	plugin->node_info = g_dbus_node_info_new_for_xml(mpris_introspection_xml, &error);
	if (error != NULL)
	{
		g_warning("Unable to read MPRIS interface specificiation: %s", error->message);
		return;
	}

	/* register root interface */
	ifaceinfo = g_dbus_node_info_lookup_interface (plugin->node_info, MPRIS_ROOT_INTERFACE);
	plugin->root_id = g_dbus_connection_register_object(plugin->connection,
							     MPRIS_OBJECT_NAME,
							     ifaceinfo,
							     &root_vtable,
							     plugin,
							     NULL,
							     &error);
	if (error != NULL)
	{
		g_warning ("unable to register MPRIS root interface: %s", error->message);
		g_error_free (error);
	}

	/* register player interface */
	ifaceinfo = g_dbus_node_info_lookup_interface(plugin->node_info, MPRIS_PLAYER_INTERFACE);
	plugin->player_id = g_dbus_connection_register_object(plugin->connection,
							       MPRIS_OBJECT_NAME,
							       ifaceinfo,
							       &player_vtable,
							       plugin,
							       NULL,
							       &error);
	if (error != NULL)
	{
		g_warning ("Unable to register MPRIS player interface: %s", error->message);
		g_error_free (error);
	}

	plugin->name_own_id = g_bus_own_name(G_BUS_TYPE_SESSION,
					      MPRIS_BUS_NAME_PREFIX ".xmradio",
					      G_BUS_NAME_OWNER_FLAGS_NONE,
					      NULL,
					      (GBusNameAcquiredCallback) name_acquired_cb,
					      (GBusNameLostCallback) name_lost_cb,
					      g_object_ref(plugin),
					      g_object_unref);
}

static void
impl_deactivate(PeasActivatable *activatable)
{
	XmrMprisPlugin *plugin;

	plugin = XMR_MPRIS_PLUGIN(activatable);

	g_signal_handlers_disconnect_by_func(plugin->window,
					track_changed, plugin);

	g_signal_handlers_disconnect_by_func(plugin->player,
				player_tick, plugin);

	if (plugin->root_id != 0)
	{
		g_dbus_connection_unregister_object(plugin->connection, plugin->root_id);
		plugin->root_id = 0;
	}
	if (plugin->player_id != 0)
	{
		g_dbus_connection_unregister_object(plugin->connection, plugin->player_id);
		plugin->player_id = 0;
	}
	if (plugin->name_own_id > 0) {
		g_bus_unown_name(plugin->name_own_id);
		plugin->name_own_id = 0;
	}
	if (plugin->connection != NULL)
	{
		g_object_unref(plugin->connection);
		plugin->connection = NULL;
	}

	g_object_unref(plugin->window);
	g_object_unref(plugin->player);
}

static void
xmr_mpris_plugin_init(XmrMprisPlugin *plugin)
{
	plugin->current_song = NULL;
}

G_MODULE_EXPORT void
peas_register_types(PeasObjectModule *module)
{
	xmr_mpris_plugin_register_type(G_TYPE_MODULE(module));

	peas_object_module_register_extension_type(module,
						    PEAS_TYPE_ACTIVATABLE,
						    XMR_TYPE_MPRIS_PLUGIN);
}
