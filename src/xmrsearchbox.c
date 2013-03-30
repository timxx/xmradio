/** 
 * xmrsearchbox.c
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

#include <glib/gi18n.h>
#include <stdlib.h>
#include <ctype.h>
#include <curl/curl.h>

#include "xmrsearchbox.h"
#include "xmrdebug.h"
#include "lib/xmrservice.h"
#include "config.h"
#include "xmrchooser.h"
#include "xmrartist.h"
#include "xmrwindow.h"

G_DEFINE_TYPE(XmrSearchBox, xmr_search_box, GTK_TYPE_WINDOW)

#define SEARCH_URL	"http://www.xiami.com/search/artist?key="
#define RADIO_URL	"http://www.xiami.com/radio/xml/type/5/id/"

#define ARTIST_PATTERN "href=\"/artist/"
#define IMG_SRC_PATTERN "<img src=\""

struct _XmrSearchBoxPrivate
{
	GtkWindow *parent;
	gboolean need_to_show; /* flag to follow parent window show/hide */
	
	GtkWidget *entry_box;
	GThread *thread;
	
	XmrChooser *chooser;
	GAsyncQueue *event_queue;
	guint event_idle_id;
	guint progress_idle_id;
};

typedef struct
{
	XmrSearchBox *box;
	gchar *keyword;
}SearchData;

typedef struct
{
	guint type;
	gpointer event;
}SearchEvent;

enum
{
	PROP_0,
	PROP_PARENT
};

enum
{
	EVENT_APPEND,
	EVENT_NOT_FOUND,
	EVENT_NETWORK_ERROR,
	EVENT_FINISH
};

static void
show_chooser(XmrSearchBox *box, gboolean show);

static void
on_artist_clicked(XmrChooser *chooser,
				  XmrArtist *artist,
				  XmrSearchBox *box);


static gboolean
search_progress_idle(GtkEntry *entry);

static void
search_progress_done(GtkEntry *entry);

static void
show_progress(XmrSearchBox *box, gboolean show);

static void
post_event(XmrSearchBox *box, guint type, gpointer event)
{
	SearchEvent *e = g_new(SearchEvent, 1);
	e->type = type;
	e->event = event;

	g_async_queue_push(box->priv->event_queue, e);
	g_main_context_wakeup(g_main_context_default());
}

static gboolean
event_poll(XmrSearchBox *box)
{
	XmrSearchBoxPrivate *priv = box->priv;
	SearchEvent *e = g_async_queue_try_pop(priv->event_queue);
	if (e == NULL)
		return TRUE;

	switch (e->type)
	{
	case EVENT_APPEND:
	{
		GtkWidget *widget = xmr_artist_new_with_info((Artist*)e->event);
		xmr_chooser_append(priv->chooser, widget);
		artist_free((Artist *)e->event);
		e->event = NULL;
		
		if (!gtk_widget_get_visible(GTK_WIDGET(priv->chooser)))
			show_chooser(box, TRUE);
	}
		break;
		
	case EVENT_NOT_FOUND:
		gtk_entry_set_text(GTK_ENTRY(priv->entry_box), _("Oops! Not found any related artist."));
		break;
		
	case EVENT_NETWORK_ERROR:
		gtk_entry_set_text(GTK_ENTRY(priv->entry_box), _("Oops! Network error."));
		break;
		
	case EVENT_FINISH:
#if GLIB_CHECK_VERSION(2, 32, 0)
		g_thread_join(priv->thread);
#endif
		priv->thread = NULL;
		g_source_remove(priv->event_idle_id);
		priv->event_idle_id = 0;
		
		show_progress(box, FALSE);
		break;
	}

	g_free(e->event);
	g_free(e);
	return TRUE;
}

static gboolean
on_key_release(XmrSearchBox	*box,
			   GdkEventKey	*event,
			   gpointer		 data)
{
	if (event->keyval == GDK_KEY_Escape)
	{
		box->priv->need_to_show = FALSE;
		gtk_widget_hide(GTK_WIDGET(box));
		gtk_widget_hide(GTK_WIDGET(box->priv->chooser));
	}

	return FALSE;
}

