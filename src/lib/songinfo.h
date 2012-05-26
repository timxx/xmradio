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

G_END_DECLS

#endif /* __SONG_INFO_H__ */
