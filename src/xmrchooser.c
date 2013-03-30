/** 
 * xmrchooser.c
 * This file is part of xmradio
 *
 * Copyright (C) 2012 - 2013  Weitian Leung (weitianleung@gmail.com)

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

#include "xmrchooser.h"

G_DEFINE_TYPE(XmrChooser, xmr_chooser, GTK_TYPE_WINDOW)

struct _XmrChooserPrivate
{
	GtkWidget *view;	/* #GtkScrolledWindow */
	GtkWidget *box;		/* #GtkBox */
	GtkWidget *widget;	/* selected widget or NULL */
	
	GtkOrientation orientation;
	gboolean hide_on_clicked;
};

enum
{
	WIDGET_SELECTED,
	LAST_SIGNAL
};

enum
{
	PROP_0,
	PROP_ORIENTATION
};

static guint signals[LAST_SIGNAL] = { 0 };

static void
on_widget_clicked(GtkWidget	*widget,
			XmrChooser *chooser);

static void
remove_widget(GtkWidget *widget,
			  gpointer data);

static void
xmr_chooser_get_property(GObject *object,
							 guint prop_id,
							 GValue *value,
							 GParamSpec *pspec)
{
	XmrChooser *chooser = XMR_CHOOSER(object);
	XmrChooserPrivate *priv = chooser->priv;

	switch(prop_id)
	{
	case PROP_ORIENTATION:
		g_value_set_enum(value, priv->orientation);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
xmr_chooser_set_property(GObject *object,
							 guint prop_id,
							 const GValue *value,
							 GParamSpec *pspec)
{
	XmrChooser *chooser = XMR_CHOOSER(object);
	XmrChooserPrivate *priv = chooser->priv;

	switch(prop_id)
	{
	case PROP_ORIENTATION:
		priv->orientation = g_value_get_enum(value);
		priv->box = gtk_box_new(priv->orientation, 10);
		gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(priv->view), priv->box);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
xmr_chooser_init(XmrChooser *chooser)
{
	XmrChooserPrivate *priv;

	chooser->priv = G_TYPE_INSTANCE_GET_PRIVATE(chooser,
				XMR_TYPE_CHOOSER,
				XmrChooserPrivate); 

	priv = chooser->priv;
	priv->orientation = GTK_ORIENTATION_HORIZONTAL;
	priv->hide_on_clicked = TRUE;

	priv->widget = NULL;
	priv->view = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(priv->view),
                                    GTK_POLICY_AUTOMATIC,
									GTK_POLICY_AUTOMATIC);

	gtk_container_add(GTK_CONTAINER(chooser), priv->view);

	g_signal_connect(chooser, "delete-event",
				G_CALLBACK(gtk_widget_hide_on_delete), NULL);

	gtk_window_set_position(GTK_WINDOW(chooser), GTK_WIN_POS_CENTER);
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(chooser), TRUE);
}

static void
xmr_chooser_class_init(XmrChooserClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	
	object_class->get_property = xmr_chooser_get_property;
	object_class->set_property = xmr_chooser_set_property;
	
	g_object_class_install_property(object_class,
				PROP_ORIENTATION,
				g_param_spec_enum("orientation",
					"Orientation",
					"Widget orientation",
					GTK_TYPE_ORIENTATION,
					GTK_ORIENTATION_HORIZONTAL,
					G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	signals[WIDGET_SELECTED] =
		g_signal_new("widget-selected",
					G_OBJECT_CLASS_TYPE(object_class),
					G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
					G_STRUCT_OFFSET(XmrChooserClass, widget_selected),
					NULL, NULL,
					g_cclosure_marshal_VOID__POINTER,
					G_TYPE_NONE,
					1,
					G_TYPE_POINTER);

	 g_type_class_add_private(G_OBJECT_CLASS(klass), sizeof(XmrChooserPrivate));
}

GtkWidget *
xmr_chooser_new(const gchar *title, GtkOrientation orientation)
{
	return g_object_new(XMR_TYPE_CHOOSER,
				"title", title,
				"height-request", 150,
				"width-request", 150,
				"default-width", 400,
				"orientation", orientation,
				NULL);
}

GtkWidget *
xmr_chooser_new_with_parent(GtkWindow *parent,
							GtkOrientation orientation)
{
	return g_object_new(XMR_TYPE_CHOOSER,
				"resizable", FALSE,
				"orientation", orientation,
				"parent", parent,
				NULL);
}

void
xmr_chooser_append(XmrChooser *chooser,
			GtkWidget *widget)
{
	g_return_if_fail( chooser != NULL && widget != NULL);

	gtk_box_pack_start(GTK_BOX(chooser->priv->box),
				widget, TRUE, FALSE, 0);

	g_signal_connect(widget, "clicked",
				G_CALLBACK(on_widget_clicked), chooser);
}

void
xmr_chooser_clear(XmrChooser *chooser)
{
	XmrChooserPrivate *priv;
	g_return_if_fail(chooser != NULL);
	priv = chooser->priv;
	
	gtk_container_foreach(GTK_CONTAINER(priv->box), (GtkCallback)remove_widget, NULL);
}

GtkWidget *
xmr_chooser_get_selected(XmrChooser *chooser)
{
	g_return_val_if_fail(chooser != NULL, NULL);

	return chooser->priv->widget;
}

static void
on_widget_clicked(GtkWidget *widget,
			XmrChooser *chooser)
{
	chooser->priv->widget = widget;

	g_signal_emit(chooser, signals[WIDGET_SELECTED], 0, widget);

	if (chooser->priv->hide_on_clicked)
		gtk_widget_hide(GTK_WIDGET(chooser));
}

static void
remove_widget(GtkWidget *widget,
			  gpointer data)
{
	gtk_widget_destroy(widget);
}

void
xmr_chooser_show(XmrChooser *chooser)
{
	g_return_if_fail(chooser != NULL);

	gtk_widget_show_all(GTK_WIDGET(chooser));
}

void
xmr_chooser_set_hide_on_clicked(XmrChooser *chooser,
								gboolean hide)
{
	g_return_if_fail(chooser != NULL);
	chooser->priv->hide_on_clicked = hide;
}
