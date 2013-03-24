/** 
 * xmrartist.h
 * This file is part of xmartist
 *
 * Copyright (C) 2013  Weitian Leung (weitianleung@gmail.com)

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
#ifndef __XMR_ARTIST_H__
#define __XMR_ARTIST_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define XMR_TYPE_ARTIST				(xmr_artist_get_type())
#define XMR_ARTIST(inst)			(G_TYPE_CHECK_INSTANCE_CAST((inst),	XMR_TYPE_ARTIST, XmrArtist))
#define XMR_ARTIST_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XMR_TYPE_ARTIST, XmrArtistClass))
#define XMR_IS_ARTIST(inst)			(G_TYPE_CHECK_INSTANCE_TYPE((inst), XMR_TYPE_ARTIST))
#define XMR_IS_ARTIST_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XMR_TYPE_ARTIST))
#define XMR_ARTIST_GET_CLASS(inst)	(G_TYPE_INSTANCE_GET_CLASS((inst), XMR_TYPE_ARTIST, XmrArtistClass))

typedef struct _XmrArtist			XmrArtist;
typedef struct _XmrArtistClass		XmrArtistClass;
typedef struct _XmrArtistPrivate	XmrArtistPrivate;

struct _XmrArtist
{
	GtkBox parent;
	XmrArtistPrivate *priv;
};

struct _XmrArtistClass
{
	GtkBoxClass parent_class;
	
	void (*clicked)(XmrArtist *artist);
};

typedef struct
{
	gchar *id;
	gchar *name;
	gchar *region;
	gchar *img_src;
}Artist;

void
artist_free(Artist *artist);


GType
xmr_artist_get_type();

GtkWidget *
xmr_artist_new();

GtkWidget *
xmr_artist_new_with_info(Artist *data);

void
xmr_artist_set_id(XmrArtist *artist,
			const gchar *id);

const gchar *
xmr_artist_get_id(XmrArtist *artist);

void
xmr_artist_set_name(XmrArtist *artist,
			const gchar *name);

const gchar *
xmr_artist_get_name(XmrArtist *artist);

void
xmr_artist_set_region(XmrArtist *artist,
			const gchar *region);

const gchar *
xmr_artist_get_region(XmrArtist *artist);

void
xmr_artist_set_img_src(XmrArtist *artist,
					   const gchar *img_src);

const gchar *
xmr_artist_get_img_src(XmrArtist *artist);

G_END_DECLS

#endif /* __XMR_ARTIST_H__ */
