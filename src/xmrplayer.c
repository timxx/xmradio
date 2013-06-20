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
/**
 * Reference from part of Rhythmbox source code
 */

#include <gst/interfaces/streamvolume.h>
#include <glib/gi18n.h>

#include "xmrplayer.h"
#include "xmrmarshal.h"
#include "xmrdebug.h"

G_DEFINE_TYPE(XmrPlayer, xmr_player, G_TYPE_OBJECT)

enum
{
	PROP_0,
	PROP_PLAYBIN,
	PROP_BUS
};

enum
{
	EOS,
	ERROR,
	TICK,
	BUFFERING,
	VOLUME_CHANGED,
	STATE_CHANGED,
	LAST_SIGNAL
};

enum StateChangeAction
{
	DO_NOTHING,
	PLAYER_SHUTDOWN,
	SET_NEXT_URI,
	STOP_TICK_TIMER,
	FINISH_TRACK_CHANGE
};

static guint signals[LAST_SIGNAL] = { 0 };

struct _XmrPlayerPrivate
{
	gchar *prev_uri;	/* previous song uri */
	gchar *uri;			/* current song */

	GstElement *playbin;
	GstElement *audio_sink;

	float cur_volume;

	gint tick_timeout_id;

	gboolean playing;
	gboolean buffering;

	gboolean current_track_finishing;
	gboolean stream_change_pending;

	enum StateChangeAction state_change_action;
};

static void
start_state_change(XmrPlayer *player, GstState state, enum StateChangeAction action);

static void
state_change_finished(XmrPlayer *player, GError *error);

static void
track_change_done(XmrPlayer *player, GError *error);


GQuark
xmr_player_error_quark()
{
	static GQuark quark = 0;
	if (!quark)
		quark = g_quark_from_static_string("xmr_player_error");

	return quark;
}

#define ENUM_ENTRY(NAME, DESC) { NAME, "" #NAME "", DESC }

GType
xmr_player_error_get_type()
{
	static GType type = 0;

	if (type == 0)
	{
		static const GEnumValue values[] =
		{
			ENUM_ENTRY(XMR_PLAYER_ERROR_NO_AUDIO, "no-audio"),
			ENUM_ENTRY(XMR_PLAYER_ERROR_GENERAL, "general-error"),
			ENUM_ENTRY(XMR_PLAYER_ERROR_INTERNAL, "internal-error"),
			ENUM_ENTRY(XMR_PLAYER_ERROR_NOT_FOUND, "not-found"),
			{ 0, 0, 0 }
		};

		type = g_enum_register_static("XmrPlayerError", values);
	}

	return type;
}

static gboolean
emit_volume_changed_idle(XmrPlayer *player)
{
	gdouble vol;

	if (gst_element_implements_interface(player->priv->playbin, GST_TYPE_STREAM_VOLUME))
	{
		vol = gst_stream_volume_get_volume(GST_STREAM_VOLUME (player->priv->playbin),
						    GST_STREAM_VOLUME_FORMAT_CUBIC);
	}
	else
	{
		vol = player->priv->cur_volume;
	}

	g_signal_emit(player, signals[VOLUME_CHANGED], 0, vol);

	return FALSE;
}

static void
volume_notify_cb(GObject *element, GstObject *prop_object, GParamSpec *pspec, XmrPlayer *player)
{
	gdouble vol;
	g_object_get(element, "volume", &vol, NULL);

	player->priv->cur_volume = vol;

	g_idle_add((GSourceFunc) emit_volume_changed_idle, player);
}

static gboolean
tick_timeout(XmrPlayer *player)
{
	if (player->priv->playing)
	{
		gint64 position;
		gint64 duration;

		position = xmr_player_get_time(player);
		duration = xmr_player_get_duration(player);

		g_signal_emit(player, signals[TICK], 0, position, duration);
	}

	return TRUE;
}

