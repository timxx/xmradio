/** 
 * testxmrplayer.c
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
#include <signal.h>

#include "../src/xmrplayer.h"
#include "../src/xmrdebug.h"

#define TEST_SONG1 "http://m1.file.xiami.com/526/49526/279740/3320602_159988_l.mp3"
#define TEST_SONG2 "http://m1.file.xiami.com/629/2629/167442/2051506_10338286_l.mp3"

static GMainLoop *loop = NULL;

static void	player_eos(XmrPlayer *player,
			gboolean  early,
			gpointer data)
{
	g_print("eos (%d)\n", early);
	g_main_loop_quit(loop);
}

static void player_error(XmrPlayer	*player,
			GError		*error,
			gpointer data)
{
	g_print("error: %s\n", error->message);
	g_main_loop_quit(loop);
}

static void	player_tick(XmrPlayer	*player,
			gint64		 elapsed,
			gint64		 duration,
			gpointer data)
{
	g_print("tick: %"G_GINT64_FORMAT"/%"G_GINT64_FORMAT"\n", elapsed, duration);
}

static void player_buffering(XmrPlayer *player,
			guint progress,
			gpointer data)
{
	g_print("buffering: %d\n", progress);
}

static void player_state_changed(XmrPlayer *player,
			gpointer data)
{
	g_print("state changed\n");
}

static void	signal_handler(int signum)
{
	g_main_loop_quit(loop);
}

static gboolean timeout_next(gpointer data)
{
	XmrPlayer *player = (XmrPlayer *)data;

	xmr_player_resume(player);
	g_print("play prev track\n");

	g_usleep(G_USEC_PER_SEC * 5);

	xmr_player_open(player, TEST_SONG2);
	xmr_player_set_loop(player, 2);
	xmr_player_play(player);

	g_print("should changed song\n");

	return FALSE;
}

static gboolean timeout_pause(gpointer data)
{
	XmrPlayer *player = (XmrPlayer *)data;

	xmr_player_pause(player);
	g_print("should be paused\n");
	g_timeout_add_seconds(3, timeout_next, player);

	return FALSE;
}

int main(int argc, char **argv)
{
	XmrPlayer *player;

#if !GLIB_CHECK_VERSION(2, 35, 7)
	g_type_init();
#endif

	xmr_debug_enable(TRUE);

	signal(SIGINT, signal_handler);

	loop = g_main_loop_new(NULL, FALSE);

	player = xmr_player_new();

	g_signal_connect(player,
				"eos",
				G_CALLBACK(player_eos),
				NULL);

	g_signal_connect(player,
				"error",
				G_CALLBACK(player_error),
				NULL);

	g_signal_connect(player,
				"tick",
				G_CALLBACK(player_tick),
				NULL);

	g_signal_connect(player,
				"buffering",
				G_CALLBACK(player_buffering),
				NULL);

	g_signal_connect(player,
				"state-changed",
				G_CALLBACK(player_state_changed),
				NULL);

	if (!xmr_player_open(player, TEST_SONG1))
	{
		g_print("open failed\n");

		g_object_unref(player);

		return 1;
	}

	xmr_player_set_volume(player, 1.0);
	if (!xmr_player_play(player))
	{
	   g_print("failed to play\n");
	}
	else
	{
		g_timeout_add_seconds(10, timeout_pause, player);
		g_main_loop_run(loop);
	}

	g_object_unref(player);

	return 0;
}
