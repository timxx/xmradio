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

		g_free(si);
	}
}
