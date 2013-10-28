/** 
 * xmrplayer.h
 * This file is part of xmradio
 *
 * Copyright (C) 2012-2013  Weitian Leung (weitianleung@gmail.com)

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
#ifndef __XMR_PLAYER_H__
#define __XMR_PLAYER_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define XMR_TYPE_PLAYER			(xmr_player_get_type())
#define XMR_PLAYER(o)			(G_TYPE_CHECK_INSTANCE_CAST((o), XMR_TYPE_PLAYER, XmrPlayer))
#define XMR_PLAYER_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), XMR_TYPE_PLAYER, XmrPlayerClass))
#define XMR_IS_PLAYER(o)		(G_TYPE_CHECK_INSTANCE_TYPE((o), XMR_TYPE_PLAYER))
#define XMR_IS_PLAYER_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE((k), XMR_TYPE_PLAYER))
#define XMR_PLAYER_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS((o), XMR_TYPE_PLAYER, XmrPlayerClass))

#define XMR_PLAYER_ERROR		xmr_player_error_quark()

typedef struct _XmrPlayer			XmrPlayer;
typedef struct _XmrPlayerClass		XmrPlayerClass;
typedef struct _XmrPlayerPrivate	XmrPlayerPrivate;

struct _XmrPlayer
{
	GObject parent;
	XmrPlayerPrivate *priv;
};

struct _XmrPlayerClass
{
	GObjectClass parent_class;

	gboolean	(*open)(XmrPlayer	*player,
						const gchar *uri);

	gboolean	(*play)(XmrPlayer	*player);

	void		(*pause)(XmrPlayer *player);
	
	void		(*playpause)(XmrPlayer *player);
	
	void		(*stop)(XmrPlayer *player);

	void		(*resume)(XmrPlayer *player);

	gboolean	(*playing)(XmrPlayer *player);

	void		(*set_volume)(XmrPlayer *player,
							 float		 volume);

	float		(*get_volume)(XmrPlayer *player);

	void		(*set_time)(XmrPlayer	*player,
							gint64		 newtime);

	gint64		(*get_time)(XmrPlayer *player);

	gint64		(*get_duration)(XmrPlayer *player);
	
	void		(*set_loop)(XmrPlayer *player, gint count);
	
	gint		(*get_loop)(XmrPlayer *player);

	/* signals */
	void		(*eos)(XmrPlayer *player,
						gboolean early);

	void		(*error)(XmrPlayer	*player,
						 GError		*error);

	void		(*tick)(XmrPlayer	*player,
						 gint64		 elapsed,
						 gint64		 duration);

	void		(*buffering)(XmrPlayer *player,
						guint progress);

	void		(*state_changed)(XmrPlayer *player);
}; /* end of struct _XmrPlayerClass */

GType xmr_player_get_type();

XmrPlayer *
xmr_player_new();

gboolean
xmr_player_open(XmrPlayer	*player,
			const gchar *uri);

gboolean
xmr_player_play(XmrPlayer *player);

void	
xmr_player_pause(XmrPlayer *player);

void
xmr_player_playpause(XmrPlayer *player);

void
xmr_player_stop(XmrPlayer *player);

void
xmr_player_resume(XmrPlayer *player);

gboolean
xmr_player_playing(XmrPlayer *player);

void
xmr_player_set_volume(XmrPlayer *player,
			float		 volume);

float
xmr_player_get_volume(XmrPlayer *player);

void
xmr_player_set_time(XmrPlayer	*player,
			gint64		 newtime);

gint64
xmr_player_get_time(XmrPlayer *player);

gint64
xmr_player_get_duration(XmrPlayer *player);

gint
xmr_player_get_loop(XmrPlayer *player);

/**
 * set @count to -1 means infinite loop
 */
void
xmr_player_set_loop(XmrPlayer *player, gint count);

G_END_DECLS

#endif /* __XMR_PLAYER_H__ */
