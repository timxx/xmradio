/**
 * xmrwindow.h
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
#ifndef __XMR_WINDOW_H__
#define __XMR_WINDOW_H__

#include <gtk/gtk.h>

#include "lib/songinfo.h"
#include "lib/radioinfo.h"

G_BEGIN_DECLS

#define XMR_TYPE_WINDOW			(xmr_window_get_type())
#define XMR_WINDOW(o)			(G_TYPE_CHECK_INSTANCE_CAST((o), XMR_TYPE_WINDOW, XmrWindow))
#define XMR_WINDOW_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), XMR_TYPE_WINDOW, XmrWindowClass))
#define XMR_IS_WINDOW(o)		(G_TYPE_CHECK_INSTANCE_TYPE((o), XMR_TYPE_WINDOW))
#define XMR_IS_WINDOW_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE((k), XMR_TYPE_WINDOW))
#define XMR_WINDOW_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS((o), XMR_TYPE_WINDOW, XmrWindowClass))

typedef struct _XmrWindow			XmrWindow;
typedef struct _XmrWindowClass		XmrWindowClass;
typedef struct _XmrWindowPrivate	XmrWindowPrivate;

struct _XmrWindow
{
	GtkWindow parent;
	XmrWindowPrivate *priv;
};

struct _XmrWindowClass
{
	GtkWindowClass parent_class;

	void (*theme_changed)(XmrWindow *window,
				const gchar *new_theme);

	void (*track_changed)(XmrWindow *window,
				SongInfo *new_track);

	void (*radio_changed)(XmrWindow *window,
				RadioInfo *new_radio);
};

GType		xmr_window_get_type();
GtkWidget*	xmr_window_new();

/**
 * play next song
 */
void
xmr_window_play_next(XmrWindow *window);

void
xmr_window_pause(XmrWindow *window);

SongInfo *
xmr_window_get_current_song(XmrWindow *window);

G_END_DECLS

#endif /* __XMR_WINDOW_H__ */
