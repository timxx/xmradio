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
