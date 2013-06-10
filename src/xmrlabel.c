/**
 * xmrlabel.c
 * This file is part of xmradio
 *
 * Copyright (C) 2012-2013  Weitian Leung (weitianleung@gmail.com)

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

#include "xmrlabel.h"

#define DEFAULT_TIMEOUT	600
#define DEFAULT_SPEED	5

G_DEFINE_TYPE(XmrLabel, xmr_label, GTK_TYPE_MISC)

struct _XmrLabelPrivate
{
	guint timeout_id;

	gchar *text;

	PangoLayout *layout;

	GdkRGBA default_color;
	GdkRGBA *current_color;

	gint current_x;		/* label pos */
};

enum
{
	PROP_0,
	PROP_LABEL
};

static void
xmr_label_update_layout(XmrLabel *label);

static gboolean
hscroll_text_timeout(XmrLabel *label)
{
	label->priv->current_x -= DEFAULT_SPEED;
	gtk_widget_queue_draw(GTK_WIDGET(label));

	return TRUE;
}

static void
xmr_label_ensure_layout(XmrLabel *label)
{
	if (label->priv->layout == NULL)
	{
		label->priv->layout = gtk_widget_create_pango_layout(GTK_WIDGET(label),
					label->priv->text);
	}
}

static void
xmr_label_finalize(GObject *object)
{
	XmrLabel *label = XMR_LABEL(object);
	XmrLabelPrivate *priv = label->priv;

	g_free(priv->text);

	if (priv->timeout_id != 0)
		g_source_remove(priv->timeout_id);

	if(priv->layout)
		g_object_unref(priv->layout);

	gdk_rgba_free(priv->current_color);

	G_OBJECT_CLASS(xmr_label_parent_class)->finalize(object);
}

