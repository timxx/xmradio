/** 
 * songinfo.h
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
#ifndef __SONG_INFO_H__
#define __SONG_INFO_H__

#include <glib.h>

G_BEGIN_DECLS

typedef struct
{
	gchar *song_id;
	gchar *song_name;	// song name

	gchar *album_id;
	gchar *album_name;

	gchar *artist_id;
	gchar *artist_name;
	gchar *album_cover;	// album image

	gchar *location;	// song url

	gchar *grade;	//
}SongInfo;

SongInfo *
song_info_new();

void
song_info_free(SongInfo *si);

SongInfo *
song_info_copy(SongInfo *si);

G_END_DECLS

#endif /* __SONG_INFO_H__ */
