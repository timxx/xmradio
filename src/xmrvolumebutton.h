/** 
 * xmrvolumebutton.h
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

#ifndef __XMR_VOLUME_BUTTON_H__
#define __XMR_VOLUME_BUTTON_H__

#include <gtk/gtk.h>

#include "xmrbutton.h"

G_BEGIN_DECLS

#define XMR_TYPE_VOLUME_BUTTON			(xmr_volume_button_get_type())
#define XMR_VOLUME_BUTTON(o)			(G_TYPE_CHECK_INSTANCE_CAST((o), XMR_TYPE_VOLUME_BUTTON, XmrVolumeButton))
#define XMR_VOLUME_BUTTON_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), XMR_TYPE_VOLUME_BUTTON, XmrVolumeButtonClass))
#define XMR_IS_VOLUME_BUTTON(o)			(G_TYPE_CHECK_INSTANCE_TYPE((o), XMR_TYPE_VOLUME_BUTTON))
#define XMR_IS_VOLUME_BUTTON_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE((k), XMR_TYPE_VOLUME_BUTTON))
#define XMR_VOLUME_BUTTON_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS((o), XMR_TYPE_VOLUME_BUTTON, XmrVolumeButtonClass))

typedef struct _XmrVolumeButton			XmrVolumeButton;
typedef struct _XmrVolumeButtonClass	XmrVolumeButtonClass;
typedef struct _XmrVolumeButtonPrivate	XmrVolumeButtonPrivate;

struct _XmrVolumeButton
{
	GtkVolumeButton parent;
	XmrVolumeButtonPrivate *priv;
};

struct _XmrVolumeButtonClass
{
	GtkVolumeButtonClass parent_class;
};

GType		xmr_volume_button_get_type();
GtkWidget*	xmr_volume_button_new(XmrButtonType type);

XmrButtonType
xmr_volume_button_set_type(XmrVolumeButton *button, XmrButtonType type);

void
xmr_volume_button_set_image_from_pixbuf(XmrVolumeButton *button, GdkPixbuf *pixbuf);

void
xmr_volume_button_set_image_from_stock(XmrVolumeButton *button, const gchar *stock);

G_END_DECLS

#endif /* __XMR_VOLUME_BUTTON_H__ */

