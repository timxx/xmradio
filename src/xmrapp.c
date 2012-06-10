/**
 * xmrapp.c
 * This file is part of xmradio
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

#include <dbus/dbus.h>

#include "xmrapp.h"
#include "xmrwindow.h"
#include "xmrdebug.h"

#define XMR_APP_GET_PRIVATE(obj)	\
	(G_TYPE_INSTANCE_GET_PRIVATE((obj), XMR_TYPE_APP, XmrAppPrivate))

G_DEFINE_TYPE(XmrApp, xmr_app, GTK_TYPE_APPLICATION);

enum
{
	PROP_0,
	PROP_WINDOW
};

struct _XmrAppPrivate
{
	GtkWidget *window; // main window
};

typedef enum
{
	ActionNone,
	ActionPlay,
	ActionPause,
	ActionNext
}
PlayerAction;

static void
xmr_app_activate(GApplication *app);

static void
xmr_app_get_property(GObject *object,
		       guint prop_id,
		       GValue *value,
		       GParamSpec *pspec);

static DBusHandlerResult
dbus_signal_filter(DBusConnection *connection,
			DBusMessage *message,
			XmrWindow *window);

static void xmr_app_class_init(XmrAppClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	GApplicationClass *app_class = G_APPLICATION_CLASS(klass);

	object_class->get_property = xmr_app_get_property;

	app_class->activate = xmr_app_activate;

	g_object_class_install_property(object_class,
					 PROP_WINDOW,
					 g_param_spec_object ("window",
							      "GtkWindow",
							      "GtkWindow object",
							      GTK_TYPE_WINDOW,
							      G_PARAM_READABLE)
					 );

	g_type_class_add_private(object_class, sizeof(XmrAppPrivate));
}

static void xmr_app_init(XmrApp *app)
{
	app->priv = XMR_APP_GET_PRIVATE(app);

	app->priv->window = NULL;
}

XmrApp*	xmr_app_new()
{
	return g_object_new(XMR_TYPE_APP,
			     "application-id", "com.timxx.XMRadio",
				 "flags", G_APPLICATION_FLAGS_NONE,
			     NULL
				 );
}

static void
xmr_app_activate(GApplication *app)
{
	GList *list;

    list = gtk_application_get_windows(GTK_APPLICATION(app));

    if (list)
    {
        gtk_window_present(GTK_WINDOW(list->data));
    }
    else
    {
		DBusConnection *bus;
		DBusError error;
		XmrApp *xmr_app = XMR_APP(app);

		xmr_app->priv->window = xmr_window_new();

		gtk_window_set_application(GTK_WINDOW(xmr_app->priv->window), GTK_APPLICATION(app));
		gtk_widget_show(xmr_app->priv->window);

		dbus_error_init (&error);
		bus = dbus_bus_get (DBUS_BUS_SESSION, &error);
		if (!bus)
		{
			g_warning ("Failed to connect to the D-BUS daemon: %s", error.message);
			dbus_error_free(&error);
		}
		dbus_connection_setup_with_g_main (bus, NULL);
		dbus_bus_add_match(bus, "type='signal',interface='com.xmradio.dbus.Signal'", &error);
		dbus_connection_add_filter(bus, (DBusHandleMessageFunction)dbus_signal_filter, xmr_app->priv->window, NULL);
    }
}

static void
xmr_app_get_property(GObject *object,
		       guint prop_id,
		       GValue *value,
		       GParamSpec *pspec)
{
	XmrApp *app = XMR_APP(object);

	switch(prop_id)
	{
	case PROP_WINDOW:
		g_value_set_object(value, app->priv->window);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static DBusHandlerResult
dbus_signal_filter(DBusConnection *connection,
			DBusMessage *message,
			XmrWindow *window)
{
	if(dbus_message_is_signal(message, "com.xmradio.dbus.Signal", "Action"))
	{
		DBusError error;
		PlayerAction action = ActionNone;
		dbus_error_init (&error);
		if(dbus_message_get_args(message, &error, DBUS_TYPE_INT32, &action, DBUS_TYPE_INVALID))
		{
			switch(action)
			{
			case ActionPlay:
				xmr_window_play(window);
				break;

			case ActionPause:
				xmr_window_pause(window);
				break;

			case ActionNext:
				xmr_window_play_next(window);
				break;
			}
		}
		else
		{
			xmr_debug("Signal received, but error getting message: %s", error.message);
			dbus_error_free(&error);
		}
		return DBUS_HANDLER_RESULT_HANDLED;
	}

	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}
