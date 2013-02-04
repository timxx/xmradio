/**
 * xmrlist.c
 * This file is part of xmradio
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

#include "xmrlist.h"
#include "lib/songinfo.h"

G_DEFINE_TYPE(XmrList, xmr_list, GTK_TYPE_WINDOW)

struct _XmrListPrivate
{
	XmrListType type;
	
	GList *list;
};

enum
{
	PROP_0,
	PROP_TYPE
};

GType
xmr_list_type()
{
	static GType etype = 0;

	if (G_UNLIKELY(etype == 0))
	{
		static const GEnumValue values[] = {
			{ XMR_PLAYLIST,		"XMR_PLAYLIST",		"playlist"		},
			{ XMR_SEARCH_LIST,	"XMR_SEARCH_LIST",	"search list"	},
			{ 0, NULL, NULL }
		};
		etype = g_enum_register_static(g_intern_static_string("XmrListType"), values);
	}

	return etype;
}

static void
xmr_list_finalize(GObject *object)
{
	XmrList *list = XMR_LIST(object);
	XmrListPrivate *priv = list->priv;
	
	if (priv->list != NULL)
	{
		g_list_free_full(priv->list, (GDestroyNotify)song_info_free);
		priv->list = NULL;
	}

	G_OBJECT_CLASS(xmr_list_parent_class)->finalize(object);
}

static void
xmr_list_set_property (GObject *object,
			guint prop_id,
			const GValue *value,
			GParamSpec *pspec)
{
	XmrList *list = XMR_LIST(object);
	XmrListPrivate *priv = list->priv;

	switch (prop_id)
	{
		case PROP_TYPE:
			priv->type = g_value_get_enum(value);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
			break;
	}
}

static void
xmr_list_get_property (GObject *object,
			guint prop_id,
			GValue *value,
			GParamSpec *pspec)
{
	XmrList *list = XMR_LIST(object);
	XmrListPrivate *priv = list->priv;

	switch (prop_id)
	{
		case PROP_TYPE:
			g_value_set_enum(value, priv->type);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
			break;
	}
}

static void
xmr_list_init(XmrList *list)
{
	XmrListPrivate *priv;

	priv = list->priv = G_TYPE_INSTANCE_GET_PRIVATE(list, XMR_TYPE_LIST, XmrListPrivate);
	
	priv->list = NULL;
}

static void
xmr_list_class_init(XmrListClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
//	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	object_class->finalize = xmr_list_finalize;
	object_class->set_property = xmr_list_set_property;
	object_class->get_property = xmr_list_get_property;

	g_object_class_install_property(object_class, PROP_TYPE,
				g_param_spec_enum("list-type",
					"XmrListType",
					"The List type",
					xmr_list_type(),
					XMR_PLAYLIST,
					G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	g_type_class_add_private(object_class, sizeof(XmrListPrivate));
}

GtkWidget *
xmr_list_new(XmrListType type)
{
	return g_object_new(XMR_TYPE_LIST,
						"type", GTK_WINDOW_TOPLEVEL,
						"list-type", type,
						NULL);
}

