/**
 * xmrlist.c
 * This file is part of xmradio
 *
 * Copyright (C) 2013	Weitian Leung (weitianleung@gmail.com)

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.	If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib/gi18n.h>

#include "xmrlist.h"
#include "lib/songinfo.h"
#include "xmrwindow.h"
#include "xmrdebug.h"

G_DEFINE_TYPE(XmrList, xmr_list, GTK_TYPE_WINDOW)

#define FOR_EACH_ROW(treeview)	\
	GtkTreeIter iter;	\
	gboolean valid;		\
	gint idx = 0;			\
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));	\
	for (valid = gtk_tree_model_get_iter_first(model, &iter);	\
		 valid;	\
		 valid = gtk_tree_model_iter_next(model, &iter), ++idx)

struct _XmrListPrivate
{
	XmrListType type;
	GtkWindow *parent;

	GList *list;
	GtkWidget *treeview;
	
	GtkWidget *layout; // GtkVBox
	GtkWidget *button_add_to_playlist;
};

enum
{
	PROP_0,
	PROP_TYPE,
	PROP_PARENT
};

enum
{
	COLUMN_CHECK,
	COLUMN_SONG,
	COLUMN_ARTIST,
	COLUMN_ALBUM,
	COLUMN_FROM,
	NUM_COLUMNS
};

static void
update_add_to_playlist_status(XmrList *list)
{
	gboolean checked = FALSE;
	FOR_EACH_ROW(list->priv->treeview)
	{
		gtk_tree_model_get(model, &iter, COLUMN_CHECK, &checked, -1);
		if (checked)
			break;
	}
	
	gtk_widget_set_sensitive(list->priv->button_add_to_playlist, checked);
}

static void
on_button_select_all_clicked(GtkButton *button, XmrList *list)
{
	FOR_EACH_ROW(list->priv->treeview)
	{
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, COLUMN_CHECK, TRUE, -1);
	}
	
	update_add_to_playlist_status(list);
}

static void
on_button_invert_selection_clicked(GtkButton *button, XmrList *list)
{
	FOR_EACH_ROW(list->priv->treeview)
	{
		gboolean checked = FALSE;
		gtk_tree_model_get(model, &iter, COLUMN_CHECK, &checked, -1);
	
		checked ^= 1;
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, COLUMN_CHECK, checked, -1);
	}
	update_add_to_playlist_status(list);
}

static void
on_button_add_to_playlist_clicked(GtkButton *button, XmrList *list)
{
	FOR_EACH_ROW(list->priv->treeview)
	{
		gboolean checked = FALSE;
		gtk_tree_model_get(model, &iter, COLUMN_CHECK, &checked, -1);
	
		if (checked)
		{
			SongInfo *info = g_list_nth_data(list->priv->list, idx);
			if (!xmr_window_add_song(XMR_WINDOW(list->priv->parent), info))
				xmr_debug("Failed to add to playlist, maybe song exists");
		}
	}
}

static void
on_check_toggled(GtkCellRendererToggle *cell,
				 gchar *path_str,
				 XmrList *list)
{
	GtkTreeView *treeview = GTK_TREE_VIEW(list->priv->treeview);
	GtkTreeModel *model = gtk_tree_view_get_model(treeview);
	GtkTreeIter  iter;
	GtkTreePath *path = gtk_tree_path_new_from_string(path_str);
	gboolean checked = FALSE;
	
	gtk_tree_model_get_iter (model, &iter, path);
	gtk_tree_model_get(model, &iter, COLUMN_CHECK, &checked, -1);
	
	checked ^= 1;
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, COLUMN_CHECK, checked, -1);
	
	gtk_tree_path_free(path);
	
	update_add_to_playlist_status(list);
}

static void
on_row_activated(GtkTreeView *treeview,
				 GtkTreePath *path,
				 GtkTreeViewColumn *column,
				 XmrList *list)
{
	gint *indices = gtk_tree_path_get_indices(path) ;
	if (indices)
	{
		SongInfo *info = g_list_nth_data(list->priv->list, indices[0]);
		if (!xmr_window_add_song_and_play(XMR_WINDOW(list->priv->parent), info))
			xmr_debug("Unable play song");
	}
}

static gboolean
on_delete_event(GtkWidget *widget,
						 GdkEvent  *event,
						 gpointer   data)
{
	gtk_widget_hide(widget);

	xmr_list_clear(XMR_LIST(widget));

	return TRUE;
}

GType
xmr_list_type()
{
	static GType etype = 0;

	if (G_UNLIKELY(etype == 0))
	{
		static const GEnumValue values[] =
		{
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
		if (priv->type == XMR_SEARCH_LIST)
		{
			GtkWidget *button_sel_all;
			GtkWidget *button_invert_selection;
			GtkWidget *hbox;
			
			button_sel_all = gtk_button_new_with_label(_("Select All"));
			button_invert_selection = gtk_button_new_with_label(_("Invert Selection"));
			priv->button_add_to_playlist = gtk_button_new_with_label(_("Add to Playlist"));
			
			gtk_button_set_relief(GTK_BUTTON(button_sel_all), GTK_RELIEF_NONE);
			gtk_button_set_relief(GTK_BUTTON(button_invert_selection), GTK_RELIEF_NONE);
			gtk_button_set_relief(GTK_BUTTON(priv->button_add_to_playlist), GTK_RELIEF_NONE);
			
			hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
			gtk_box_pack_start(GTK_BOX(hbox), button_sel_all, FALSE, FALSE, 0);
			gtk_box_pack_start(GTK_BOX(hbox), button_invert_selection, FALSE, FALSE, 0);
			gtk_box_pack_start(GTK_BOX(hbox), priv->button_add_to_playlist, FALSE, FALSE, 0);
			
			gtk_box_pack_start(GTK_BOX(priv->layout), hbox, FALSE, FALSE, 0);
			
			g_signal_connect(button_sel_all, "clicked", G_CALLBACK(on_button_select_all_clicked), list);
			g_signal_connect(button_invert_selection, "clicked", G_CALLBACK(on_button_invert_selection_clicked), list);
			g_signal_connect(priv->button_add_to_playlist, "clicked", G_CALLBACK(on_button_add_to_playlist_clicked), list);
			
			g_signal_connect(list->priv->treeview, "row-activated", G_CALLBACK(on_row_activated), list);
		}
		else
		{
			GtkTreeViewColumn *column;
			// hide these tow cols
			column = gtk_tree_view_get_column(GTK_TREE_VIEW(priv->treeview), COLUMN_CHECK);
			gtk_tree_view_column_set_visible(column, FALSE);
			
			column = gtk_tree_view_get_column(GTK_TREE_VIEW(priv->treeview), COLUMN_FROM);
			gtk_tree_view_column_set_visible(column, FALSE);
		}
		break;
			
	case PROP_PARENT:
		priv->parent = g_value_get_object(value);
		gtk_window_set_transient_for(GTK_WINDOW(list), priv->parent);
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
		
	case PROP_PARENT:
		g_value_set_object(value, priv->parent);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
init_columns(XmrList *list)
{
	GtkTreeView *treeview;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	gint i;
	
	const gchar *column_names[] =
	{
		"?", _("Song Name"), _("Artist"), _("Album"), _("From")
	};
	
	treeview = GTK_TREE_VIEW(list->priv->treeview);
	
	renderer = gtk_cell_renderer_toggle_new();
	column = gtk_tree_view_column_new_with_attributes(column_names[0],
													  renderer,
													  "active",
													  COLUMN_CHECK,
													  NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column(treeview, column);
	
	g_signal_connect(renderer, "toggled", G_CALLBACK (on_check_toggled), list);

	for (i = COLUMN_SONG; i < NUM_COLUMNS; ++i)
	{
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(column_names[i],
														  renderer,
														  "text",
														  i,
														  NULL);
		gtk_tree_view_column_set_resizable(column, TRUE);
		gtk_tree_view_append_column(treeview, column);
	}
}

static void
xmr_list_init(XmrList *list)
{
	XmrListPrivate *priv;
	GtkWidget *sw;
	GtkListStore *store;

	priv = list->priv = G_TYPE_INSTANCE_GET_PRIVATE(list, XMR_TYPE_LIST, XmrListPrivate);
	
	priv->list = NULL;
	store = gtk_list_store_new(NUM_COLUMNS,
									 G_TYPE_BOOLEAN,
									 G_TYPE_STRING,
									 G_TYPE_STRING,
									 G_TYPE_STRING,
									 G_TYPE_STRING);
	priv->treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	init_columns(list);
	
	priv->layout = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_container_add(GTK_CONTAINER(list), priv->layout);
	gtk_container_set_border_width(GTK_CONTAINER(list), 5);
	
	sw = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
									GTK_POLICY_AUTOMATIC,
									GTK_POLICY_AUTOMATIC);

	gtk_box_pack_start(GTK_BOX(priv->layout), sw, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(sw), priv->treeview);
	
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(list), FALSE);
	gtk_window_set_position(GTK_WINDOW(list), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(list), 500, 200);
	
	g_signal_connect(list, "delete-event", G_CALLBACK(on_delete_event), NULL);
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
	
	g_object_class_install_property(object_class, PROP_PARENT,
				g_param_spec_object("parent-window",
					"GtkWindow",
					"The parent window",
					GTK_TYPE_WINDOW,
					G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	g_type_class_add_private(object_class, sizeof(XmrListPrivate));
}

GtkWidget *
xmr_list_new(XmrListType type, GtkWindow *parent)
{
	return g_object_new(XMR_TYPE_LIST,
						"type", GTK_WINDOW_TOPLEVEL,
						"parent-window", parent,
						"list-type", type,
						"title", type == XMR_PLAYLIST ? _("Playlist") : _("Search result"),
						NULL);
}

static gpointer
copy_list_func(gconstpointer src,
				 gpointer data)
{
	return song_info_copy((SongInfo *)src);
}

void
xmr_list_append(XmrList *list, GList *song_list, const gchar *from, const gchar *link)
{
	XmrListPrivate *priv;
	GtkTreeIter iter;
	GtkTreeModel *model;
	GList *new_list = NULL;
	GList *p = song_list;

	g_return_if_fail(list != NULL);
	priv = list->priv;
	
	new_list = g_list_copy_deep(song_list, (GCopyFunc)copy_list_func, NULL);
	priv->list = g_list_concat(priv->list, new_list);
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(priv->treeview));
	while (p)
	{
		SongInfo *info = (SongInfo *)p->data;
		gtk_list_store_append(GTK_LIST_STORE(model), &iter);
		gtk_list_store_set(GTK_LIST_STORE(model), &iter,
						   COLUMN_CHECK, TRUE,
						   COLUMN_SONG, info->song_name,
						   COLUMN_ARTIST, info->artist_name,
						   COLUMN_ALBUM, info->album_name,
						   COLUMN_FROM, from,
						   -1);
		
		p = p->next;
	}
	
	gtk_widget_set_sensitive(priv->button_add_to_playlist, TRUE);
}

void
xmr_list_clear(XmrList *list)
{
	XmrListPrivate *priv;
	GtkTreeModel *model;

	g_return_if_fail(list != NULL);
	priv = list->priv;
	
	g_list_free_full(priv->list, (GDestroyNotify)song_info_free);
	priv->list = NULL;
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(priv->treeview));
	gtk_list_store_clear(GTK_LIST_STORE(model));
}
