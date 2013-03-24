/** 
 * xmrsearchbox.h
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

#ifndef __XMR_SEARCH_BOX_H__
#define __XMR_SEARCH_BOX_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define XMR_TYPE_SEARCH_BOX			(xmr_search_box_get_type())
#define XMR_SEARCH_BOX(o)			(G_TYPE_CHECK_INSTANCE_CAST((o), XMR_TYPE_SEARCH_BOX, XmrSearchBox))
#define XMR_SEARCH_BOX_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), XMR_TYPE_SEARCH_BOX, XmrSearchBoxClass))
#define XMR_IS_SEARCH_BOX(o)		(G_TYPE_CHECK_INSTANCE_TYPE((o), XMR_TYPE_SEARCH_BOX))
#define XMR_IS_SEARCH_BOX_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE((k), XMR_TYPE_SEARCH_BOX))
#define XMR_SEARCH_BOX_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS((o), XMR_TYPE_SEARCH_BOX, XmrSearchBoxClass))

typedef struct _XmrSearchBox			XmrSearchBox;
typedef struct _XmrSearchBoxClass		XmrSearchBoxClass;
typedef struct _XmrSearchBoxPrivate		XmrSearchBoxPrivate;

struct _XmrSearchBox
{
	GtkWindow parent;
	XmrSearchBoxPrivate *priv;
};

struct _XmrSearchBoxClass
{
	GtkWindowClass parent_class;
};

GType
xmr_search_box_get_type();

GtkWidget*
xmr_search_box_new(GtkWindow *parent);

/**
 * @brief xmr_search_box_show, outside should call this
 * @param box
 */
void
xmr_search_box_show(XmrSearchBox *box);

G_END_DECLS

#endif // __XMR_SEARCH_BOX_H__

