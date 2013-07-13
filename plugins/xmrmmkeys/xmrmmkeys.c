/** 
 * xmrmmkeys.c
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

/**
  * source refer to Rhythmbox's mmkeys plugin
  */

#include <glib.h>

#include "xmrplugin.h"
#include "xmrwindow.h"
#include "xmrplayer.h"
#include "xmrdebug.h"
#include "config.h"

#ifdef HAVE_MMKEYS_H
#include <X11/XF86keysym.h>
#include <gdk/gdkx.h>
#endif

#define XMR_TYPE_MMKEYS_PLUGIN			(xmr_mmkeys_plugin_get_type())
#define XMR_MMKEYS_PLUGIN(o)			(G_TYPE_CHECK_INSTANCE_CAST((o), XMR_TYPE_MMKEYS_PLUGIN, XmrMMKeysPlugin))
#define XMR_MMKEYS_PLUGIN_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), XMR_TYPE_MMKEYS_PLUGIN, XmrMMKeysPluginClass))
#define XMR_IS_MMKEYS_PLUGIN(o)			(G_TYPE_CHECK_INSTANCE_TYPE((o), XMR_TYPE_MMKEYS_PLUGIN))
#define XMR_IS_MMKEYS_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE((k), XMR_TYPE_MMKEYS_PLUGIN))
#define XMR_MMKEYS_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS((o),  XMR_TYPE_MMKEYS_PLUGIN, XmrMMKeysPluginClass))

typedef struct
{
	PeasExtensionBase parent;
	XmrWindow *window;

	enum
	{
		NONE = 0,
		SETTINGS_DAEMON,
		X_KEY_GRAB
	} grab_type;

	GDBusProxy *proxy;
}XmrMMKeysPlugin;

typedef struct
{
	PeasExtensionBaseClass parent_class;
}XmrMMKeysPluginClass;

XMR_DEFINE_PLUGIN(XMR_TYPE_MMKEYS_PLUGIN, XmrMMKeysPlugin, xmr_mmkeys_plugin,)

static void
media_player_key_pressed(GDBusProxy *proxy,
			  const gchar *sender,
			  const gchar *signal,
			  GVariant *parameters,
			  XmrMMKeysPlugin *plugin)
{
	gchar *key;
	gchar *application;

	g_variant_get(parameters, "(ss)", &application, &key);

	if (g_strcmp0(application, PACKAGE))
	{
		xmr_debug("got media player key signal for unexpected application '%s'", application);
		return;
	}

	if (g_strcmp0(key, "Play") == 0 ||
		g_strcmp0(key, "Pause") == 0)
	{
		if (xmr_window_playing(plugin->window))
			xmr_window_pause(plugin->window);
		else
			xmr_window_play(plugin->window);
	}
	else if (g_strcmp0(key, "Stop") == 0)
	{
		xmr_window_pause(plugin->window);
	}
	else if (g_strcmp0(key, "Next") == 0)
	{
		xmr_window_play_next(plugin->window);
	}

	g_free(key);
	g_free(application);
}

static void
grab_call_complete(GObject *proxy, GAsyncResult *res, XmrMMKeysPlugin *plugin)
{
	GError *error = NULL;
	GVariant *result;

	result = g_dbus_proxy_call_finish(G_DBUS_PROXY(proxy), res, &error);
	if (error != NULL)
	{
		xmr_debug("Unable to grab media player keys: %s", error->message);
		g_clear_error(&error);
	}
	else
	{
		g_variant_unref(result);
	}
}

static gboolean
window_focus_cb(GtkWidget *window,
		 GdkEventFocus *event,
		 XmrMMKeysPlugin *plugin)
{
	g_dbus_proxy_call(plugin->proxy,
			   "GrabMediaPlayerKeys",
			   g_variant_new("(su)", PACKAGE, 0),
			   G_DBUS_CALL_FLAGS_NONE,
			   -1,
			   NULL,
			   (GAsyncReadyCallback)grab_call_complete,
			   plugin);
	return FALSE;
}

#ifdef HAVE_MMKEYS_H

