/**
 * xmrwindow.h
 * This file is part of xmradio
 *
 * Copyright (C) 2012 - 2013  Weitian Leung (weitianleung@gmail.com)

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
				const gchar *name,
				const gchar *url);

	void (*login_finish)(XmrWindow *window,
						 gboolean ok,
						 const gchar *message);

	void (*logout)(XmrWindow *window);

	void (*fetch_playlist_finish)(XmrWindow *window,
								  gint status,
								  GList *list);

	void (*fetch_cover_finish)(XmrWindow *window,
							   GdkPixbuf *pixbuf);
	
	void (*search_music)(XmrWindow *window,
						 const gchar *keyword);
	
	void (*playlist_changed)(XmrWindow *window);
};

GType		xmr_window_get_type();
GtkWidget*	xmr_window_new();

void
xmr_window_play(XmrWindow *window);

/**
 * play next song
 */
void
xmr_window_play_next(XmrWindow *window);

void
xmr_window_pause(XmrWindow *window);

SongInfo *
xmr_window_get_current_song(XmrWindow *window);

void
xmr_window_quit(XmrWindow *window);

gboolean
xmr_window_playing(XmrWindow *window);

void
xmr_window_love(XmrWindow *window);

void
xmr_window_hate(XmrWindow *window);

void
xmr_window_login(XmrWindow *window);

void
xmr_window_logout(XmrWindow *window);

void
xmr_window_set_volume(XmrWindow *window,
			float value);

/**
 * @brief xmr_window_add_song
 * @param window
 * @param info
 * @return FALSE if failed to add
 */
gboolean
xmr_window_add_song(XmrWindow *window,
					SongInfo *info);

gboolean
xmr_window_add_song_and_play(XmrWindow *window,
							 SongInfo *info);

/**
 * you should free your @list
 */
void
xmr_window_set_search_result(XmrWindow *window,
							 GList *list,
							 const gchar *from,
							 const gchar *link);

gboolean
xmr_window_is_current_song_marked(XmrWindow *window);

/**
 * @brief xmr_window_search_radio, show search window only
 * @param window
 */
void
xmr_window_search_radio(XmrWindow *window);

void
xmr_window_play_custom_radio(XmrWindow *window,
							 const gchar *name,
							 const gchar *url);

void
xmr_window_increase_search_music_count(XmrWindow *window);

void
xmr_window_decrease_search_music_count(XmrWindow *window);

G_END_DECLS

#endif /* __XMR_WINDOW_H__ */
