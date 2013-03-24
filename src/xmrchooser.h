/** 
 * xmrchooser.h
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

#ifndef __XMR_CHOOSER_H__
#define __XMR_CHOOSER_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define XMR_TYPE_CHOOSER				(xmr_chooser_get_type())
#define XMR_CHOOSER(inst)				(G_TYPE_CHECK_INSTANCE_CAST((inst),	XMR_TYPE_CHOOSER, XmrChooser))
#define XMR_CHOOSER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XMR_TYPE_CHOOSER, XmrChooserClass))
#define XMR_IS_CHOOSER(inst)			(G_TYPE_CHECK_INSTANCE_TYPE((inst), XMR_TYPE_CHOOSER))
#define XMR_IS_CHOOSER_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), XMR_TYPE_CHOOSER))
#define XMR_CHOOSER_GET_CLASS(inst)		(G_TYPE_INSTANCE_GET_CLASS((inst), XMR_TYPE_CHOOSER, XmrChooserClass))

typedef struct _XmrChooser			XmrChooser;
typedef struct _XmrChooserClass		XmrChooserClass;
typedef struct _XmrChooserPrivate	XmrChooserPrivate;

struct _XmrChooser
{
	GtkWindow parent;
	XmrChooserPrivate *priv;
};

struct _XmrChooserClass
{
	GtkWindowClass parent_class;

	void (*widget_selected)(XmrChooser *chooser, GtkWidget *widget);
};

GType
xmr_chooser_get_type();

/**
 * @brief xmr_chooser_new
 * @param title
 * @param orientation
 * @return 
 */
GtkWidget *
xmr_chooser_new(const gchar *title,
				GtkOrientation orientation);

void
xmr_chooser_append(XmrChooser *chooser,
			GtkWidget *widget);

void
xmr_chooser_clear(XmrChooser *chooser);

GtkWidget *
xmr_chooser_get_selected(XmrChooser *chooser);

void
xmr_chooser_show(XmrChooser *chooser);

void
xmr_chooser_set_hide_on_clicked(XmrChooser *chooser,
								gboolean hide);

G_END_DECLS

#endif /* __XMR_CHOOSER_H__ */