static void
xmr_label_set_property (GObject *object,
			guint prop_id,
			const GValue *value,
			GParamSpec *pspec)
{
	XmrLabel *label = XMR_LABEL(object);

	switch (prop_id)
	{
		case PROP_LABEL:
			xmr_label_set_text(label, g_value_get_string(value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
			break;
	}
}

static void
xmr_label_get_property (GObject *object,
			guint prop_id,
			GValue *value,
			GParamSpec *pspec)
{
	XmrLabel *label = XMR_LABEL(object);
	XmrLabelPrivate *priv = label->priv;

	switch (prop_id)
	{
		case PROP_LABEL:
			g_value_set_string(value, priv->text);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
			break;
	}
}

static void
xmr_label_realize(GtkWidget *widget)
{
	GtkAllocation allocation;
	GdkWindow *window;
	GdkWindowAttr attributes;
	gint attributes_mask;

	gtk_widget_set_realized(widget, TRUE);

	if (!gtk_widget_get_has_window(widget))
	{
		window = gtk_widget_get_parent_window(widget);
		gtk_widget_set_window(widget, window);
		g_object_ref(window);
	}
	else
	{
		GdkRGBA rgba = { 0 };
		gtk_widget_get_allocation(widget, &allocation);

		attributes.window_type = GDK_WINDOW_CHILD;
		attributes.x = allocation.x;
		attributes.y = allocation.y;
		attributes.width = allocation.width;
		attributes.height = allocation.height;
		attributes.wclass = GDK_INPUT_OUTPUT;
		attributes.visual = gtk_widget_get_visual(widget);
		attributes.event_mask = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK;
		attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL;

		window = gdk_window_new(gtk_widget_get_parent_window(widget), &attributes, attributes_mask);
		gtk_widget_set_window(widget, window);
		gdk_window_set_user_data(window, widget);

		gdk_window_set_background_rgba(window, &rgba);
	}
}

static void
xmr_label_unrealize(GtkWidget *widget)
{
	if (gtk_widget_get_window(widget))
	{
		gdk_window_set_user_data(gtk_widget_get_window(widget), NULL);
		gdk_window_destroy(gtk_widget_get_window(widget));

		gtk_widget_set_window(widget, NULL);
	}
}

static void
xmr_label_size_allocate(GtkWidget *widget,
			GtkAllocation *allocation)
{
	GTK_WIDGET_CLASS(xmr_label_parent_class)->size_allocate(widget, allocation);

	if(gtk_widget_get_window(widget))
	{
		gdk_window_move_resize(gtk_widget_get_window(widget),
					allocation->x, allocation->y,
					allocation->width,
					allocation->height);
	}
}

static gboolean
xmr_label_draw(GtkWidget *widget, cairo_t *cr)
{
	XmrLabel *label = XMR_LABEL(widget);
	XmrLabelPrivate *priv = label->priv;

	GtkAllocation allocation;
	gint width, height;
	
	// FIXME:
	// why always need to set background rgba to
	// make it transparency
	{
		GdkRGBA rgba = { 0 };
		GdkWindow *window;
	
		window = gtk_widget_get_window(widget);
		if (window)
			gdk_window_set_background_rgba(window, &rgba);
	}

	if (!priv->text || priv->text[0] == '\0')
		return FALSE;

	xmr_label_ensure_layout(label);

	gtk_widget_get_allocation(widget, &allocation);

	pango_layout_get_pixel_size(priv->layout, &width, &height);

	if (width <= allocation.width)
	{
		if (priv->timeout_id != 0)
		{
			g_source_remove(priv->timeout_id);
			priv->timeout_id = 0;
		}

		priv->current_x = 0;
	}
	else if (priv->current_x + width <= 0)
	{
		priv->current_x = allocation.width;
	}

	cairo_move_to(cr, priv->current_x, (allocation.height - height) / 2);
	gdk_cairo_set_source_rgba(cr, priv->current_color);
	pango_cairo_show_layout(cr, priv->layout);

	return FALSE;
}

static void 
xmr_label_class_init(XmrLabelClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	object_class->finalize = xmr_label_finalize;
	object_class->set_property = xmr_label_set_property;
	object_class->get_property = xmr_label_get_property;

	widget_class->realize = xmr_label_realize;
	widget_class->unrealize = xmr_label_unrealize;
	widget_class->size_allocate = xmr_label_size_allocate;
	widget_class->draw = xmr_label_draw;

	g_object_class_install_property(object_class, PROP_LABEL,
				g_param_spec_string ("label",
					"Label",
					"The text of the label",
					"",
					G_PARAM_READWRITE));

	g_type_class_add_private(object_class, sizeof(XmrLabelPrivate));
}

static void 
xmr_label_init(XmrLabel *label)
{
	XmrLabelPrivate *priv;
	GtkStyleContext *context;

	label->priv = G_TYPE_INSTANCE_GET_PRIVATE(label, XMR_TYPE_LABEL, XmrLabelPrivate);
	priv = label->priv;

	priv->timeout_id = 0;
	priv->text = NULL;
	priv->layout = NULL;
	priv->current_x = 0;

	context = gtk_widget_get_style_context(GTK_WIDGET(label));

	gtk_style_context_lookup_color(context, "text_color", &priv->default_color);
	if (priv->default_color.alpha <= 0)
		priv->default_color.alpha = 1.0;

	priv->current_color = gdk_rgba_copy(&priv->default_color);
}

static void
xmr_label_update_layout(XmrLabel *label)
{
	XmrLabelPrivate *priv = label->priv;
	GtkAllocation allocation;
	gint width, height;

	priv->current_x = 0;

	if (priv->timeout_id != 0)
	{
		g_source_remove(priv->timeout_id);
		priv->timeout_id = 0;
	}

	xmr_label_ensure_layout(label);

	pango_layout_set_text(priv->layout, priv->text, -1);

	gtk_widget_get_allocation(GTK_WIDGET(label), &allocation);
	pango_layout_get_pixel_size(priv->layout, &width, &height);

	if (width > allocation.width) {
		priv->timeout_id = g_timeout_add(DEFAULT_TIMEOUT, (GSourceFunc)hscroll_text_timeout, label);
	} else {
		gtk_widget_queue_draw(GTK_WIDGET(label));
	}
}

GtkWidget*
xmr_label_new(const gchar *text)
{
	return g_object_new(XMR_TYPE_LABEL,
						"has-tooltip", TRUE,
						"tooltip-text", text,
						"label", text,
						NULL);
}

void
xmr_label_set_text(XmrLabel *label,
			const gchar *str)
{
	XmrLabelPrivate *priv;

	g_return_if_fail(label != NULL);
	priv = label->priv;

	g_free(priv->text);
	priv->text = g_strdup(str);

	gtk_widget_set_tooltip_text(GTK_WIDGET(label), priv->text);

	xmr_label_update_layout(label);
}

void
xmr_label_set_color(XmrLabel *label,
			const gchar *color)
{
	GdkColor gdk_color;
	XmrLabelPrivate *priv;

	g_return_if_fail(label != NULL);

	priv = label->priv;
	if (color == NULL)
	{
		gdk_rgba_free(priv->current_color);
		priv->current_color = gdk_rgba_copy(&priv->default_color);
	}
	else
	{
		if (gdk_color_parse(color, &gdk_color))
		{
			priv->current_color->red = gdk_color.red / 65535.0;
			priv->current_color->green = gdk_color.green / 65535.0;
			priv->current_color->blue = gdk_color.blue / 65535.0;
			// always set to 1.0
			priv->current_color->alpha = 1.0;
		}
	}

	gtk_widget_queue_draw(GTK_WIDGET(label));
}

void
xmr_label_set_font(XmrLabel *label,
			const gchar *font)
{
	XmrLabelPrivate *priv;

	g_return_if_fail(label != NULL);
	priv = label->priv;
	
	xmr_label_ensure_layout(label);

	// set to default
	if (font == NULL)
	{
		pango_layout_set_font_description(priv->layout, NULL);
	}
	else
	{
		PangoFontDescription * pfd;
		pfd = pango_font_description_from_string(font);
		if (pfd)
		{
			pango_layout_set_font_description(priv->layout, pfd);
			pango_font_description_free(pfd);
		}
	}

	xmr_label_update_layout(label);
}

void
xmr_label_set_size(XmrLabel *label,
			gint width, gint height)
{
	gint w = width;
	gint h = height;

	g_return_if_fail(label != NULL);

	xmr_label_ensure_layout(label);
	pango_layout_get_pixel_size(label->priv->layout, &w, &h);

	if (width == -1)
		width = w;
	if (height == -1)
		height = h;

	gtk_widget_set_size_request(GTK_WIDGET(label), width, height);

	xmr_label_update_layout(label);
}
