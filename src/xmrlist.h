/**
 * xmrlist.h
 * This file is part of xmradio
 *
 * Copyright (C) 2013  Weitian Leung (weitianleung@gmail.com)

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

#ifndef __XMR_LIST_H__
#define __XMR_LIST_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define XMR_TYPE_LIST			(xmr_list_get_type())
#define XMR_LIST(o)				(G_TYPE_CHECK_INSTANCE_CAST((o), XMR_TYPE_LIST, XmrList))
#define XMR_LIST_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), XMR_TYPE_LIST, XmrListClass))
#define XMR_IS_LIST(o)			(G_TYPE_CHECK_INSTANCE_TYPE((o), XMR_TYPE_LIST))
#define XMR_IS_LIST_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE((k), XMR_TYPE_LIST))
#define XMR_LIST_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS((o), XMR_TYPE_LIST, XmrListClass))

typedef struct _XmrList			XmrList;
typedef struct _XmrListClass	XmrListClass;
typedef struct _XmrListPrivate	XmrListPrivate;

struct _XmrList
{
	GtkWindow parent;
	XmrListPrivate *priv;
};

struct _XmrListClass
{
	GtkWindowClass parent_class;
};

GType
xmr_list_get_type();

typedef enum
{
	XMR_PLAYLIST,
	XMR_SEARCH_LIST
}XmrListType;

GtkWidget*
xmr_list_new(XmrListType type);

G_END_DECLS

#endif /* __XMR_LIST_H__ */
