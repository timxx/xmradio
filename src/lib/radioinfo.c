#include "radioinfo.h"

RadioInfo *
radio_info_new()
{
	return g_new0(RadioInfo, 1);
}

void
radio_info_free(RadioInfo *ri)
{
	if (ri)
	{
		g_free(ri->id);
		g_free(ri->name);
		g_free(ri->logo);
		g_free(ri->url);

		g_free(ri);
	}
}
