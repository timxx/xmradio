/** 
 * xmrradio.c
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
#include "xmrradio.h"

G_DEFINE_TYPE(XmrRadio, xmr_radio, GTK_TYPE_VBOX);

struct _XmrRadioPrivate
{
	GtkWidget *image;
	GtkWidget *label;

	gchar *url;
};

enum
{
	PROP_0,
	PROP_LOGO,
	PROP_NAME,
	PROP_URL
};

static void
xmr_radio_get_property(GObject *object,
		       guint prop_id,
		       GValue *value,
		       GParamSpec *pspec)
{
	XmrRadio *radio = XMR_RADIO(object);
	XmrRadioPrivate *priv = radio->priv;

	switch(prop_id)
	{
	case PROP_LOGO:
		{
			gchar *file = NULL;
			g_object_get(priv->image, "file", &file, NULL);
			g_value_set_string(value, file);
			g_free(file);
		}
		break;

	case PROP_NAME:
		g_value_set_string(value, gtk_label_get_text(GTK_LABEL(priv->label)));
		break;

	case PROP_URL:
		g_value_set_string(value, priv->url);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
xmr_radio_set_property(GObject *object,
		       guint prop_id,
		       const GValue *value,
		       GParamSpec *pspec)
{
	XmrRadio *radio = XMR_RADIO(object);
	XmrRadioPrivate *priv = radio->priv;

	gchar *str = NULL;

	switch(prop_id)
	{
	case PROP_LOGO:
		str = g_value_dup_string(value);
		xmr_radio_set_logo(radio, str);
		g_free(str);
		break;

	case PROP_NAME:
		str = g_value_dup_string(value);
		xmr_radio_set_name(radio, str);
		g_free(str);
		break;

	case PROP_URL:
		g_free(priv->url);
		priv->url = g_value_dup_string(value);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
xmr_radio_dispose(GObject *obj)
{
	XmrRadio *radio = XMR_RADIO(obj);
	XmrRadioPrivate *priv = radio->priv;

	if (priv->url)
	{
		g_free(priv->url);
		priv->url = NULL;
	}
}

static void xmr_radio_init(XmrRadio *radio)
{
	XmrRadioPrivate *priv;
	
	priv = G_TYPE_INSTANCE_GET_PRIVATE(radio, XMR_TYPE_RADIO, XmrRadioPrivate);
	radio->priv = priv;

	priv->image = gtk_image_new();
	priv->label = gtk_label_new("");
	priv->url = NULL;

	gtk_box_pack_start(GTK_BOX(radio), priv->image, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(radio), priv->label, FALSE, TRUE, 0);

	gtk_widget_show_all(GTK_WIDGET(radio));
}

static void xmr_radio_class_init(XmrRadioClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->set_property = xmr_radio_set_property;
	object_class->get_property = xmr_radio_get_property;
	object_class->dispose = xmr_radio_dispose;

	g_object_class_install_property(object_class,
				PROP_LOGO,
				g_param_spec_string("logo",
					"Logo uri",
					"Logo uri",
					NULL,
					G_PARAM_READWRITE)
					 );

	g_object_class_install_property(object_class,
				PROP_NAME,
				g_param_spec_string("name",
					"Name",
					"Name of the radio",
					NULL,
					G_PARAM_READWRITE)
					 );

	g_object_class_install_property(object_class,
				PROP_URL,
				g_param_spec_string("url",
					"URL",
					"URL of the radio",
					NULL,
					G_PARAM_READWRITE)
					 );

	 g_type_class_add_private(G_OBJECT_CLASS(klass), sizeof(XmrRadioPrivate));
}

XmrRadio *
xmr_radio_new()
{
	return g_object_new(XMR_TYPE_RADIO,
				"expand", FALSE,
				"spacing", 5,
				NULL);
}

XmrRadio *
xmr_radio_new_with_info(const gchar *logo,
			const gchar *name,
			const gchar *url)
{
	return g_object_new(XMR_TYPE_RADIO,
				"expand", FALSE,
				"spacing", 5,
				"logo", logo,
				"name", name,
				"url", url,
				NULL);
}

void
xmr_radio_set_logo(XmrRadio *radio,
			const gchar *uri)
{
	g_return_if_fail(radio != NULL);

	gtk_image_set_from_file(GTK_IMAGE(radio->priv->image), uri);
}

void
xmr_radio_set_name(XmrRadio *radio,
			const gchar *name)
{
	g_return_if_fail(radio != NULL);

	gtk_label_set_text(GTK_LABEL(radio->priv->label), name);
}

const gchar *
xmr_radio_get_name(XmrRadio *radio)
{
	g_return_val_if_fail(radio != NULL, NULL);

	return gtk_label_get_text(GTK_LABEL(radio->priv->label));
}

void
xmr_radio_set_url(XmrRadio *radio,
			const gchar *url)
{
	g_return_if_fail(radio != NULL);

	if (radio->priv->url)
		g_free(radio->priv->url);

	radio->priv->url = (url == NULL ? NULL : g_strdup(url));
}

const gchar *
xmr_radio_get_url(XmrRadio *radio)
{
	g_return_val_if_fail(radio != NULL, NULL);

	return radio->priv->url;
}
