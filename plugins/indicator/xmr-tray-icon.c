/** 
 * xmr-tray-icon.c
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

#define GDK_DISABLE_DEPRECATION_WARNINGS

#include "xmr-tray-icon.h"

G_DEFINE_TYPE(XmrTrayIcon, xmr_tray_icon, GTK_TYPE_STATUS_ICON)

struct _XmrTrayIconPrivate
{
	GtkWidget *popup_menu;
	GtkWidget *main_window;
};

enum
{
	PROP_0,
	PROP_MAIN_WINDOW,
	PROP_POPUP_MENU
};

static void on_tray_popup_menu(GtkStatusIcon *status_icon,
			guint          button,
            guint          activate_time,
            gpointer       data);

static gboolean
on_tray_activate(GtkStatusIcon *status_icon,
			gpointer data);

static void
xmr_tray_icon_get_property(GObject *object,
		       guint prop_id,
		       GValue *value,
		       GParamSpec *pspec);

static void
xmr_tray_icon_set_property(GObject *object,
		       guint prop_id,
		       const GValue *value,
		       GParamSpec *pspec);

static void
xmr_tray_icon_init(XmrTrayIcon *tray)
{
	XmrTrayIconPrivate *priv;

	tray->priv = G_TYPE_INSTANCE_GET_PRIVATE(tray, XMR_TYPE_TRAY_ICON, XmrTrayIconPrivate);

	priv = tray->priv;
	priv->popup_menu = NULL;
	priv->main_window = NULL;

	gtk_status_icon_set_tooltip_markup(GTK_STATUS_ICON(tray), "<b>xmradio</b>");

	g_signal_connect(tray, "popup-menu", G_CALLBACK(on_tray_popup_menu), NULL);
    g_signal_connect(tray, "activate", G_CALLBACK(on_tray_activate), NULL);
}

static void
xmr_tray_icon_class_init(XmrTrayIconClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->get_property = xmr_tray_icon_get_property;
	object_class->set_property = xmr_tray_icon_set_property;

	g_object_class_install_property(object_class,
				PROP_MAIN_WINDOW,
				g_param_spec_object("main-window",
					"GtkWindow",
					"GtkWindow object",
					GTK_TYPE_WINDOW,
					G_PARAM_READWRITE)
					 );

	g_object_class_install_property(object_class,
				PROP_POPUP_MENU,
				g_param_spec_object("popup-menu",
					"Popup menu",
					"Popup menu",
					GTK_TYPE_WIDGET,
					G_PARAM_READWRITE)
					 );

	g_type_class_add_private(G_OBJECT_CLASS(klass), sizeof(XmrTrayIconPrivate));
}

GtkWidget* xmr_tray_icon_new(GtkWidget *main_window,
			GtkWidget *popup_menu)
{
    return g_object_new(XMR_TYPE_TRAY_ICON,
				"icon-name", "xmradio",
				"main-window", main_window,
				"popup_menu", popup_menu,
				NULL);
}

void xmr_tray_icon_set_tooltips(XmrTrayIcon *tray, const gchar *text)
{
	g_return_if_fail(tray != NULL && text != NULL);

	gtk_status_icon_set_tooltip_markup(GTK_STATUS_ICON(tray), text);
}

static void
on_tray_popup_menu(GtkStatusIcon *status_icon,
			guint          button,
            guint          activate_time,
            gpointer       data)
{
	XmrTrayIcon *tray = XMR_TRAY_ICON(status_icon);

	if (tray->priv->popup_menu != NULL)
		gtk_menu_popup(GTK_MENU(tray->priv->popup_menu), NULL, NULL, NULL, data, button, activate_time); 
}

static gboolean
on_tray_activate(GtkStatusIcon *status_icon,
			gpointer       data)
{
	XmrTrayIcon *tray = XMR_TRAY_ICON(status_icon);
	XmrTrayIconPrivate *priv = tray->priv;

	if (priv->main_window == NULL)
	{
		return FALSE;
	}

    if (gtk_widget_get_visible(priv->main_window))
    {
        gtk_widget_hide(priv->main_window);
    }
    else
    {
        gtk_widget_show(priv->main_window);
        gtk_window_present(GTK_WINDOW(priv->main_window));
    }

	return FALSE;
}

static void
xmr_tray_icon_get_property(GObject *object,
		       guint prop_id,
		       GValue *value,
		       GParamSpec *pspec)
{
	XmrTrayIcon *tray = XMR_TRAY_ICON(object);
	XmrTrayIconPrivate *priv = tray->priv;

	switch(prop_id)
	{
	case PROP_MAIN_WINDOW:
		g_value_set_object(value, priv->main_window);
		break;

	case PROP_POPUP_MENU:
		g_value_set_object(value, priv->popup_menu);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
xmr_tray_icon_set_property(GObject *object,
		       guint prop_id,
		       const GValue *value,
		       GParamSpec *pspec)
{
	XmrTrayIcon *tray = XMR_TRAY_ICON(object);
	XmrTrayIconPrivate *priv = tray->priv;

	switch(prop_id)
	{
	case PROP_MAIN_WINDOW:
		priv->main_window = g_value_get_object(value);
		break;

	case PROP_POPUP_MENU:
		priv->popup_menu = g_value_get_object(value);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}