static void
grab_mmkey(int key_code, GdkWindow *root)
{
	Display *display;
	gdk_error_trap_push();

	display = GDK_DISPLAY_XDISPLAY(gdk_display_get_default ());
	XGrabKey(display, key_code,
		  0,
		  GDK_WINDOW_XID(root), True,
		  GrabModeAsync, GrabModeAsync);
	XGrabKey(display, key_code,
		  Mod2Mask,
		  GDK_WINDOW_XID(root), True,
		  GrabModeAsync, GrabModeAsync);
	XGrabKey(display, key_code,
		  Mod5Mask,
		  GDK_WINDOW_XID(root), True,
		  GrabModeAsync, GrabModeAsync);
	XGrabKey(display, key_code,
		  LockMask,
		  GDK_WINDOW_XID(root), True,
		  GrabModeAsync, GrabModeAsync);
	XGrabKey(display, key_code,
		  Mod2Mask | Mod5Mask,
		  GDK_WINDOW_XID(root), True,
		  GrabModeAsync, GrabModeAsync);
	XGrabKey(display, key_code,
		  Mod2Mask | LockMask,
		  GDK_WINDOW_XID(root), True,
		  GrabModeAsync, GrabModeAsync);
	XGrabKey(display, key_code,
		  Mod5Mask | LockMask,
		  GDK_WINDOW_XID(root), True,
		  GrabModeAsync, GrabModeAsync);
	XGrabKey(display, key_code,
		  Mod2Mask | Mod5Mask | LockMask,
		  GDK_WINDOW_XID(root), True,
		  GrabModeAsync, GrabModeAsync);

	gdk_flush();
	
	if (gdk_error_trap_pop())
	{
		xmr_debug("Error grabbing key");
	}
}

static void
ungrab_mmkey(int key_code, GdkWindow *root)
{
	Display *display;
	gdk_error_trap_push();

	display = GDK_DISPLAY_XDISPLAY(gdk_display_get_default());
	XUngrabKey(display, key_code, 0, GDK_WINDOW_XID(root));
	XUngrabKey(display, key_code, Mod2Mask, GDK_WINDOW_XID(root));
	XUngrabKey(display, key_code, Mod5Mask, GDK_WINDOW_XID(root));
	XUngrabKey(display, key_code, LockMask, GDK_WINDOW_XID(root));
	XUngrabKey(display, key_code, Mod2Mask | Mod5Mask, GDK_WINDOW_XID(root));
	XUngrabKey(display, key_code, Mod2Mask | LockMask, GDK_WINDOW_XID(root));
	XUngrabKey(display, key_code, Mod5Mask | LockMask, GDK_WINDOW_XID(root));
	XUngrabKey(display, key_code, Mod2Mask | Mod5Mask | LockMask, GDK_WINDOW_XID(root));

	gdk_flush();
	
	if (gdk_error_trap_pop ())
	{
		xmr_debug("Error grabbing key");
	}
}


static GdkFilterReturn
filter_mmkeys(GdkXEvent *xevent,
			  GdkEvent *event,
			  gpointer data)
{
	XEvent *xev;
	XKeyEvent *key;
	Display *display;
	XmrWindow *window;
	
	GdkFilterReturn retv = GDK_FILTER_CONTINUE;

	xev = (XEvent *)xevent;

	if (xev->type != KeyPress)
		return GDK_FILTER_CONTINUE;

	key = (XKeyEvent *)xevent;

	window = (XmrWindow *)data;
	display = GDK_DISPLAY_XDISPLAY(gdk_display_get_default());

	if (XKeysymToKeycode(display, XF86XK_AudioPlay) == key->keycode ||
		XKeysymToKeycode(display, XF86XK_AudioPause) == key->keycode)
	{
		if (xmr_window_playing(window))
			xmr_window_pause(window);
		else
			xmr_window_play(window);
		retv = GDK_FILTER_REMOVE;
	}
	else if (XKeysymToKeycode (display, XF86XK_AudioStop) == key->keycode)
	{
		xmr_window_pause(window);
		retv = GDK_FILTER_REMOVE;
	}
	else if (XKeysymToKeycode (display, XF86XK_AudioNext) == key->keycode)
	{
		xmr_window_play_next(window);
		retv = GDK_FILTER_REMOVE;
	}
	
	return retv;
}

static void
mmkeys_grab(XmrMMKeysPlugin *plugin, gboolean grab)
{
	gint keycodes[] = {0, 0, 0, 0, 0};
	GdkDisplay *display;

	guint i, j;

	display = gdk_display_get_default();
	keycodes[0] = XKeysymToKeycode(GDK_DISPLAY_XDISPLAY(display), XF86XK_AudioPlay);
	keycodes[1] = XKeysymToKeycode(GDK_DISPLAY_XDISPLAY(display), XF86XK_AudioStop);
	keycodes[2] = XKeysymToKeycode(GDK_DISPLAY_XDISPLAY(display), XF86XK_AudioPrev);
	keycodes[3] = XKeysymToKeycode(GDK_DISPLAY_XDISPLAY(display), XF86XK_AudioNext);
	keycodes[4] = XKeysymToKeycode(GDK_DISPLAY_XDISPLAY(display), XF86XK_AudioPause);

	for (i = 0; i < gdk_display_get_n_screens(display); ++i)
	{
		GdkScreen *screen = gdk_display_get_screen(display, i);

		if (screen != NULL)
		{
			GdkWindow *root = gdk_screen_get_root_window (screen);

			for (j = 0; j < G_N_ELEMENTS(keycodes); ++j)
			{
				if (keycodes[j] != 0)
				{
					if (grab)
						grab_mmkey(keycodes[j], root);
					else
						ungrab_mmkey(keycodes[j], root);
				}
			}

			if (grab)
				gdk_window_add_filter(root, filter_mmkeys, (gpointer)plugin->window);
			else
				gdk_window_remove_filter(root, filter_mmkeys, (gpointer) plugin->window);
		}
	}
}