static void
append_artist(XmrSearchBox *box,
			  const gchar *id,
			  const gchar *img_src,
			  const gchar *name,
			  const gchar *region)
{
	Artist *artist = g_new0(Artist, 1);
#ifdef _DEBUG
	g_print("======================\n");
	g_print("id: %s\n", id);
	g_print("img: %s\n", img_src);
	g_print("name: %s\n", name);
	g_print("region: %s\n", region);
	g_print("======================\n");
#endif
	
	artist->id = g_strdup(id);
	artist->name = g_strdup(name);
	artist->region = g_strdup(region);
	artist->img_src = g_strdup(img_src);
	
	post_event(box, EVENT_APPEND, artist);
}

static gchar *
get_artist_name(const gchar *str, gint length)
{
	gchar *name = g_malloc0(length * sizeof(gchar));
	gint i = 0, j;

	for (j = 0; j < length; ++j)
	{
		if (str[j] == '<')
		{
			for ( ; j < length; ++j)
			{
				if (str[j] == '>')
				{
					j++;
					break;
				}
			}
		}
		if (j < length)
		{
			if (str[j] == '<') // skip "<...>" in next loop
			{
				--j;
				continue ;
			}
			name[i++] = str[j];
		}
	}
	
	return name;
}

static void
parse_data(XmrSearchBox *box, GString *data)
{
	const gchar *current = data->str;
	const gchar *p;
	gboolean found_one = FALSE;
	
	while ((p = strstr(current, ARTIST_PATTERN)) != NULL)
	{
		gchar *artist_id = NULL;
		gchar *img_src = NULL;
		gchar *artist_name = NULL;
		gchar *artist_region = NULL;

		current = p + strlen(ARTIST_PATTERN);
		// PATTERN: href="/artist/XXXXXXX". 'X' must be number 
		if (!isdigit(*current))
			continue;
		
		p = strchr(current, '\"');
		if (p == NULL)
			break;

		// artist id
		artist_id = g_strndup(current, p - current);
		if (artist_id == NULL)
			continue ;
		
		current = p + 1;
		p = strstr(current, IMG_SRC_PATTERN);
		if (p != NULL)
		{
			current = p + strlen(IMG_SRC_PATTERN);
			p = strchr(current, '\"');
			if (p == NULL)
			{
				g_free(artist_id);
				break;
			}
			
			img_src = g_strndup(current, p - current);
			current = p + 1;
			
			// download img_src to tmp file
			{
				XmrService *srv = xmr_service_new();
				GString *img_data = g_string_new("");
				gint result = xmr_service_get_url_data(srv, img_src, img_data);
				if (result == CURLE_OK)
				{
					gchar *tmp_file = g_malloc0(260);
					strcpy(tmp_file, "/tmp/xmradio-img-srcXXXXXX");
					int fd = mkstemp(tmp_file);
					
					write(fd, img_data->str, img_data->len * sizeof(gchar));
					close(fd);
					
					g_free(img_src);
					img_src = tmp_file;
				}
				else
				{
					g_free(img_src);
					img_src = NULL;
				}
				
				g_string_free(img_data, TRUE);
				g_object_unref(srv);
			}
		}
		
		// artist name
		// FIXME: maybe wrong someday...
		p = strstr(current, "<strong>");
		if (p != NULL)
		{
			current = p + strlen("<strong>");
			p = strstr(current, "</strong>");
			if (p != NULL)
			{
				artist_name = get_artist_name(current, p - current);
				current = p + 1;
			}
		}
		
		p = strstr(current, "singer_region\">");
		if (p != NULL)
		{
			current = p + strlen("singer_region\">");
			p = strchr(current, '<');
			if (p != NULL)
			{
				artist_region = g_strndup(current, p - current);
				current = p + 1;
			}
		}
		
		found_one = TRUE;
		append_artist(box, artist_id, img_src, artist_name, artist_region);
		
		g_free(artist_id);
		g_free(img_src);
		g_free(artist_name);
		g_free(artist_region);
	}
	
	if (!found_one)
	{
		post_event(box, EVENT_NOT_FOUND, NULL);
	}
}

static gpointer
search_artist_thread(SearchData *data)
{
	XmrService *srv = xmr_service_new();
	GString *result_data = g_string_new("");
	gchar *escape_keyword = curl_escape(data->keyword, 0);
	gchar *url = g_strdup_printf(SEARCH_URL"%s", escape_keyword);
	
	gint result = xmr_service_get_url_data(srv, url, result_data);
	g_object_unref(srv);
	
	if (result == CURLE_OK)
		parse_data(data->box, result_data);
	else
		post_event(data->box, EVENT_NETWORK_ERROR, NULL);
	
	g_free(url);
	curl_free(escape_keyword);
	g_string_free(result_data, TRUE);
	
	post_event(data->box, EVENT_FINISH, NULL);
	
	g_free(data->keyword);
	g_free(data);

	return NULL;
}

