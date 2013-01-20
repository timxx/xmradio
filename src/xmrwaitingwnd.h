/**
 * xmrwaitingwnd.h
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
#ifndef __XMR_WAITING_WND_H__
#define __XMR_WAITING_WND_H_

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define XMR_TYPE_WAITING_WND			(xmr_waiting_wnd_get_type())
#define XMR_WAITING_WND(o)				(G_TYPE_CHECK_INSTANCE_CAST((o), XMR_TYPE_WAITING_WND, XmrWaitingWnd))
#define XMR_WAITING_WND_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), XMR_TYPE_WAITING_WND, XmrWaitingWndClass))
#define XMR_IS_WAITING_WND(o)			(G_TYPE_CHECK_INSTANCE_TYPE((o), XMR_TYPE_WAITING_WND))
#define XMR_IS_WAITING_WND_CLASS(k)		(G_TYPE_CHECK_CLASS_TYPE((k), XMR_TYPE_WAITING_WND))
#define XMR_WAITING_WND_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS((o), XMR_TYPE_WAITING_WND, XmrWaitingWndClass))

typedef struct _XmrWaitingWnd			XmrWaitingWnd;
typedef struct _XmrWaitingWndClass		XmrWaitingWndClass;
typedef struct _XmrWaitingWndPrivate	XmrWaitingWndPrivate;

struct _XmrWaitingWnd
{
	GtkWindow parent;
	XmrWaitingWndPrivate *priv;
};

struct _XmrWaitingWndClass
{
	GtkWindowClass parent_class;
};

typedef enum
{
	INFO_LOGIN,
	INFO_BUFFERING,
	INFO_PLAYLIST
}InfoType;

GType
xmr_waiting_wnd_get_type();

GtkWidget*
xmr_waiting_wnd_new(GtkWindow *parent);

/**
 * @brief xmr_waiting_wnd_add_task
 * add a new task if type doesn't exists
 * it will shown im
 */
void
xmr_waiting_wnd_add_task(XmrWaitingWnd *wnd,
						 InfoType type,
						 const gchar *text);
void
xmr_waiting_wnd_show(XmrWaitingWnd *wnd);

void
xmr_waiting_wnd_hide(XmrWaitingWnd *wnd);

/**
 * @brief xmr_waiting_wnd_next_task: switch to next task
 * @param wnd
 */
void
xmr_waiting_wnd_next_task(XmrWaitingWnd *wnd);

G_END_DECLS

#endif /* __XMR_WAITING_WND_H__ */
