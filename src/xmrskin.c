/** 
 * xmrskin.c
 * This file is part of xmradio
 *
 * Copyright (C) 2012, 2015  Weitian Leung (weitianleung@gmail.com)

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
#include <libxml/parser.h>

#include "xmrskin.h"
#include "xmrdebug.h"

G_DEFINE_TYPE(XmrSkin, xmr_skin, G_TYPE_OBJECT)

struct _XmrSkinPrivate
{
	xmlDocPtr doc;
	SkinInfo *skin_info;
};

static gboolean
fill_skin_info(XmrSkin *skin, const gchar *file);

static xmlChar*
xml_get_prop(xmlNodePtr node, const xmlChar *name);

static xmlNodePtr
xml_first_child(xmlNodePtr root, const xmlChar *child);

static gboolean
pos_str_to_ii(const gchar *str,
			  gint *x, gint *y);

static xmlNodePtr
xml_get_ui_node(XmrSkin *skin, const gchar *ui);

static gboolean
get_name_value(XmrSkin *skin,
			   const gchar *ui,
			   const gchar *name,
			   const gchar *attr,
			   gchar **value);

static void 
xmr_skin_dispose(GObject *obj)
{
	XmrSkin *skin = XMR_SKIN(obj);
	XmrSkinPrivate *priv = skin->priv;

	if (priv->doc)
		xmlFreeDoc(priv->doc);

	xmr_skin_info_free(priv->skin_info);

	G_OBJECT_CLASS(xmr_skin_parent_class)->dispose(obj);
}

static void xmr_skin_class_init(XmrSkinClass *klass)
{
	GObjectClass *obj_class = (GObjectClass *)klass;

	obj_class->dispose = xmr_skin_dispose;

	g_type_class_add_private(obj_class, sizeof(XmrSkinPrivate));
}

static void xmr_skin_init(XmrSkin *skin)
{
	XmrSkinPrivate *priv;

	skin->priv = G_TYPE_INSTANCE_GET_PRIVATE(skin, XMR_TYPE_SKIN, XmrSkinPrivate);
	priv = skin->priv;

	priv->doc = NULL;
	priv->skin_info = g_new0(SkinInfo, 1);
}

XmrSkin* xmr_skin_new()
{
	return g_object_new(XMR_TYPE_SKIN,
						NULL);
}

gboolean
xmr_skin_load(XmrSkin *skin, const gchar *uri)
{
	XmrSkinPrivate *priv;
	gboolean result = FALSE;
	gchar *xml_file = NULL;

	g_return_val_if_fail(skin != NULL && uri != NULL, FALSE);

	priv = skin->priv;
	if (g_strcmp0(priv->skin_info->file, uri) == 0)
	{
		xmr_debug("%s already loaded\n", uri);
		return FALSE;
	}

	xmr_debug("load skin: %s\n", uri);

	xml_file = g_strdup_printf("%s/skin.xml", uri);

	priv->doc = xmlReadFile(xml_file, NULL, XML_PARSE_RECOVER | XML_PARSE_NOERROR);
	if (priv->doc)
		result = fill_skin_info(skin, uri);

	g_free(xml_file);

	return result;
}

static gboolean
fill_skin_info(XmrSkin *skin, const gchar *file)
{
	XmrSkinPrivate *priv = skin->priv;
	xmlNodePtr node;

	node = xmlDocGetRootElement(priv->doc);
	if (node == NULL)
	{
		xmr_debug("skin has no root node!!!\n");
		return FALSE;
	}

	priv->skin_info->file		= g_strdup(file);
	priv->skin_info->version	= (gchar *)xml_get_prop(node, BAD_CAST "version");
	priv->skin_info->name		= (gchar *)xml_get_prop(node, BAD_CAST "name");
	priv->skin_info->author		= (gchar *)xml_get_prop(node, BAD_CAST "author");
	priv->skin_info->url		= (gchar *)xml_get_prop(node, BAD_CAST "url");
	priv->skin_info->email		= (gchar *)xml_get_prop(node, BAD_CAST "email");

	return (priv->skin_info->name && priv->skin_info->name[0]);
}

SkinInfo*
xmr_skin_info_new()
{
	return g_new0(SkinInfo, 1);
}

void
xmr_skin_info_free(SkinInfo *info)
{
	if (info)
	{
		g_free(info->name);
		g_free(info->version);
		g_free(info->author);
		g_free(info->url);
		g_free(info->email);
		g_free(info->file);

		if (info->data && info->data_destroy)
			info->data_destroy(info->data);

		g_free(info);
	}
}

void
xmr_skin_get_info(XmrSkin *skin, SkinInfo *info)
{
	XmrSkinPrivate *priv;

	g_return_if_fail(skin != NULL && info != NULL);
	priv = skin->priv;

	info->name = g_strdup(priv->skin_info->name);
	info->version = g_strdup(priv->skin_info->version);
	info->author = g_strdup(priv->skin_info->author);
	info->url = g_strdup(priv->skin_info->url);
	info->email = g_strdup(priv->skin_info->email);
	info->file = g_strdup(priv->skin_info->file);
}

static xmlChar*
xml_get_prop(xmlNodePtr node, const xmlChar *name)
{
	if (node && xmlHasProp(node, name))
		return xmlGetProp(node, name);

	return NULL;
}

static xmlNodePtr
xml_first_child(xmlNodePtr root, const xmlChar *child)
{
	xmlNodePtr p = (root == NULL ? NULL : root->children);

	for( ; p ; p = p->next)
	{
		if (xmlStrEqual(p->name, child))
			return p;
	}

	return NULL;
}

static gboolean
pos_str_to_ii(const gchar *str,
			  gint *x, gint *y)
{
	gchar **strv;

	strv = g_strsplit(str, ",", 0);
	if (strv == NULL)
		return FALSE;

	if (g_strv_length(strv) != 2)
	{
		g_strfreev(strv);
		return FALSE;
	}

	if (x){
		*x = g_strtod(strv[0], NULL);
	}
	if (y){
		*y = g_strtod(strv[1], NULL);
	}

	g_strfreev(strv);

	return TRUE;
}

static xmlNodePtr
xml_get_ui_node(XmrSkin *skin, const gchar *ui)
{
	XmrSkinPrivate *priv;
	xmlNodePtr root = NULL;
	xmlNodePtr child = NULL;

	priv = skin->priv;

	root = xmlDocGetRootElement(priv->doc);
	if (root == NULL)
		return NULL;

	child = xml_first_child(root, BAD_CAST ui);

	return child;
}

static gboolean
get_name_value(XmrSkin *skin,
			   const gchar *ui,
			   const gchar *name,
			   const gchar *attr,
			   gchar **value)
{
	XmrSkinPrivate *priv;
	xmlNodePtr root = NULL;
	xmlNodePtr child = NULL;
	xmlChar *xml_value = NULL;

	g_return_val_if_fail(skin != NULL && name != NULL &&
			attr != NULL && value != NULL, FALSE);

	priv = skin->priv;
	g_return_val_if_fail(priv->doc != NULL, FALSE);

	root = xml_get_ui_node(skin, ui);
	if (root == NULL)
		return FALSE;

	child = xml_first_child(root, BAD_CAST name);
	if (child == NULL)
	{
		xmr_debug("No such element: %s", name);
		return FALSE;
	}

	xml_value = xml_get_prop(child, BAD_CAST attr);
	if (xml_value == NULL)
		return FALSE;

	*value = (gchar *)xml_value;

	return TRUE;
}

gboolean
xmr_skin_get_position(XmrSkin *skin,
					  const gchar *ui,
					  const gchar *name,
					  gint *x, gint *y)
{
	gchar *value = NULL;
	gint result;

	get_name_value(skin, ui, name, "position", &value);
	if (value == NULL)
		return FALSE;

	result = pos_str_to_ii(value, x, y);

	g_free(value);

	return result;
}

gboolean
xmr_skin_get_size(XmrSkin *skin,
				  const gchar *ui,
				  const gchar *name,
				  gint *w, gint *h)
{
	gchar *value = NULL;
	gint result;

	get_name_value(skin, ui, name, "size", &value);
	if (value == NULL)
		return FALSE;

	result = pos_str_to_ii(value, w, h);

	g_free(value);

	return result;
}

GdkPixbuf *
xmr_skin_get_image(XmrSkin *skin,
				   const gchar *ui,
				   const gchar *name)
{
	XmrSkinPrivate *priv;
	xmlNodePtr root = NULL;
	xmlNodePtr child = NULL;
	gchar *image_file = NULL;
	xmlChar *image = NULL;
	GdkPixbuf *pixbuf = NULL;

	g_return_val_if_fail(skin != NULL, pixbuf);
	priv = skin->priv;
	g_return_val_if_fail(priv->doc != NULL, pixbuf);

	root = xml_get_ui_node(skin, ui);
	if (root == NULL)
		return pixbuf;

	if (name == NULL) /* get root node image */
	{
		image = xml_get_prop(root, BAD_CAST "image");
	}
	else
	{
		child = xml_first_child(root, BAD_CAST name);
		if (child == NULL)
		{
			xmr_debug("No such element: %s", name);
			return pixbuf;
		}
		image = xml_get_prop(child, BAD_CAST "image");
	}

	if (image == NULL)
	{
		xmr_debug("No image property");
		return pixbuf;
	}

	image_file = g_strdup_printf("%s/%s", priv->skin_info->file, image);
	pixbuf = gdk_pixbuf_new_from_file(image_file, NULL);

	xmlFree(image);
	g_free(image_file);

	return pixbuf;
}

gboolean
xmr_skin_get_color(XmrSkin *skin,
				   const gchar *ui,
				   const gchar *name,
				   gchar **color)
{
	return get_name_value(skin, ui, name, "color", color);
}

gboolean
xmr_skin_get_font(XmrSkin *skin,
				  const gchar *ui,
				  const gchar *name,
				  gchar **font)
{
	return get_name_value(skin, ui, name, "font", font);
}

