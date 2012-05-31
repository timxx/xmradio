/** 
 * testlibxmrservice.c
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
#include <glib.h>
#include <locale.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include "../src/lib/xmrservice.h"
#include "../src/lib/songinfo.h"
#include "../src/lib/radioinfo.h"

static void print_song_info(SongInfo *song);
static void print_radio_info(RadioInfo *radio);

int main(int argc, char **argv)
{
	XmrService *service;
	gint result;
	const gchar *name, *pwd;
	GList *list = NULL;

	setlocale(LC_ALL, "");
	g_type_init();

	service = xmr_service_new();

	name = g_getenv("XIAMI_USER_NAME");
	pwd = g_getenv("XIAMI_USER_PWD");

	if (name && pwd)
	{
		g_print("Login with user: %s\n", name);
		result = xmr_service_login(XMR_SERVICE(service), name, pwd);
	}
	else
	{
		struct termios term, term_orig;
		gchar usr_name[32], usr_pwd[32];

		g_print("Enter username: ");
		scanf("%s", usr_name);
		g_print("Enter password: ");

		tcgetattr(STDIN_FILENO, &term);
		term_orig = term;
		term.c_lflag &= ~ECHO;
		tcsetattr(STDIN_FILENO, TCSANOW, &term);

		scanf("%s", usr_pwd);

		tcsetattr(STDIN_FILENO, TCSANOW, &term_orig);
		g_print("\n");

		result = xmr_service_login(XMR_SERVICE(service),
				usr_name,
				usr_pwd
				);
	}

	if (result != 0)
	{
		g_print("login failed(%d)\n", result);
	}
	else
	{
		g_print("login ok\n");
	}

	g_print("Getting private track list...\n\n");
	result = xmr_service_get_track_list(XMR_SERVICE(service), &list);
	if (result != 0)
	{
		g_print("failed to get private track list\n");
	}
	else
	{
		GList *p = list;

		g_print("Track List(%d)\n", g_list_length(list));

		while(p)
		{
			print_song_info(p->data);
			p = p->next;
		}

		g_list_free_full(list, (GDestroyNotify)song_info_free);
		list = NULL;
	}

	g_print("Getting FENGGE radio list...\n\n");

	result = xmr_service_get_radio_list(XMR_SERVICE(service), &list, Style_FengGe);
	if (result != 0)
	{
		g_print("failed to get radio list\n");
	}
	else
	{
		GList *p = list;

		g_print("Radio List(%d)\n", g_list_length(list));

		while(p)
		{
			print_radio_info(p->data);
			p = p->next;
		}

		g_list_free_full(list, (GDestroyNotify)radio_info_free);
		list = NULL;
	}

	g_object_unref(service);

	return 0;
}

static void print_song_info(SongInfo *song)
{
	g_print("song_id:\t%s\n"
			"song_name:\t%s\n"
			"album_id:\t%s\n"
			"album_name:\t%s\n"
			"artist_id:\t%s\n"
			"artist_name:\t%s\n"
			"album_cover:\t%s\n"
			"location:\t%s\n"
			"grade:\t%s\n\n"
			,
			song->song_id,
			song->song_name,
			song->album_id,
			song->album_name,
			song->artist_id,
			song->artist_name,
			song->album_cover,
			song->location,
			song->grade
		   );
}

static void print_radio_info(RadioInfo *radio)
{
	g_print("radio_id:\t%s\n"
			"radio_name:\t%s\n"
			"radio_logo:\t%s\n"
			"radio_url:\t%s\n"
			"\n"
			,

			radio->id,
			radio->name,
			radio->logo,
			radio->url
		   );
}