static void
about_to_finish_cb(GstElement *playbin, XmrPlayer *player)
{
	player->priv->current_track_finishing = TRUE;
	g_signal_emit(player, signals[EOS], 0, TRUE);
}

static void
track_change_done(XmrPlayer *player, GError *error)
{
	XmrPlayerPrivate *priv = player->priv;

	priv->stream_change_pending = FALSE;

	if (error != NULL)
	{
		xmr_debug ("track change failed: %s", error->message);
		return;
	}

	xmr_debug ("track change finished");

	priv->current_track_finishing = FALSE;
	priv->buffering = FALSE;
	priv->playing = TRUE;

	if (priv->tick_timeout_id == 0)
	{
		priv->tick_timeout_id =
			g_timeout_add (1000 / 2,
				       (GSourceFunc) tick_timeout,
				       player);
	}
}

static void
start_state_change(XmrPlayer *player, GstState state, enum StateChangeAction action)
{
	GstStateChangeReturn scr;

	player->priv->state_change_action = action;
	scr = gst_element_set_state(player->priv->playbin, state);
	if (scr == GST_STATE_CHANGE_SUCCESS)
	{
		xmr_debug ("state change succeeded synchronously");
		state_change_finished(player, NULL);
	}
}

static void
state_change_finished(XmrPlayer *player, GError *error)
{
	XmrPlayerPrivate *priv = player->priv;
	enum StateChangeAction action = priv->state_change_action;
	priv->state_change_action = DO_NOTHING;

	switch (action)
	{
	case DO_NOTHING:
		break;

	case PLAYER_SHUTDOWN:
		if (error != NULL) {
			g_warning ("unable to shut down player pipeline: %s\n", error->message);
		}
		break;

	case SET_NEXT_URI:
		if (error != NULL) {
			g_warning ("unable to stop playback: %s\n", error->message);
		} else {
			xmr_debug ("setting new playback URI %s", priv->uri);
			g_object_set (priv->playbin, "uri", priv->uri, NULL);
			start_state_change (player, GST_STATE_PLAYING, FINISH_TRACK_CHANGE);
		}
		break;

	case STOP_TICK_TIMER:
		if (error != NULL) {
			g_warning ("unable to pause playback: %s\n", error->message);
		} else {
			if (priv->tick_timeout_id != 0) {
				g_source_remove (priv->tick_timeout_id);
				priv->tick_timeout_id = 0;
			}
		}
		break;

	case FINISH_TRACK_CHANGE:
		track_change_done (player, error);
		break;
	}
}

static gboolean
bus_cb(GstBus *bus, GstMessage *message, XmrPlayer *player)
{
	XmrPlayerPrivate *priv;

	g_return_val_if_fail (player != NULL, FALSE);
	priv = player->priv;

	switch (GST_MESSAGE_TYPE (message))
	{
	case GST_MESSAGE_ERROR:
		{
			gchar *debug;
			GError *error;

			gst_message_parse_error(message, &error, &debug);

			g_signal_emit(player, signals[ERROR], 0, error);

			g_free(debug);
			g_error_free(error);
		}
		break;

	case GST_MESSAGE_EOS:
		g_signal_emit(player, signals[EOS], 0, FALSE);
		break;

	case GST_MESSAGE_BUFFERING:
	{
		gint progress;
		gst_message_parse_buffering(message, &progress);
		if (progress >= 100)
		{
			priv->buffering = FALSE;
			if (priv->playing)
			{
				xmr_debug("buffering done, setting pipeline back to PLAYING");
				gst_element_set_state (priv->playbin, GST_STATE_PLAYING);
			}
			else
			{
				xmr_debug("buffering done, leaving pipeline PAUSED");
			}
		}
		else if (priv->buffering == FALSE && priv->playing)
		{
			xmr_debug ("buffering - temporarily pausing playback");
			gst_element_set_state (priv->playbin, GST_STATE_PAUSED);
			priv->buffering = TRUE;
		}
		g_signal_emit(player, signals[BUFFERING], 0, progress);
		break;
	}

	case GST_MESSAGE_STATE_CHANGED:
		{
			GstState oldstate;
			GstState newstate;
			GstState pending;
			gst_message_parse_state_changed (message, &oldstate, &newstate, &pending);

			if (GST_MESSAGE_SRC(message) == GST_OBJECT(priv->playbin))
			{
				if (pending == GST_STATE_VOID_PENDING)
				{
					xmr_debug("playbin reached state %s", gst_element_state_get_name(newstate));
					state_change_finished(player, NULL);
				}
				g_signal_emit(player, signals[STATE_CHANGED], 0, oldstate, newstate);
			}
			break;
		}
		
	case GST_MESSAGE_CLOCK_LOST:
		/* Get a new clock */
		gst_element_set_state(priv->playbin, GST_STATE_PAUSED);
		gst_element_set_state(priv->playbin, GST_STATE_PLAYING);
		break;

	default:
		break;
	}

	return TRUE;
}