#endif // HAVE_MMKEYS_H

static void
first_call_complete(GObject *proxy, GAsyncResult *res, XmrMMKeysPlugin *plugin)
{
	GVariant *result;
	GError *error = NULL;

	result = g_dbus_proxy_call_finish(G_DBUS_PROXY(proxy), res, &error);
	if (error != NULL)
	{
		xmr_debug("Unable to grab media player keys: %s", error->message);
		g_clear_error(&error);
#ifdef HAVE_MMKEYS_H
		mmkeys_grab(plugin, TRUE);
		plugin->grab_type = X_KEY_GRAB;
#endif
		return;
	}

	g_signal_connect_object(plugin->proxy, "g-signal", G_CALLBACK(media_player_key_pressed), plugin, 0);

	/* re-grab keys when the main window gains focus */
	g_signal_connect_object(plugin->window, "focus-in-event",
				 G_CALLBACK(window_focus_cb),
				 plugin, 0);

	g_variant_unref(result);
}

static void
final_call_complete(GObject *proxy, GAsyncResult *res, gpointer nothing)
{
	GError *error = NULL;
	GVariant *result;

	result = g_dbus_proxy_call_finish(G_DBUS_PROXY (proxy), res, &error);
	if (error != NULL)
	{
		xmr_debug("Unable to release media player keys: %s", error->message);
		g_clear_error (&error);
	} 
	else
	{
		g_variant_unref(result);
	}
}


static void
impl_activate(PeasActivatable *activatable)
{
	XmrMMKeysPlugin *plugin;
	GDBusConnection *bus;
	GError *error = NULL;

	plugin = XMR_MMKEYS_PLUGIN(activatable);
	plugin->grab_type = NONE;

	g_object_get(plugin, "object", &plugin->window, NULL);
	
	bus = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
	if (bus == NULL)
	{
		xmr_debug("g_bus_get_sync failed: %s", error->message);
		g_clear_error(&error);
	}
	else
	{
		plugin->proxy = g_dbus_proxy_new_sync(bus, G_DBUS_PROXY_FLAGS_NONE,
											  NULL,
											  "org.gnome.SettingsDaemon",
											  "/org/gnome/SettingsDaemon/MediaKeys",
											  "org.gnome.SettingsDaemon.MediaKeys",
											  NULL, &error);
		if (error != NULL)
		{
			xmr_debug("Unable to grab media player keys: %s", error->message);
		}
		else
		{
			g_dbus_proxy_call(plugin->proxy, "GrabMediaPlayerKeys",
							  g_variant_new("(su)", PACKAGE, 0),
							  G_DBUS_CALL_FLAGS_NONE,
							  -1, NULL,
							  (GAsyncReadyCallback)first_call_complete,
							  plugin);
			
			plugin->grab_type = SETTINGS_DAEMON;
		}
	}
	
#ifdef HAVE_MMKEYS_H
	if (plugin->grab_type == NONE)
	{
		mmkeys_grab(plugin, TRUE);
		plugin->grab_type = X_KEY_GRAB;
	}
#endif
}

static void
impl_deactivate(PeasActivatable *activatable)
{
	XmrMMKeysPlugin *plugin;
	
	plugin = XMR_MMKEYS_PLUGIN(activatable);

	if (plugin->proxy != NULL)
	{
		g_dbus_proxy_call(plugin->proxy,
						  "ReleaseMediaPlayerKeys",
						  g_variant_new("(s)", PACKAGE),
						  G_DBUS_CALL_FLAGS_NONE,
						  -1, NULL,
						  (GAsyncReadyCallback)final_call_complete,
						  NULL);

		g_object_unref(plugin->proxy);
		plugin->proxy = NULL;
	}
	
#ifdef HAVE_MMKEYS_H
	if (plugin->grab_type == X_KEY_GRAB)
		mmkeys_grab(plugin, FALSE);
#endif

	plugin->grab_type = NONE;
	g_object_unref(plugin->window);
}

static void
xmr_mmkeys_plugin_init(XmrMMKeysPlugin *plugin)
{
}

G_MODULE_EXPORT void
peas_register_types(PeasObjectModule *module)
{
	xmr_mmkeys_plugin_register_type(G_TYPE_MODULE(module));
	peas_object_module_register_extension_type(module,
						    PEAS_TYPE_ACTIVATABLE,
						    XMR_TYPE_MMKEYS_PLUGIN);
}

