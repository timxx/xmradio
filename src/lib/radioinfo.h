#ifndef __RADIO_INFO_H__
#define __RADIO_INFO_H__

#include <glib.h>

G_BEGIN_DECLS

typedef struct
{
	gchar *id;
	gchar *name;
	gchar *logo;
	gchar *url;

}RadioInfo;

RadioInfo *
radio_info_new();

void
radio_info_free(RadioInfo *ri);

G_END_DECLS

#endif /* __RADIO_INFO_H__ */
