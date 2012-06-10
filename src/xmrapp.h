/**
 * xmrapp.h
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

#ifndef __XMR_APP_H__
#define __XMR_APP_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define XMR_TYPE_APP			(xmr_app_get_type())
#define XMR_APP(o)				(G_TYPE_CHECK_INSTANCE_CAST((o), XMR_TYPE_APP, XmrApp))
#define XMR_APP_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), XMR_TYPE_APP, XmrAppClass))
#define XMR_IS_APP(o)			(G_TYPE_CHECK_INSTANCE_TYPE((o), XMR_TYPE_APP))
#define XMR_IS_APP_CLASS(k)		(G_TYPE_CHECK_CLASS_TYPE((k), XMR_TYPE_APP))
#define XMR_APP_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS((o), XMR_TYPE_APP, XmrAppClass))

typedef struct _XmrApp			XmrApp;
typedef struct _XmrAppClass		XmrAppClass;
typedef struct _XmrAppPrivate	XmrAppPrivate;

struct _XmrApp
{
	GtkApplication parent;

	XmrAppPrivate *priv;
};

struct _XmrAppClass
{
	GtkApplicationClass parent_class;
};

GType	xmr_app_get_type();
XmrApp*	xmr_app_new();


G_END_DECLS

#endif /* __XMR_APP_H__ */
