/** 
 * xmrservice.h
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
#ifndef __XMR_SERVICE_H__
#define __XMR_SERVICE_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define XMR_SERVICE_TYPE            (xmr_service_get_type())
#define XMR_SERVICE(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), XMR_SERVICE_TYPE, XmrService))
#define XMR_SERVICE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  XMR_SERVICE_TYPE, XmrServiceClass))
#define IS_XMR_SERVICE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), XMR_SERVICE_TYPE))
#define IS_XMR_SERVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  XMR_SERVICE_TYPE))
#define XMR_SERVICE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  XMR_SERVICE_TYPE, XmrServiceClass))

typedef struct _XmrService			XmrService;
typedef struct _XmrServiceClass		XmrServiceClass;
typedef struct _XmrServicePrivate	XmrServicePrivate;

struct _XmrService
{
	GObject parent;

	XmrServicePrivate *priv;
};

struct _XmrServiceClass
{
	GObjectClass parent;
};

GType		xmr_service_get_type();
XmrService*	xmr_service_new();

/**
 * login server
 * @param usr user name (email)
 * @param pwd password
 */
gint
xmr_service_login(XmrService *xs,
			const gchar *usr,
			const gchar *pwd
			);

gboolean
xmr_service_is_logged_in(XmrService *xs);

/**
 * get private list
 */
gint
xmr_service_get_track_list(XmrService *xs, GList **list);

/**
 * get by music style
 */
gint
xmr_service_get_track_list_by_style(XmrService *xs, GList **list, const gchar *url);

/**
 * possible style for radio
 */
enum
{
	Style_XingZuo	= 1,
	Style_NianDai	= 2,
	Style_FengGe	= 12
};

/**
 * get radio list
 */
gint
xmr_service_get_radio_list(XmrService *xs, GList **list, gint style);


gint
xmr_service_like_song(XmrService *xs, const gchar *song_id, gboolean like);

gint
xmr_service_get_lyric(XmrService *xs, const gchar *song_id, GString *data);

const gchar *
xmr_service_get_usr_id(XmrService *xs);

const gchar *
xmr_service_get_usr_name(XmrService *xs);

gint
xmr_service_get_url_data(XmrService *xs,
			const gchar *url,
			GString *data);

const gchar *
xmr_service_get_error_str(gint code);

G_END_DECLS

#endif /* __XMR_SERVICE_H__ */