static gboolean
init_pipeline(XmrPlayer *player, GError **error)
{
	XmrPlayerPrivate *priv = player->priv;

	priv->playbin = gst_element_factory_make ("playbin2", NULL);
	if (priv->playbin == NULL)
	{
		g_set_error(error,
			     XMR_PLAYER_ERROR,
			     XMR_PLAYER_ERROR_GENERAL,
			     _("Failed to create playbin2 element; check your GStreamer installation"));
		return FALSE;
	}

	g_signal_connect_object(G_OBJECT(priv->playbin),
				 "about-to-finish",
				 G_CALLBACK(about_to_finish_cb),
				 player, 0);
	g_signal_connect_object(G_OBJECT(priv->playbin),
				 "deep-notify::volume",
				 G_CALLBACK(volume_notify_cb),
				 player, 0);

	gst_bus_add_watch(gst_element_get_bus(priv->playbin),
			   (GstBusFunc)bus_cb,
			   player);

	g_object_notify(G_OBJECT(player), "playbin");
	g_object_notify(G_OBJECT(player), "bus");

	g_object_get(priv->playbin, "audio-sink", &priv->audio_sink, NULL);
	if (priv->audio_sink == NULL)
	{
		priv->audio_sink = gst_element_factory_make("autoaudiosink", "audio-output");
		if (priv->audio_sink) {
			g_object_set(priv->playbin, "audio-sink", priv->audio_sink, NULL);
		}
	}
	else
	{
		xmr_debug ("existing audio sink found");
		g_object_unref(priv->audio_sink);
	}

	return TRUE;
}

static void
xmr_player_dispose(GObject *object)
{
	XmrPlayer *player = XMR_PLAYER(object);
	XmrPlayerPrivate *priv = player->priv;

	if (priv->tick_timeout_id != 0)
	{
		g_source_remove (priv->tick_timeout_id);
		priv->tick_timeout_id = 0;
	}

	if (priv->playbin != NULL)
	{
		gst_element_set_state(priv->playbin, GST_STATE_NULL);
		g_object_unref(priv->playbin);
		priv->playbin = NULL;
		priv->audio_sink = NULL;
	}

	g_free(priv->uri);
	g_free(priv->prev_uri);

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
	case PROP_PLAYBIN:
		g_value_set_object (value, priv->playbin);
		break;
	case PROP_BUS:
		if (priv->playbin)
		{
			GstBus *bus;
			bus = gst_element_get_bus (priv->playbin);
			g_value_set_object(value, bus);
			gst_object_unref(bus);
		}
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
					G_STRUCT_OFFSET(XmrPlayerClass, tick),
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
					xmr_marshal_VOID__INT_INT,
					G_TYPE_NONE,
					2,
					G_TYPE_INT, G_TYPE_INT);

	signals[VOLUME_CHANGED] =
		g_signal_new("volume-changed",
					G_OBJECT_CLASS_TYPE(object_class),
					G_SIGNAL_RUN_LAST,
					G_STRUCT_OFFSET(XmrPlayerClass, volume_changed),
					NULL, NULL,
					g_cclosure_marshal_VOID__FLOAT,
					G_TYPE_NONE,
					1,
					G_TYPE_FLOAT);
}

