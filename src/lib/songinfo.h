#ifndef __SONG_INFO_H__
#define __SONG_INFO_H__

G_BEGIN_DECLS

typedef struct
{
	gchar *song_name;	// song name
	gchar *song_id;

	gchar *ablum_id;
	gchar *ablum_name;

	gchar *artist_id;
	gchar *artist_name;
	gchar *album_cover;	// ablum image

	gchar *url;	// song url

	gchar *grade;	//
}SongInfo;

SongInfo *
song_info_new();

void
song_info_free(SongInfo *si);

G_END_DECLS

#endif /* __SONG_INFO_H__ */
