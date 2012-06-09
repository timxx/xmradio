/** 
 * xmrradiochooser.c
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

#include "xmrradiochooser.h"

G_DEFINE_TYPE(XmrRadioChooser, xmr_radio_chooser, GTK_TYPE_WINDOW);

struct _XmrRadioChooserPrivate
{
	GtkWidget *view;	/* #GtkScrolledWindow */
	GtkWidget *box;		/* #GtkBox */
	XmrRadio *radio;	/* selected radio or NULL */
};

enum
{
	RADIO_SELECTED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static void
on_radio_clicked(XmrRadio	*radio,
			XmrRadioChooser *chooser);

static void
xmr_radio_chooser_init(XmrRadioChooser *chooser)
{
	XmrRadioChooserPrivate *priv;

	chooser->priv = G_TYPE_INSTANCE_GET_PRIVATE(chooser,
				XMR_TYPE_RADIO_CHOOSER,
				XmrRadioChooserPrivate); 

	priv = chooser->priv;

	priv->radio = NULL;
	priv->view = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(priv->view),
                                    GTK_POLICY_AUTOMATIC,
									GTK_POLICY_AUTOMATIC);

	gtk_container_add(GTK_CONTAINER(chooser), priv->view); 

	priv->box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	g_object_set(priv->box, 
				"margin-left", 10,
				"margin-top", 10,
				"valign", GTK_ALIGN_CENTER,
				"halign", GTK_ALIGN_CENTER,
				NULL);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(priv->view), priv->box);

	g_signal_connect(chooser, "delete-event",
				G_CALLBACK(gtk_widget_hide_on_delete), NULL);

	gtk_window_set_position(GTK_WINDOW(chooser), GTK_WIN_POS_CENTER);
}

static void
xmr_radio_chooser_class_init(XmrRadioChooserClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	signals[RADIO_SELECTED] =
		g_signal_new("radio-selected",
					G_OBJECT_CLASS_TYPE(object_class),
					G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
					G_STRUCT_OFFSET(XmrRadioChooserClass, radio_selected),
					NULL, NULL,
					g_cclosure_marshal_VOID__POINTER,
					G_TYPE_NONE,
					1,
					G_TYPE_POINTER);

	 g_type_class_add_private(G_OBJECT_CLASS(klass), sizeof(XmrRadioChooserPrivate));
}

GtkWidget *
xmr_radio_chooser_new(const gchar *title)
{
	return g_object_new(XMR_TYPE_RADIO_CHOOSER,
				"title", title,
				"height-request", 150,
				"width-request", 210,
				"default-width", 400,
				NULL);
}

void
xmr_radio_chooser_append(XmrRadioChooser *chooser,
			XmrRadio *radio)
{
	g_return_if_fail( chooser != NULL && radio != NULL);

	gtk_box_pack_start(GTK_BOX(chooser->priv->box),
				GTK_WIDGET(radio), TRUE, FALSE, 0);

	g_signal_connect(radio, "clicked",
				G_CALLBACK(on_radio_clicked), chooser);
}

XmrRadio *
xmr_radio_chooser_get_selected(XmrRadioChooser *chooser)
{
	g_return_val_if_fail(chooser != NULL, NULL);

	return chooser->priv->radio;
}

static void
on_radio_clicked(XmrRadio	*radio,
			XmrRadioChooser *chooser)
{
	g_signal_emit(chooser, signals[RADIO_SELECTED], 0, radio);

	gtk_widget_hide(GTK_WIDGET(chooser));
}
