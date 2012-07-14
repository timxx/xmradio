/** 
 * songinfo.c
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
#include "songinfo.h"

SongInfo *
song_info_new()
{
	SongInfo *si = NULL;

	si = g_new0(SongInfo, 1);

	return si;
}

void
song_info_free(SongInfo *si)
{
	if (si)
	{
		g_free(si->song_name);
		g_free(si->song_id);
		g_free(si->album_id);
		g_free(si->album_name);
		g_free(si->album_cover);
		g_free(si->location);
		g_free(si->grade);
		g_free(si->artist_id);
		g_free(si->artist_name);

		g_free(si);
	}
}

SongInfo *
song_info_copy(SongInfo *si)
{
	SongInfo *new_si;

	if (si == NULL)
		return NULL;

	new_si = g_new0(SongInfo, 1);
	g_return_val_if_fail(new_si != NULL, NULL);

	new_si->song_name = g_strdup(si->song_name);
	new_si->song_id = g_strdup(si->song_id);
	new_si->album_id = g_strdup(si->album_id);
	new_si->album_name = g_strdup(si->album_name);
	new_si->album_cover = g_strdup(si->album_cover);
	new_si->location = g_strdup(si->location);
	new_si->grade = g_strdup(si->grade);
	new_si->artist_id = g_strdup(si->artist_id);
	new_si->artist_name = g_strdup(si->artist_name);

	return new_si;
}
