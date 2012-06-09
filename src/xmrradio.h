/** 
 * xmrradio.h
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
#ifndef __XMR_RADIO_H__
#define __XMR_RADIO_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define XMR_TYPE_RADIO				(xmr_radio_get_type())
#define XMR_RADIO(inst)				(G_TYPE_CHECK_INSTANCE_CAST((inst),	XMR_TYPE_RADIO, XmrRadio))
#define XMR_RADIO_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XMR_TYPE_RADIO, XmrRadioClass))
#define XMR_IS_RADIO(inst)			(G_TYPE_CHECK_INSTANCE_TYPE((inst), XMR_TYPE_RADIO))
#define XMR_IS_RADIO_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XMR_TYPE_RADIO))
#define XMR_RADIO_GET_CLASS(inst)	(G_TYPE_INSTANCE_GET_CLASS((inst), XMR_TYPE_RADIO, XmrRadioClass))

typedef struct _XmrRadio		XmrRadio;
typedef struct _XmrRadioClass	XmrRadioClass;
typedef struct _XmrRadioPrivate	XmrRadioPrivate;

struct _XmrRadio
{
	GtkVBox parent;
	XmrRadioPrivate *priv;
};

struct _XmrRadioClass
{
	GtkVBoxClass parent_class;
};

GType xmr_radio_get_type();

XmrRadio *
xmr_radio_new();

XmrRadio *
xmr_radio_new_with_info(const gchar *logo,
			const gchar *name,
			const gchar *url);

void
xmr_radio_set_logo(XmrRadio *radio,
			const gchar *uri);

void
xmr_radio_set_name(XmrRadio *radio,
			const gchar *name);

const gchar *
xmr_radio_get_name(XmrRadio *radio);

void
xmr_radio_set_url(XmrRadio *radio,
			const gchar *url);

const gchar *
xmr_radio_get_url(XmrRadio *radio);

G_END_DECLS

#endif /* __XMR_RADIO_H__ */
