/**
 * xmrlabel.h
 * This file is part of xmradio
 *
 * Copyright (C) 2012 - 2015 Weitian Leung (weitianleung@gmail.com)

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
#ifndef __XMR_LABEL_H__
#define __XMR_LABEL_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define XMR_TYPE_LABEL			(xmr_label_get_type())
#define XMR_LABEL(o)			(G_TYPE_CHECK_INSTANCE_CAST((o), XMR_TYPE_LABEL, XmrLabel))
#define XMR_LABEL_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), XMR_TYPE_LABEL, XmrLabelClass))
#define XMR_IS_LABEL(o)			(G_TYPE_CHECK_INSTANCE_TYPE((o), XMR_TYPE_LABEL))
#define XMR_IS_LABEL_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE((k), XMR_TYPE_LABEL))
#define XMR_LABEL_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS((o), XMR_TYPE_LABEL, XmrLabelClass))

typedef struct _XmrLabel			XmrLabel;
typedef struct _XmrLabelClass		XmrLabelClass;
typedef struct _XmrLabelPrivate		XmrLabelPrivate;

struct _XmrLabel
{
	GtkWidget parent;
	XmrLabelPrivate *priv;
};

struct _XmrLabelClass
{
	GtkWidgetClass parent_class;
};

GType		xmr_label_get_type();
GtkWidget*	xmr_label_new(const gchar *text);

void
xmr_label_set_text(XmrLabel *label,
			const gchar *str);

/**
 * set @color to NULL to use default color
 */
void
xmr_label_set_color(XmrLabel *label,
			const gchar *color);

/**
 * set @font to NULL to use default font
 */
void
xmr_label_set_font(XmrLabel *label,
			const gchar *font);

void
xmr_label_set_size(XmrLabel *label,
			gint width, gint height);

G_END_DECLS

#endif /* __XMR_LABEL_H__ */