static void
on_search_box_activate(GtkEntry  *entry,
					   XmrSearchBox *box)
{
	const gchar *text;
	SearchData *data;
	XmrSearchBoxPrivate *priv = box->priv;

	if (gtk_entry_get_text_length(entry) == 0)
		return ;

	text = gtk_entry_get_text(entry);
	
	xmr_debug("search artist: %s", text);
	if (priv->thread != NULL)
	{
	}
	
	if (priv->event_idle_id == 0)
	{
		priv->event_idle_id = g_idle_add((GSourceFunc)event_poll, box);
	}
	
	show_progress(box, TRUE);
	
	// clear any prev result
	xmr_chooser_clear(priv->chooser);
	
	data = g_new0(SearchData, 1);
	data->box = box;
	data->keyword = g_strdup(text);
#if GLIB_CHECK_VERSION(2, 32, 0)
	priv->thread = g_thread_new("search_artist", (GThreadFunc)search_artist_thread, data);
#else
	priv->thread = g_thread_create((GThreadFunc)search_artist_thread, data, FALSE, NULL);
#endif
}

static void
on_parent_hide(GtkWindow *parent, XmrSearchBox *box)
{
	gtk_widget_hide(GTK_WIDGET(box));
	show_chooser(box, FALSE);
}

static void
on_parent_show(GtkWindow *parent, XmrSearchBox *box)
{
	if (box->priv->need_to_show)
		gtk_widget_show(GTK_WIDGET(box));
}

static void
xmr_search_box_finalize(GObject *obj)
{
	XmrSearchBox *box = XMR_SEARCH_BOX(obj);
	XmrSearchBoxPrivate *priv = box->priv;

	g_async_queue_unref(priv->event_queue);
	if (priv->event_idle_id != 0)
		g_source_remove(priv->event_idle_id);

	G_OBJECT_CLASS(xmr_search_box_parent_class)->finalize(obj);
}

