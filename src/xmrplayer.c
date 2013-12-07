/** 
 * xmrplayer.c
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

#include <vlc/vlc.h>
#include <glib/gi18n.h>

#include "xmrplayer.h"
#include "xmrmarshal.h"
#include "xmrdebug.h"

G_DEFINE_TYPE(XmrPlayer, xmr_player, G_TYPE_OBJECT)

enum
{
	PROP_0,
	PROP_PLAYER
};

enum
{
	EOS,
	ERROR,
	TICK,
	BUFFERING,
	STATE_CHANGED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

struct _XmrPlayerPrivate
{
	libvlc_instance_t *instance;
	libvlc_media_player_t *player;
	
	GAsyncQueue *event_queue;
	guint event_idle_id;
	
	gint loop_count;
};

static gint g_vlc_events[] =
{
	libvlc_MediaPlayerEncounteredError,
	libvlc_MediaPlayerBuffering,
	libvlc_MediaPlayerPlaying,
	libvlc_MediaPlayerPaused,
	libvlc_MediaPlayerStopped,
	libvlc_MediaPlayerTimeChanged,
	libvlc_MediaPlayerEndReached
};

static gint g_vlc_event_count = sizeof(g_vlc_events) / sizeof(g_vlc_events[0]);

typedef struct
{
	int type;
	union
	{
		float buffering;
		gint64 position;
	};
} PlayerEvent;

static void
vlc_event_callback(const libvlc_event_t *event, void *userdata)
{
	XmrPlayer *player = (XmrPlayer *)userdata;
	PlayerEvent *player_event = g_new0(PlayerEvent, 1);
	
	switch(event->type)
	{
	case libvlc_MediaPlayerBuffering:
		player_event->buffering = event->u.media_player_buffering.new_cache;
		break;

	case libvlc_MediaPlayerTimeChanged:
		player_event->position = event->u.media_player_time_changed.new_time;
		break;

	default:
		break;
	}
	
	player_event->type = event->type;
	
	g_async_queue_push(player->priv->event_queue, player_event);
	g_main_context_wakeup(g_main_context_default());
}

static gboolean
player_event_poll(XmrPlayer *player)
{
	PlayerEvent *event = g_async_queue_try_pop(player->priv->event_queue);
	if (event == NULL)
		return TRUE;
	
	switch(event->type)
	{
	case libvlc_MediaPlayerEncounteredError:
	{
		GError error = { 0 };
		error.message = (gchar *)libvlc_errmsg();

		g_signal_emit(player, signals[ERROR], 0, &error);
	}
		break;

	case libvlc_MediaPlayerBuffering:
	{
		g_signal_emit(player, signals[BUFFERING], 0, (guint)event->buffering);
	}
		break;

	case libvlc_MediaPlayerPlaying:
	case libvlc_MediaPlayerPaused:
	case libvlc_MediaPlayerStopped:
		g_signal_emit(player, signals[STATE_CHANGED], 0);
		break;

	case libvlc_MediaPlayerTimeChanged:
	{
		g_signal_emit(player, signals[TICK], 0, event->position, xmr_player_get_duration(player));
	}
		break;

	case libvlc_MediaPlayerEndReached:
		g_signal_emit(player, signals[EOS], 0, FALSE);
		break;
		
	default:
		break;
	}
	
	g_free(event);
	
	return TRUE;
}

static void
xmr_player_dispose(GObject *object)
{
	XmrPlayer *player = XMR_PLAYER(object);
	XmrPlayerPrivate *priv = player->priv;
	
	libvlc_event_manager_t *event_mgr;
	gint i;

	event_mgr = libvlc_media_player_event_manager(priv->player);
	
	for (i = 0; i < g_vlc_event_count; ++i)
		libvlc_event_detach(event_mgr, g_vlc_events[i], vlc_event_callback, player);

	if (priv->event_idle_id != 0)
	{
		g_source_remove (priv->event_idle_id);
		priv->event_idle_id = 0;
	}

	if (priv->event_queue)
	{
		g_async_queue_unref(priv->event_queue);
		priv->event_queue = NULL;
	}

	libvlc_media_player_release(priv->player);
	libvlc_release(priv->instance);

	G_OBJECT_CLASS(xmr_player_parent_class)->dispose(object);
}

static void
xmr_player_get_property(GObject *object,
		       guint prop_id,
		       GValue *value,
		       GParamSpec *pspec)
{
	XmrPlayer *player = XMR_PLAYER(object);
	XmrPlayerPrivate *priv = player->priv;

	switch(prop_id)
	{
	case PROP_PLAYER:
		g_value_set_object (value, priv->player);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
xmr_player_set_property(GObject *object,
		       guint prop_id,
		       const GValue *value,
		       GParamSpec *pspec)
{
	switch(prop_id)
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
init_signals(XmrPlayerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	signals[EOS] =
		g_signal_new("eos",
					G_OBJECT_CLASS_TYPE(object_class),
					G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
					G_STRUCT_OFFSET(XmrPlayerClass, eos),
					NULL, NULL,
					g_cclosure_marshal_VOID__BOOLEAN,
					G_TYPE_NONE,
					1,
					G_TYPE_BOOLEAN);

	signals[ERROR] =
		g_signal_new ("error",
					G_OBJECT_CLASS_TYPE(object_class),
					G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
					G_STRUCT_OFFSET(XmrPlayerClass, error),
					NULL, NULL,
					g_cclosure_marshal_VOID__POINTER,
					G_TYPE_NONE,
					1,
					G_TYPE_POINTER);

	signals[TICK] =
		g_signal_new("tick",
					G_OBJECT_CLASS_TYPE(object_class),
					G_SIGNAL_RUN_LAST,
					G_STRUCT_OFFSET(XmrPlayerClass, tick),
					NULL, NULL,
					xmr_marshal_VOID__INT64_INT64,
					G_TYPE_NONE,
					2,
					G_TYPE_INT64, G_TYPE_INT64);

	signals[BUFFERING] =
		g_signal_new("buffering",
					G_OBJECT_CLASS_TYPE(object_class),
					G_SIGNAL_RUN_LAST,
					G_STRUCT_OFFSET(XmrPlayerClass, buffering),
					NULL, NULL,
					g_cclosure_marshal_VOID__UINT,
					G_TYPE_NONE,
					1,
					G_TYPE_UINT);

	signals[STATE_CHANGED] =
		g_signal_new("state-changed",
					G_OBJECT_CLASS_TYPE(object_class),
					G_SIGNAL_RUN_LAST,
					G_STRUCT_OFFSET(XmrPlayerClass, state_changed),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID,
					G_TYPE_NONE,
					0);
}

static void install_properties(XmrPlayerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_object_class_install_property(object_class,
				PROP_PLAYER,
				g_param_spec_pointer("player",
					"player",
					"player",
					G_PARAM_READABLE));
}

static void 
xmr_player_class_init(XmrPlayerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->get_property = xmr_player_get_property;
	object_class->set_property = xmr_player_set_property;
	object_class->dispose = xmr_player_dispose;

	init_signals(klass);
	install_properties(klass);

	klass->open = xmr_player_open;
	klass->play = xmr_player_play;
	klass->pause = xmr_player_pause;
	klass->playpause = xmr_player_playpause;
	klass->stop = xmr_player_stop;
	klass->playing = xmr_player_playing;
	klass->set_volume = xmr_player_set_volume;
	klass->get_volume = xmr_player_get_volume;
	klass->set_time = xmr_player_set_time;
	klass->get_time = xmr_player_get_time;
	klass->get_loop = xmr_player_get_loop;
	klass->set_loop = xmr_player_set_loop;

	g_type_class_add_private(object_class, sizeof(XmrPlayerPrivate));
}

static void 
xmr_player_init(XmrPlayer *player)
{
	XmrPlayerPrivate *priv;
	libvlc_event_manager_t *event_mgr;
	gint i;

	player->priv = G_TYPE_INSTANCE_GET_PRIVATE(player, XMR_TYPE_PLAYER, XmrPlayerPrivate);
	priv = player->priv;
	
	priv->instance = libvlc_new(0, NULL);
	priv->player = libvlc_media_player_new(priv->instance);
	
	priv->event_queue = g_async_queue_new();
	priv->event_idle_id = g_timeout_add(10, (GSourceFunc)player_event_poll, player);
	
	event_mgr = libvlc_media_player_event_manager(priv->player);
	
	for (i = 0; i < g_vlc_event_count; ++i)
		libvlc_event_attach(event_mgr, g_vlc_events[i], vlc_event_callback, player);
	
	priv->loop_count = 0;
}

static void
xmr_player_set_repeat(XmrPlayer *player)
{
	libvlc_media_t *media;
	gchar *option;
	
	media = libvlc_media_player_get_media(player->priv->player);
	if (media == NULL)
		return ;

	option = g_strdup_printf("input-repeat=%d", player->priv->loop_count);

	libvlc_media_add_option(media, option);
	
	g_free(option);
}

XmrPlayer*
xmr_player_new()
{
	return g_object_new(XMR_TYPE_PLAYER,
				NULL
				);
}

gboolean
xmr_player_open(XmrPlayer	*player,
			const gchar *uri)
{
	XmrPlayerPrivate *priv;
	gchar *r_uri = NULL;
	libvlc_media_t *media = NULL;

	g_return_val_if_fail( player != NULL && uri != NULL, FALSE);
	priv = player->priv;
	
	xmr_player_stop(player);

	{
		static const gchar * const prefix[] =
		{
			"http://", "file://" // we don't deal with others
		};
		gboolean prefix_ok = FALSE;
		gint i;

		for (i = 0; i < 2; ++i)
		{
			if (g_str_has_prefix(uri, prefix[i]))
			{
				prefix_ok = TRUE;
				break ;
			}
		}

		if (!prefix_ok) {
			r_uri = g_filename_to_uri(uri, NULL, NULL);
		} else {
			r_uri = g_strdup(uri);
		}
	}

	media = libvlc_media_new_location(priv->instance, r_uri);
	g_free(r_uri);

	if(media == NULL)
		return FALSE;
	
	libvlc_media_player_set_media(priv->player, media);
	xmr_player_set_repeat(player);
	
	return TRUE;
}

gboolean
xmr_player_play(XmrPlayer *player)
{
	g_return_val_if_fail(player != NULL, FALSE);
	
	return libvlc_media_player_play(player->priv->player) == 0;
}

void	
xmr_player_pause(XmrPlayer *player)
{
	g_return_if_fail(player != NULL);
	
	libvlc_media_player_pause(player->priv->player);
}

void
xmr_player_playpause(XmrPlayer *player)
{
	if (xmr_player_playing(player))
		xmr_player_pause(player);
	else
		xmr_player_play(player);
}

void
xmr_player_stop(XmrPlayer *player)
{
	g_return_if_fail(player != NULL);
	
	libvlc_media_player_stop(player->priv->player);
}

void
xmr_player_resume(XmrPlayer *player)
{
	g_return_if_fail(player != NULL);

	libvlc_media_player_set_pause(player->priv->player, 0);
}

gboolean
xmr_player_playing(XmrPlayer *player)
{
	g_return_val_if_fail(player != NULL, FALSE);
	
	return libvlc_media_player_is_playing(player->priv->player) == 1;
}

void
xmr_player_set_volume(XmrPlayer *player,
			float		 volume)
{
	g_return_if_fail(player != NULL);
	g_return_if_fail(volume >= 0.0 && volume <= 1.0);
	
	libvlc_audio_set_volume(player->priv->player, volume * 100);
}

float
xmr_player_get_volume(XmrPlayer *player)
{
	g_return_val_if_fail(player != NULL, 0);

	return libvlc_audio_get_volume(player->priv->player) / 100;
}

void
xmr_player_set_time(XmrPlayer	*player,
			gint64		 newtime)
{
	g_return_if_fail(player != NULL);
	
	libvlc_media_player_set_time(player->priv->player, newtime);
}

gint64
xmr_player_get_time(XmrPlayer *player)
{
	g_return_val_if_fail(player != NULL, 0L);

	return libvlc_media_player_get_time(player->priv->player);
}

gint64
xmr_player_get_duration(XmrPlayer *player)
{
	g_return_val_if_fail(player != NULL, 0L);

	return libvlc_media_player_get_length(player->priv->player);
}

gint
xmr_player_get_loop(XmrPlayer *player)
{
	g_return_val_if_fail(player != NULL, 0);
	
	return player->priv->loop_count;
}

void
xmr_player_set_loop(XmrPlayer *player, gint count)
{
	g_return_if_fail(player != NULL);
	
	player->priv->loop_count = count;
	xmr_player_set_repeat(player);
}
