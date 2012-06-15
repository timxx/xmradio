/** 
 * xmr-app-indicator.c
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

#include "xmr-app-indicator.h"

G_DEFINE_TYPE(XmrAppIndicator, xmr_app_indicator, APP_INDICATOR_TYPE)

struct _XmrAppIndicatorPrivate
{
	GtkWidget *popup_menu;
};

enum
{
	PROP_0,
	PROP_POPUP_MENU
};

static void
xmr_app_indicator_get_property(GObject *object,
		       guint prop_id,
		       GValue *value,
		       GParamSpec *pspec)
{
	XmrAppIndicator *indicator = XMR_APP_INDICATOR(object);
	XmrAppIndicatorPrivate *priv = indicator->priv;

	switch(prop_id)
	{
	case PROP_POPUP_MENU:
		g_value_set_object(value, priv->popup_menu);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
xmr_app_indicator_set_property(GObject *object,
		       guint prop_id,
		       const GValue *value,
		       GParamSpec *pspec)
{
	XmrAppIndicator *indicator = XMR_APP_INDICATOR(object);
	XmrAppIndicatorPrivate *priv = indicator->priv;

	switch(prop_id)
	{
	case PROP_POPUP_MENU:
		priv->popup_menu = g_value_get_object(value);
		app_indicator_set_menu(APP_INDICATOR(indicator), GTK_MENU(priv->popup_menu));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
xmr_app_indicator_init(XmrAppIndicator *indicator)
{
	XmrAppIndicatorPrivate *priv;

	indicator->priv = G_TYPE_INSTANCE_GET_PRIVATE(indicator, XMR_TYPE_APP_INDICATOR, XmrAppIndicatorPrivate);

	priv = indicator->priv;
	priv->popup_menu = NULL;

	app_indicator_set_status(APP_INDICATOR(indicator), APP_INDICATOR_STATUS_ACTIVE);
}

static void
xmr_app_indicator_class_init(XmrAppIndicatorClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->get_property = xmr_app_indicator_get_property;
	object_class->set_property = xmr_app_indicator_set_property;

	g_object_class_install_property(object_class,
				PROP_POPUP_MENU,
				g_param_spec_object("popup-menu",
					"Popup menu",
					"Popup menu",
					GTK_TYPE_WIDGET,
					G_PARAM_READWRITE)
					 );

	g_type_class_add_private(G_OBJECT_CLASS(klass), sizeof(XmrAppIndicatorPrivate));
}


XmrAppIndicator *
xmr_app_indicator_new(GtkWidget *popup_menu)
{
	return g_object_new(XMR_TYPE_APP_INDICATOR,
				"category", "ApplicationStatus",
				"icon-name", "xmradio",
				"id", "xmradio",
				"popup-menu", popup_menu,
				NULL);
}