static void
xmr_search_box_get_property(GObject *object,
							 guint prop_id,
							 GValue *value,
							 GParamSpec *pspec)
{
	XmrSearchBox *box = XMR_SEARCH_BOX(object);
	XmrSearchBoxPrivate *priv = box->priv;

	switch(prop_id)
	{
	case PROP_PARENT:
		g_value_set_object(value, priv->parent);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
xmr_search_box_set_property(GObject *object,
							 guint prop_id,
							 const GValue *value,
							 GParamSpec *pspec)
{
	XmrSearchBox *box = XMR_SEARCH_BOX(object);
	XmrSearchBoxPrivate *priv = box->priv;

	switch(prop_id)
	{
	case PROP_PARENT:
		priv->parent = g_value_get_object(value);
		gtk_window_set_transient_for(GTK_WINDOW(object), priv->parent);
		g_signal_connect(priv->parent, "hide", G_CALLBACK(on_parent_hide), box);
		g_signal_connect(priv->parent, "show", G_CALLBACK(on_parent_show), box);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
xmr_search_box_class_init(XmrSearchBoxClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	
	object_class->finalize = xmr_search_box_finalize;
	object_class->set_property = xmr_search_box_set_property;
	object_class->get_property = xmr_search_box_get_property;
	
	g_object_class_install_property(object_class,
				PROP_PARENT,
				g_param_spec_object("parent",
					"Parent",
					"Parent window",
					GTK_TYPE_WINDOW,
					G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	
	g_type_class_add_private(object_class, sizeof(XmrSearchBoxPrivate));
}

static gboolean
on_delete(XmrSearchBox *box, GdkEvent *event, gpointer data)
{
	XmrSearchBoxPrivate *priv = box->priv;
	
	priv->need_to_show = FALSE;
	show_chooser(box, FALSE);
	return gtk_widget_hide_on_delete(GTK_WIDGET(box));
}

static void
xmr_search_box_init(XmrSearchBox *box)
{
	XmrSearchBoxPrivate *priv;
	GtkWidget *vbox;
	
	priv = box->priv = G_TYPE_INSTANCE_GET_PRIVATE(box, XMR_TYPE_SEARCH_BOX, XmrSearchBoxPrivate);
	priv->parent = NULL;
	priv->need_to_show = FALSE;
	priv->thread = NULL;
	priv->chooser = XMR_CHOOSER(xmr_chooser_new(_("Artist Radio"), GTK_ORIENTATION_VERTICAL));
	priv->event_queue = g_async_queue_new();
	priv->event_idle_id = 0;
	
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	gtk_container_add(GTK_CONTAINER(box), vbox);
	
	priv->entry_box = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(vbox), priv->entry_box, TRUE, TRUE, 0);
	
	gtk_entry_set_icon_from_stock(GTK_ENTRY(priv->entry_box), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_FIND);
	gtk_entry_set_placeholder_text(GTK_ENTRY(priv->entry_box), _("Enter artist name..."));
	
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(box), TRUE);
	gtk_window_set_skip_pager_hint(GTK_WINDOW(box), TRUE);
	gtk_window_set_title(GTK_WINDOW(box), _("Search Artist Radio"));
	gtk_widget_set_size_request(GTK_WIDGET(box), 350, 45);
	
	gtk_window_set_decorated(GTK_WINDOW(priv->chooser),  FALSE);
	gtk_window_set_transient_for(GTK_WINDOW(priv->chooser), GTK_WINDOW(box));
	gtk_window_set_resizable(GTK_WINDOW(priv->chooser), FALSE);
	gtk_widget_set_size_request(GTK_WIDGET(priv->chooser), 350, 200);
	xmr_chooser_set_hide_on_clicked(priv->chooser, FALSE);
	
	g_signal_connect(box, "delete-event", G_CALLBACK(on_delete), NULL);
	g_signal_connect(box, "key-release-event", G_CALLBACK(on_key_release), NULL);
	
	g_signal_connect(priv->entry_box, "activate", G_CALLBACK(on_search_box_activate), box);
	
	g_signal_connect(priv->chooser, "widget-selected", G_CALLBACK(on_artist_clicked), box);
	
	gtk_window_set_position(GTK_WINDOW(box), GTK_WIN_POS_CENTER);
	gtk_widget_show_all(vbox);
}

GtkWidget *
xmr_search_box_new(GtkWindow *parent)
{
	return g_object_new(XMR_TYPE_SEARCH_BOX,
						"type", GTK_WINDOW_TOPLEVEL,
						"resizable", FALSE,
						"parent", parent,
						NULL);
}

void
xmr_search_box_show(XmrSearchBox *box)
{
	g_return_if_fail(box != NULL);

	xmr_chooser_clear(box->priv->chooser);
	gtk_widget_hide(GTK_WIDGET(box->priv->chooser));

	box->priv->need_to_show = TRUE;
	gtk_widget_show(GTK_WIDGET(box));
}

static void
show_chooser(XmrSearchBox *box, gboolean show)
{
	XmrSearchBoxPrivate *priv = box->priv;
	if (show)
	{
		gint x, y, w, h;
		
		gtk_window_get_position(GTK_WINDOW(box), &x, &y);
		gtk_window_get_size(GTK_WINDOW(box), &w, &h);
		
		gtk_window_move(GTK_WINDOW(priv->chooser), x, y + h + 30);
		
		xmr_chooser_show(priv->chooser);
	}
	else
	{
		gtk_widget_hide(GTK_WIDGET(priv->chooser));
	}
}

static void
on_artist_clicked(XmrChooser *chooser,
				  XmrArtist *artist,
				  XmrSearchBox *box)
{
	const gchar *name = xmr_artist_get_name(artist);
	const gchar *id = xmr_artist_get_id(artist);

	gchar *radio_name = g_strdup_printf("%s - %s", _("Artist Radio"), name);
	gchar *url = g_strdup_printf(RADIO_URL"%s", id);

	xmr_window_play_custom_radio(XMR_WINDOW(box->priv->parent), radio_name, url);
	
	g_free(radio_name);
	g_free(url);
}

static gboolean
search_progress_idle(GtkEntry *entry)
{
	gtk_entry_progress_pulse(entry);

	return TRUE;
}

static void
search_progress_done(GtkEntry *entry)
{
	gtk_entry_set_progress_fraction(entry, 0.0);
}

static void
show_progress(XmrSearchBox *box, gboolean show)
{
	XmrSearchBoxPrivate *priv = box->priv;
	
	if (show)
	{
		priv->progress_idle_id = g_timeout_add_full(
					G_PRIORITY_DEFAULT, 100,
					(GSourceFunc)search_progress_idle, priv->entry_box,
					(GDestroyNotify)search_progress_done);
	}
	else
	{
		g_source_remove(priv->progress_idle_id);
	}
}