static void install_properties(XmrPlayerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_object_class_install_property(object_class,
				PROP_PLAYBIN,
				g_param_spec_object ("playbin",
					"playbin",
					"playbin element",
					GST_TYPE_ELEMENT,
					G_PARAM_READABLE));

	g_object_class_install_property (object_class,
				PROP_BUS,
				g_param_spec_object ("bus",
					"bus",
					"GStreamer message bus",
					GST_TYPE_BUS,
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
	klass->opened = xmr_player_opened;
	klass->close = xmr_player_close;
	klass->play = xmr_player_play;
	klass->pause = xmr_player_pause;
	klass->playing = xmr_player_playing;
	klass->set_volume = xmr_player_set_volume;
	klass->get_volume = xmr_player_get_volume;
	klass->set_time = xmr_player_set_time;
	klass->get_time = xmr_player_get_time;

	g_type_class_add_private(object_class, sizeof(XmrPlayerPrivate));
}

static void 
xmr_player_init(XmrPlayer *player)
{
	XmrPlayerPrivate *priv;

	player->priv = G_TYPE_INSTANCE_GET_PRIVATE(player, XMR_TYPE_PLAYER, XmrPlayerPrivate);
	priv = player->priv;

	priv->playbin = NULL;
	priv->audio_sink = NULL;

	priv->uri = NULL;
	priv->prev_uri = NULL;
	priv->playing = FALSE;
	priv->buffering = FALSE;
	priv->cur_volume = 1.0;
	priv->tick_timeout_id = 0;
	priv->current_track_finishing = FALSE;
	priv->stream_change_pending = FALSE;
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
			const gchar *uri,
			GError		**error)
{
	XmrPlayerPrivate *priv;
	gchar *r_uri = NULL;

	g_return_val_if_fail( player != NULL && uri != NULL, FALSE);
	priv = player->priv;

	if (priv->playbin == NULL)
	{
		if (!init_pipeline(player, error))
			return FALSE;
	}

	xmr_player_close(player);

	{
		static const gchar * const prefix[] =
		{
			"http://", "file://" // we don't deal with others
		};
		gboolean prefix_ok = FALSE;
		int i;

		for (i=0; i<2; i++)
		{
			if (g_str_has_prefix(uri, prefix[i]))
			{
				prefix_ok = TRUE;
				break ;
			}
		}

		if (!prefix_ok) {
			r_uri = g_strdup_printf("file://%s", uri);
		} else {
			r_uri = g_strdup(uri);
		}
	}

	g_free (priv->prev_uri);
	priv->prev_uri = priv->uri;
	priv->uri = r_uri;

	priv->stream_change_pending = TRUE;

	return TRUE;
}

gboolean
xmr_player_opened(XmrPlayer *player)
{
	g_return_val_if_fail( player != NULL, FALSE);
	return player->priv->uri != NULL;
}

gboolean
xmr_player_close(XmrPlayer	*player)
{
	XmrPlayerPrivate *priv;

	g_return_val_if_fail( player != NULL, FALSE);
	priv = player->priv;
	
	xmr_player_set_time(player, 0);

	if (priv->playbin != NULL)
	{
		gst_element_set_state(priv->playbin, GST_STATE_NULL);
	}

	g_free (priv->uri);
	g_free (priv->prev_uri);
	priv->uri = NULL;
	priv->prev_uri = NULL;

	priv->playing = FALSE;
	priv->buffering = FALSE;
	priv->current_track_finishing = FALSE;

	if (priv->tick_timeout_id != 0)
	{
		g_source_remove(priv->tick_timeout_id);
		priv->tick_timeout_id = 0;
	}

	return TRUE;
}

gboolean
xmr_player_play(XmrPlayer *player)
{
	XmrPlayerPrivate *priv;

	g_return_val_if_fail( player != NULL, FALSE);
	priv = player->priv;

	g_return_val_if_fail( priv->playbin != NULL, FALSE);
	g_return_val_if_fail( priv->uri != NULL, FALSE);

	if (priv->stream_change_pending == FALSE)
	{
		xmr_debug ("no stream change pending, just restarting playback");
		start_state_change (player, GST_STATE_PLAYING, FINISH_TRACK_CHANGE);
	}
	else if (priv->current_track_finishing)
	{
		xmr_debug ("current track finishing -> just setting URI on playbin");
		g_object_set(priv->playbin, "uri", priv->uri, NULL);

		track_change_done(player, NULL);
	}
	else
	{
		xmr_debug ("play next song");
		start_state_change(player, GST_STATE_READY, SET_NEXT_URI);
	}

	return TRUE;
}

void	
xmr_player_pause(XmrPlayer *player)
{
	XmrPlayerPrivate *priv;

	g_return_if_fail( player != NULL);
	priv = player->priv;

	if (!priv->playing)
		return ;

	priv->playing = FALSE;
	g_return_if_fail( priv->playbin != NULL);

	start_state_change(player, GST_STATE_PAUSED, STOP_TICK_TIMER);
}

gboolean
xmr_player_resume(XmrPlayer *player)
{
	g_return_val_if_fail( player != NULL, FALSE);
	g_return_val_if_fail( player->priv->playbin != NULL, FALSE);

	if (player->priv->playing)
		return TRUE;

	start_state_change(player, GST_STATE_PLAYING, FINISH_TRACK_CHANGE);

	return TRUE;
}

gboolean
xmr_player_playing(XmrPlayer *player)
{
	g_return_val_if_fail( player != NULL, FALSE);

	return player->priv->playing;
}

void
xmr_player_set_volume(XmrPlayer *player,
			float		 volume)
{
	g_return_if_fail( player != NULL);
	g_return_if_fail (volume >= 0.0 && volume <= 1.0);

	if (player->priv->playbin == NULL)
		return ;

	g_signal_handlers_block_by_func(player->priv->playbin, volume_notify_cb, player);

	if (gst_element_implements_interface(player->priv->playbin, GST_TYPE_STREAM_VOLUME))
		gst_stream_volume_set_volume(GST_STREAM_VOLUME(player->priv->playbin),
					      GST_STREAM_VOLUME_FORMAT_CUBIC, volume);
	else
		g_object_set(player->priv->playbin, "volume", volume, NULL);

	g_signal_handlers_unblock_by_func(player->priv->playbin, volume_notify_cb, player);

	player->priv->cur_volume = volume;
}

float
xmr_player_get_volume(XmrPlayer *player)
{
	g_return_val_if_fail( player != NULL, 0);

	return player->priv->cur_volume;
}

void
xmr_player_set_time(XmrPlayer	*player,
			gint64		 newtime)
{
	g_return_if_fail( player != NULL);

	gst_element_seek(player->priv->playbin, 1.0,
			  GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
			  GST_SEEK_TYPE_SET, newtime,
			  GST_SEEK_TYPE_NONE, -1);
}

gint64
xmr_player_get_time(XmrPlayer *player)
{
	g_return_val_if_fail( player != NULL, 0L);

	if (player->priv->playbin != NULL)
	{
		gint64 position = -1;
		GstFormat fmt = GST_FORMAT_TIME;

		gst_element_query_position(player->priv->playbin, &fmt, &position);
		return position;
	}

	return -1;
}

gint64
xmr_player_get_duration(XmrPlayer *player)
{
	GstFormat fmt = GST_FORMAT_TIME;
	gint64 duration = 0;

	gst_element_query_duration(player->priv->playbin, &fmt, &duration);

	return duration;
}
