/**
 * xmrbutton.h
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
#ifndef __XMR_BUTTON_H__
#define __XMR_BUTTON_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define XMR_TYPE_BUTTON			(xmr_button_get_type())
#define XMR_BUTTON(o)			(G_TYPE_CHECK_INSTANCE_CAST((o), XMR_TYPE_BUTTON, XmrButton))
#define XMR_BUTTON_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), XMR_TYPE_BUTTON, XmrButtonClass))
#define XMR_IS_BUTTON(o)		(G_TYPE_CHECK_INSTANCE_TYPE((o), XMR_TYPE_BUTTON))
#define XMR_IS_BUTTON_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE((k), XMR_TYPE_BUTTON))
#define XMR_BUTTON_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS((o), XMR_TYPE_BUTTON, XmrButtonClass))

typedef struct _XmrButton			XmrButton;
typedef struct _XmrButtonClass		XmrButtonClass;
typedef struct _XmrButtonPrivate	XmrButtonPrivate;

struct _XmrButton
{
	GtkButton parent;
	XmrButtonPrivate *priv;
};

struct _XmrButtonClass
{
	GtkButtonClass parent_class;
};

typedef enum
{
	XMR_BUTTON_NORMAL,
	XMR_BUTTON_SKIN
}
XmrButtonType;

typedef enum
{
	STATE_NORMAL = 0,
	STATE_FOCUS,
	STATE_PUSH,
	STATE_DISABLE,
	LAST_STATE
}
XmrButtonState;

GType		xmr_button_get_type();
GtkWidget*	xmr_button_new(XmrButtonType type);

XmrButtonType
xmr_button_set_type(XmrButton *button, XmrButtonType type);

/**
 * toggle button state to @state
 */
XmrButtonState
xmr_button_toggle_state_on(XmrButton *button, XmrButtonState state);

void
xmr_button_toggle_state_off(XmrButton *button);

gboolean
xmr_button_is_toggled(XmrButton *button);

void
xmr_button_set_image_from_uri(XmrButton *button, const gchar *uri);

void
xmr_button_set_image_from_pixbuf(XmrButton *button, GdkPixbuf *pixbuf);

void
xmr_button_set_image_from_stock(XmrButton *button, const gchar *stock);

G_END_DECLS

#endif /* __XMR_BUTTON_H__ */
