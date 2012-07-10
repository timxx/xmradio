/** 
 * xmrskin.h
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
#ifndef __XMR_SKIN_H__
#define __XMR_SKIN_H__

#include <glib-object.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

G_BEGIN_DECLS

#define XMR_TYPE_SKIN			(xmr_skin_get_type())
#define XMR_SKIN(o)				(G_TYPE_CHECK_INSTANCE_CAST((o), XMR_TYPE_SKIN, XmrSkin))
#define XMR_SKIN_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), XMR_TYPE_SKIN, XmrSkinClass))
#define XMR_IS_SKIN(o)			(G_TYPE_CHECK_INSTANCE_TYPE((o), XMR_TYPE_SKIN))
#define XMR_IS_SKIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE((k), XMR_TYPE_SKIN))
#define XMR_SKIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS((o), XMR_TYPE_SKIN, XmrSkinClass))

typedef struct _XmrSkin			XmrSkin;
typedef struct _XmrSkinClass	XmrSkinClass;
typedef struct _XmrSkinPrivate	XmrSkinPrivate;

struct _XmrSkin
{
	GObject parent;

	XmrSkinPrivate *priv;
};

struct _XmrSkinClass
{
	GObjectClass parent_class;
};

typedef struct
{
	gchar *name;		// name of the skin
	gchar *version;		// version of the skin
	gchar *author;		// skin maker
	gchar *url;			// url
	gchar *email;		// email
	gchar *file;		// the skin file path

	gpointer data;		// extra data
	GDestroyNotify data_destroy;
}SkinInfo;

GType		xmr_skin_get_type();
XmrSkin*	xmr_skin_new();

/**
 * call once only
 * @param uri skin file path
 */
gboolean
xmr_skin_load(XmrSkin *skin, const gchar *uri);


SkinInfo*
xmr_skin_info_new();

void
xmr_skin_info_free(SkinInfo *info);

#define UI_MAIN "main_window"

/**
 * fill skin info to @info
 */
void
xmr_skin_get_info(XmrSkin *skin, SkinInfo *info);

/**
 * get 'position' element value
 */
gboolean
xmr_skin_get_position(XmrSkin *skin,
			const gchar *ui,
			const gchar *name,
			gint *x, gint *y);

gboolean
xmr_skin_get_size(XmrSkin *skin,
			const gchar *ui,
			const gchar *name,
			gint *w, gint *h);
/**
 * @param name when @name is NULL,
 * #XmrSkin will return the root node image,
 * otherwise get child which named @name's image
 */
GdkPixbuf *
xmr_skin_get_image(XmrSkin *skin,
			const gchar *ui,
			const gchar *name);

/**
 * get color attribute value
 */
gboolean
xmr_skin_get_color(XmrSkin *skin,
			const gchar *ui,
			const gchar *name,
			gchar **color);

/**
 * get font attribute value
 */
gboolean
xmr_skin_get_font(XmrSkin *skin,
			const gchar *ui,
			const gchar *name,
			gchar **font);


G_END_DECLS

#endif /* __XMR_SKIN_H__ */
