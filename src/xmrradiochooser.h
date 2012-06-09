/** 
 * xmrradiochooser.h
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

#ifndef __XMR_RADIO_CHOOSER_H__
#define __XMR_RADIO_CHOOSER_H__

#include <gtk/gtk.h>

#include "xmrradio.h"

G_BEGIN_DECLS

#define XMR_TYPE_RADIO_CHOOSER				(xmr_radio_chooser_get_type())
#define XMR_RADIO_CHOOSER(inst)				(G_TYPE_CHECK_INSTANCE_CAST((inst),	XMR_TYPE_RADIO_CHOOSER, XmrRadioChooser))
#define XMR_RADIO_CHOOSER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XMR_TYPE_RADIO_CHOOSER, XmrRadioChooserClass))
#define XMR_IS_RADIO_CHOOSER(inst)			(G_TYPE_CHECK_INSTANCE_TYPE((inst), XMR_TYPE_RADIO_CHOOSER))
#define XMR_IS_RADIO_CHOOSER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XMR_TYPE_RADIO_CHOOSER))
#define XMR_RADIO_CHOOSER_GET_CLASS(inst)	(G_TYPE_INSTANCE_GET_CLASS((inst), XMR_TYPE_RADIO_CHOOSER, XmrRadioChooserClass))

typedef struct _XmrRadioChooser			XmrRadioChooser;
typedef struct _XmrRadioChooserClass	XmrRadioChooserClass;
typedef struct _XmrRadioChooserPrivate	XmrRadioChooserPrivate;

struct _XmrRadioChooser
{
	GtkWindow parent;
	XmrRadioChooserPrivate *priv;
};

struct _XmrRadioChooserClass
{
	GtkWindowClass parent_class;

	void (*radio_selected)(XmrRadioChooser *chooser,
				XmrRadio *radio);
};

GType xmr_radio_chooser_get_type();

GtkWidget *
xmr_radio_chooser_new(const gchar *title);

void
xmr_radio_chooser_append(XmrRadioChooser *chooser,
			XmrRadio *radio);

XmrRadio *
xmr_radio_chooser_get_selected(XmrRadioChooser *chooser);

G_END_DECLS

#endif /* __XMR_RADIO_CHOOSER_H__ */
