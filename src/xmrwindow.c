/**
 * xmrwindow.c
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
#include <glib/gi18n.h>
#include <stdlib.h>

#include "xmrwindow.h"
#include "xmrplayer.h"
#include "xmrbutton.h"
#include "xmrskin.h"
#include "xmrdebug.h"
#include "xmrplayer.h"
#include "lib/xmrservice.h"
#include "config.h"
#include "xmrutil.h"

G_DEFINE_TYPE(XmrWindow, xmr_window, GTK_TYPE_WINDOW);

#define DEFAULT_RADIO_URL	"http://www.xiami.com/kuang/xml/type/6/id/0"
#define COVER_WIDTH	100
#define COVER_HEIGHT 100

enum
{
	BUTTON_CLOSE = 0,	// X button
	BUTTON_MINIMIZE,	// - button
	BUTTON_PLAY,		// play
	BUTTON_PAUSE,
	BUTTON_NEXT,
	BUTTON_LIKE,
	BUTTON_DISLIKE,
	BUTTON_VOLUME,
	BUTTON_LYRIC,
	BUTTON_DOWNLOAD,
	BUTTON_SHARE,
	BUTTON_SIREN,
	BUTTON_FENGGE,
	BUTTON_XINGZUO,
	BUTTON_NIANDAI,
	LAST_BUTTON
};

enum
{
	LABEL_RADIO = 0,
	LABEL_SONG_NAME,
	LABEL_ARTIST,
	LABEL_TIME,
	LAST_LABEL
};

struct _XmrWindowPrivate
{
	gchar		*skin;
	gboolean	gtk_theme;	/* TRUE to use gtk theme rather than skin */

	GtkWidget	*buttons[LAST_BUTTON];	/* ui buttons */
	GtkWidget	*labels[LAST_LABEL];
	GtkWidget	*image;	/* album cover image */

	cairo_surface_t	*cs_bkgnd;	/* backgroud image surface */

	GtkWidget	*fixed;			/* #GtkFixed */
	GtkWidget	*popup_menu;	/* #GtkMenu */
	GtkWidget	*skin_menu;

	XmrPlayer *player;
	XmrService *service;

	GList *playlist;
	gchar *playlist_url;	/* set NULL to get private list */
};

enum
{
	PROP_0,
	PROP_PLAYER,
	PROP_SERVICE,
	PROP_SKIN,
	PROP_GTK_THEME,
	PROP_MENU_POPUP,
	PROP_LAYOUT
};

enum
{
	THEME_CHANGED,
	TRACK_CHANGED,
	RADIO_CHANGED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static void
xmr_window_dispose(GObject *obj);

static void
xmr_window_finalize(GObject *obj);

static void
xmr_window_get_property(GObject *object,
		       guint prop_id,
		       GValue *value,
		       GParamSpec *pspec);

static void
xmr_window_set_property(GObject *object,
		       guint prop_id,
		       const GValue *value,
		       GParamSpec *pspec);

static gboolean
on_draw(XmrWindow *window, cairo_t *cr, gpointer data);

static gboolean
on_button_press(XmrWindow *window, GdkEventButton *event, gpointer data);

static void
on_xmr_button_clicked(GtkWidget *widget, gpointer data);

static void
set_window_image(XmrWindow *window, GdkPixbuf *pixbuf);

static void
set_cover_image(XmrWindow *window, GdkPixbuf *pixbuf);

static void
set_gtk_theme(XmrWindow *window);

static void
set_skin(XmrWindow *window, const gchar *skin);

static void
hide_children(XmrWindow *window);

static void
player_eos(XmrPlayer *player,
			gboolean early,
			XmrWindow *window);

static void
player_error(XmrPlayer *player,
			GError *error,
			XmrWindow *window);

static void
player_tick(XmrPlayer *player,
			gint64 elapsed,
			gint64 duration,
			XmrWindow *window);

static void
player_buffering(XmrPlayer *player,
			guint progress,
			XmrWindow *window);

static void
player_state_changed(XmrPlayer *player,
			gint old_state,
			gint new_state,
			XmrWindow *window);

static gboolean
thread_finish(GThread *thread);

static gpointer
thread_get_playlist(XmrWindow *window);

static gpointer
thread_get_cover_image(XmrWindow *window);

static void
xmr_window_get_playlist(XmrWindow *window);

static void
xmr_window_get_cover_image(XmrWindow *window);

static void
xmr_window_set_track_info(XmrWindow *window);

static void
create_popup_menu(XmrWindow *window);

static void
on_menu_item_activate(GtkMenuItem *item, XmrWindow *window);

static void
install_properties(GObjectClass *object_class)
{
	g_object_class_install_property(object_class,
				PROP_PLAYER,
				g_param_spec_object("player",
					"Player",
					"Player object",
					G_TYPE_OBJECT,
					G_PARAM_READABLE));

	g_object_class_install_property(object_class,
				PROP_SERVICE,
				g_param_spec_object("service",
					"Service",
					"XmrService object",
					G_TYPE_OBJECT,
					G_PARAM_READABLE));

	g_object_class_install_property(object_class,
				PROP_SKIN,
				g_param_spec_string("skin",
					"Skin",
					"Skin",
					NULL,
					G_PARAM_READWRITE));

	g_object_class_install_property(object_class,
				PROP_GTK_THEME,
				g_param_spec_boolean("gtk-theme",
					"Gtk theme",
					"Use gtk theme or not",
					FALSE,
					G_PARAM_READWRITE));

	g_object_class_install_property(object_class,
				PROP_GTK_THEME,
				g_param_spec_object("layout",
					"Layout",
					"Window layout",
					G_TYPE_OBJECT,
					G_PARAM_READABLE));

	g_object_class_install_property(object_class,
				PROP_MENU_POPUP,
				g_param_spec_object("menu-popup",
					"Popup menu",
					"Popup menu",
					G_TYPE_OBJECT,
					G_PARAM_READABLE));
}

static void
create_signals(XmrWindowClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	/**
	 * emit when user changed skin
	 * when @new_theme is NULL, it means
	 * UI use GTK theme
	 */
	signals[THEME_CHANGED] =
		g_signal_new("theme-changed",
					G_OBJECT_CLASS_TYPE(object_class),
					G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
					G_STRUCT_OFFSET(XmrWindowClass, theme_changed),
					NULL, NULL,
					g_cclosure_marshal_VOID__STRING,
					G_TYPE_NONE,
					1,
					G_TYPE_STRING);

	/**
	 * emit when current track changed
	 */
	signals[TRACK_CHANGED] =
		g_signal_new("track-changed",
					G_OBJECT_CLASS_TYPE(object_class),
					G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
					G_STRUCT_OFFSET(XmrWindowClass, track_changed),
					NULL, NULL,
					g_cclosure_marshal_VOID__POINTER,
					G_TYPE_NONE,
					1,
					G_TYPE_POINTER);

	/**
	 * emit when current radio changed
	 */
	signals[RADIO_CHANGED] =
		g_signal_new("radio-changed",
					G_OBJECT_CLASS_TYPE(object_class),
					G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
					G_STRUCT_OFFSET(XmrWindowClass, radio_changed),
					NULL, NULL,
					g_cclosure_marshal_VOID__POINTER,
					G_TYPE_NONE,
					1,
					G_TYPE_POINTER);
}

static void 
xmr_window_class_init(XmrWindowClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->set_property = xmr_window_set_property;
	object_class->get_property = xmr_window_get_property;

	object_class->dispose = xmr_window_dispose;
	object_class->finalize = xmr_window_finalize;

	install_properties(object_class);
	create_signals(klass);

	g_type_class_add_private(object_class, sizeof(XmrWindowPrivate));
}

static void 
xmr_window_init(XmrWindow *window)
{
	XmrWindowPrivate *priv;
	gint i;

	window->priv = G_TYPE_INSTANCE_GET_PRIVATE(window, XMR_TYPE_WINDOW, XmrWindowPrivate);
	priv = window->priv;

	priv->player = xmr_player_new();
	priv->service = xmr_service_new();
	priv->skin = NULL;
	priv->gtk_theme = FALSE;
	priv->cs_bkgnd = NULL;
	priv->playlist = NULL;
	priv->playlist_url = g_strdup(DEFAULT_RADIO_URL);

	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_widget_set_app_paintable(GTK_WIDGET(window), TRUE);
	gtk_widget_add_events(GTK_WIDGET(window), GDK_BUTTON_PRESS_MASK);

	gtk_window_set_default_icon_name("xmradio");

	priv->fixed = gtk_fixed_new();
	create_popup_menu(window);

	for(i=0; i<LAST_BUTTON; ++i)
	{
		priv->buttons[i] = xmr_button_new(XMR_BUTTON_SKIN);
		gtk_fixed_put(GTK_FIXED(priv->fixed), priv->buttons[i], 0, 0);

		g_signal_connect(priv->buttons[i], "clicked",
					G_CALLBACK(on_xmr_button_clicked), (gpointer)(glong)i);
	}

	for(i=0; i<LAST_LABEL; ++i)
	{
		priv->labels[i] = gtk_label_new("");
		gtk_fixed_put(GTK_FIXED(priv->fixed), priv->labels[i], 0, 0);
	}

	priv->image = gtk_image_new();
	gtk_fixed_put(GTK_FIXED(priv->fixed), priv->image, 0, 0);

	gtk_container_add(GTK_CONTAINER(window), priv->fixed);

	gtk_widget_show(priv->fixed);

	g_signal_connect(window, "draw", G_CALLBACK(on_draw), NULL);
	g_signal_connect(window, "button-press-event", G_CALLBACK(on_button_press), NULL);

	g_signal_connect(priv->player, "eos", G_CALLBACK(player_eos), window);
	g_signal_connect(priv->player, "error", G_CALLBACK(player_error), window);
	g_signal_connect(priv->player, "tick", G_CALLBACK(player_tick), window);
	g_signal_connect(priv->player, "buffering", G_CALLBACK(player_buffering), window);
	g_signal_connect(priv->player, "state-changed", G_CALLBACK(player_state_changed), window);

	// set_gtk_theme(window);
	set_skin(window, "../data/skin/pure.skn");

	gtk_widget_hide(priv->buttons[BUTTON_PAUSE]);

	xmr_window_get_playlist(window);
}

GtkWidget* xmr_window_new()
{
	 return g_object_new(XMR_TYPE_WINDOW,
				"type", GTK_WINDOW_TOPLEVEL,
				"title", _("XMRadio"),
				"resizable", FALSE,
				NULL);
}

static void
xmr_window_dispose(GObject *obj)
{
	XmrWindow *window = XMR_WINDOW(obj);
	XmrWindowPrivate *priv = window->priv;

	if (priv->player != NULL)
	{
		g_object_unref(priv->player);
		priv->player = NULL;
	}
	if (priv->service != NULL)
	{
		g_object_unref(priv->service);
		priv->service = NULL;
	}

	G_OBJECT_CLASS(xmr_window_parent_class)->dispose(obj);
}

static void
xmr_window_finalize(GObject *obj)
{
	XmrWindow *window = XMR_WINDOW(obj);
	XmrWindowPrivate *priv = window->priv;

	if (priv->skin)
		g_free(priv->skin);

	if (priv->cs_bkgnd)
		cairo_surface_destroy(priv->cs_bkgnd);

	if (priv->playlist_url)
		g_free(priv->playlist_url);

	if (priv->playlist)
		g_list_free_full(priv->playlist, (GDestroyNotify)song_info_free);

	G_OBJECT_CLASS(xmr_window_parent_class)->finalize(obj);
}

static void
xmr_window_get_property(GObject *object,
		       guint prop_id,
		       GValue *value,
		       GParamSpec *pspec)
{
	XmrWindow *window = XMR_WINDOW(object);
	XmrWindowPrivate *priv = window->priv;

	switch(prop_id)
	{
	case PROP_PLAYER:
		g_value_set_object(value, priv->player);
		break;

	case PROP_SERVICE:
		g_value_set_object(value, priv->service);
		break;

	case PROP_SKIN:
		g_value_set_string(value, priv->skin);
		break;

	case PROP_GTK_THEME:
		g_value_set_boolean(value, priv->gtk_theme);
		break;

	case PROP_LAYOUT:
		g_value_set_object(value, priv->fixed);
		break;

	case PROP_MENU_POPUP:
		g_value_set_object(value, priv->popup_menu);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
xmr_window_set_property(GObject *object,
		       guint prop_id,
		       const GValue *value,
		       GParamSpec *pspec)
{
	XmrWindow *window = XMR_WINDOW(object);
	XmrWindowPrivate *priv = window->priv;

	switch(prop_id)
	{
	case PROP_SKIN:
		g_free(priv->skin);
		priv->skin = g_value_dup_string(value);
		break;

	case PROP_GTK_THEME:
		priv->gtk_theme = g_value_get_boolean(value);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static gboolean
on_draw(XmrWindow *window, cairo_t *cr, gpointer data)
{
	XmrWindowPrivate *priv = window->priv;

	if (priv->gtk_theme || priv->cs_bkgnd == NULL)
		return FALSE;

	cairo_set_source_surface(cr, priv->cs_bkgnd, 0, 0);
	cairo_paint(cr);
	
	return FALSE;
}

static gboolean
on_button_press(XmrWindow *window, GdkEventButton *event, gpointer data)
{
	// ignore if using gtk theme
	if (window->priv->gtk_theme)
		return FALSE;

	if (event->button == 1)  
    {  
        gtk_window_begin_move_drag(GTK_WINDOW(window),
					event->button,
					event->x_root, event->y_root,
					event->time);
    }
	else if (event->button == 3)
	{
		GtkMenu *menu = GTK_MENU(window->priv->popup_menu);
		gtk_menu_popup(menu, NULL, NULL, NULL, NULL,
					event->button, event->time);
		return TRUE;
	}

	return FALSE;
}

static void
on_xmr_button_clicked(GtkWidget *widget, gpointer data)
{
	XmrButton *button = XMR_BUTTON(widget);
	XmrWindow *window = XMR_WINDOW(gtk_widget_get_toplevel(widget));
	XmrWindowPrivate *priv = window->priv;
	glong id = (glong)data;

	switch(id)
	{
	case BUTTON_CLOSE:
		gtk_widget_destroy(GTK_WIDGET(window));
		break;

	case BUTTON_MINIMIZE:
		gtk_window_iconify(GTK_WINDOW(window));
		break;

	case BUTTON_PLAY:
		xmr_player_play(priv->player);
		break;

	case BUTTON_PAUSE:
		xmr_player_pause(priv->player);
		break;

	case BUTTON_NEXT:
		xmr_window_play_next(window);
		break;

	case BUTTON_LIKE:
	case BUTTON_DISLIKE:
	{
		SongInfo *song;
		if (!xmr_service_is_logged_in(priv->service))
		{
			g_warning("You should login first\n");
			break;
		}

		if (priv->playlist == NULL || priv->playlist->data == NULL)
			break;

		song = (SongInfo *)priv->playlist->data;
		xmr_service_like_song(priv->service, song->song_id, id == BUTTON_LIKE);
		if (id == BUTTON_DISLIKE)
			xmr_window_play_next(window);
	}
		break;

	case BUTTON_LYRIC:
		break;

	case BUTTON_DOWNLOAD:
	{
		SongInfo *song = xmr_window_get_current_song(window);
		if (song == NULL)
		{
			xmr_debug("Playlist empty");
		}
		gchar *command = g_strdup_printf("xdg-open http://www.xiami.com/song/%s",
					song->song_id);
		if (command == NULL)
			g_error("No more memory\n");

		system(command);

		g_free(command);
	}
		break;

	case BUTTON_SHARE:
		xmr_debug("Not implemented yet");
		break;
	}
}

static void
set_window_image(XmrWindow *window, GdkPixbuf *pixbuf)
{
	cairo_surface_t *image;
	cairo_t *cr;
	cairo_region_t *mask;
	gint image_width, image_height;
	XmrWindowPrivate *priv;
	
	g_return_if_fail(window != NULL && pixbuf != NULL);
	priv = window->priv;

	image_width = gdk_pixbuf_get_width(pixbuf);
	image_height = gdk_pixbuf_get_height(pixbuf);

	image = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, image_width, image_height);
	if (image == NULL)
		return ;

	cr = cairo_create(image);

	gdk_cairo_set_source_pixbuf(cr, pixbuf, 0, 0);
	cairo_paint(cr);

	cairo_destroy(cr);

	if (priv->cs_bkgnd){
		cairo_surface_destroy(priv->cs_bkgnd);
	}

	priv->cs_bkgnd = image;
	mask = gdk_cairo_region_create_from_surface(priv->cs_bkgnd);
	if (mask == NULL)
		return ;
	gtk_widget_shape_combine_region(GTK_WIDGET(window), mask);

	cairo_region_destroy(mask);

	gtk_widget_set_size_request(GTK_WIDGET(window), image_width, image_height);

	gtk_widget_queue_draw(GTK_WIDGET(window));
}

static void
set_cover_image(XmrWindow *window, GdkPixbuf *pixbuf)
{
	XmrWindowPrivate *priv = window->priv;

	gint i_w, i_h;

	i_w = gdk_pixbuf_get_width(pixbuf);
	i_h = gdk_pixbuf_get_height(pixbuf);

	if (i_w != COVER_WIDTH || i_h != COVER_HEIGHT) //scale
	{
		GdkPixbuf *pb = gdk_pixbuf_scale_simple(pixbuf, COVER_WIDTH, COVER_HEIGHT, GDK_INTERP_BILINEAR);
		if (pb)
		{
			gtk_image_set_from_pixbuf(GTK_IMAGE(priv->image), pb);
			g_object_unref(pb);
		}
	}
	else
	{
		gtk_image_set_from_pixbuf(GTK_IMAGE(priv->image), pixbuf);
	}
}

static void
set_gtk_theme(XmrWindow *window)
{
	XmrWindowPrivate *priv = window->priv;
	struct Pos { gint x, y;	};
	gint i;

	static struct Pos button_pos[LAST_BUTTON] =
	{
		{0, 0}, {0, 0},
		{360, 100}, {430, 100},
		{170, 160}, {210, 160}, {250, 160},
		{330, 170}, {385, 170}, {440, 170},
		{110, 250}, {195, 250}, {280, 250}, {365, 250}
	};

	static struct Pos label_pos[LAST_LABEL] =
	{
		{60, 45}, {175, 90},
		{175, 110}, {175, 140}
	};

	// for testing only
	static gchar *stock_id[] =
	{
		"", "",
		GTK_STOCK_MEDIA_PLAY, GTK_STOCK_MEDIA_NEXT,
		GTK_STOCK_ABOUT, GTK_STOCK_CANCEL, NULL,
		NULL, NULL, NULL,
		NULL, NULL, NULL, NULL
		
	};

	priv->gtk_theme = TRUE;

	gtk_window_set_decorated(GTK_WINDOW(window), TRUE);
	gtk_widget_set_size_request(GTK_WIDGET(window), 540, 250);

	hide_children(window);

	for(i=BUTTON_PLAY; i<LAST_BUTTON; ++i)
	{
		gtk_fixed_move(GTK_FIXED(priv->fixed),
					priv->buttons[i],
					button_pos[i].x, button_pos[i].y);

		xmr_button_set_type(XMR_BUTTON(priv->buttons[i]),
						XMR_BUTTON_NORMAL);

		xmr_button_set_image_from_stock(XMR_BUTTON(priv->buttons[i]),
					stock_id[i]);

		gtk_widget_show(priv->buttons[i]);
	}

	for(i=0; i<LAST_LABEL; ++i)
	{
		gtk_fixed_move(GTK_FIXED(priv->fixed),
					priv->labels[i],
					label_pos[i].x, label_pos[i].y);

		gtk_widget_show(priv->labels[i]);
	}

	gtk_fixed_move(GTK_FIXED(priv->fixed), priv->image, 60, 85);
	gtk_widget_set_size_request(GTK_WIDGET(priv->image), 100, 100);
	gtk_widget_show(priv->image);

	gtk_button_set_label(GTK_BUTTON(priv->buttons[BUTTON_LYRIC]), _("歌词"));
	gtk_button_set_label(GTK_BUTTON(priv->buttons[BUTTON_DOWNLOAD]), _("下载"));
	gtk_button_set_label(GTK_BUTTON(priv->buttons[BUTTON_SHARE]), _("分享"));

	gtk_button_set_label(GTK_BUTTON(priv->buttons[BUTTON_SIREN]), _("私人电台"));
	gtk_button_set_label(GTK_BUTTON(priv->buttons[BUTTON_FENGGE]), _("风格电台"));
	gtk_button_set_label(GTK_BUTTON(priv->buttons[BUTTON_XINGZUO]), _("星座电台"));
	gtk_button_set_label(GTK_BUTTON(priv->buttons[BUTTON_NIANDAI]), _("年代电台"));
}

static void
set_skin(XmrWindow *window, const gchar *skin)
{
	XmrWindowPrivate *priv = window->priv;
	gint x, y;
	GdkPixbuf *pixbuf;
	gint i;
	XmrSkin *xmr_skin;
	static const gchar *ui_main_buttons[] =
	{
		"close", "minimize", "play", "pause",
		"next", "like", "dislike", "volume",
		"lyric", "download", "share", "siren",
		"fengge", "xingzuo", "niandai"
	};

	static const gchar *ui_main_labels[] =
	{
		"radio_name", "song_name",
		"artist", "progress"
	};

	xmr_skin = xmr_skin_new();

	do
	{
		if (!xmr_skin_load(xmr_skin, skin))
		{
			xmr_debug("failed to load skin: %s", skin);
			break;
		}

		pixbuf = xmr_skin_get_image(xmr_skin, UI_MAIN, NULL);
		if (pixbuf == NULL)
		{
			xmr_debug("Missing background image ?");
			break;
		}

		x = gdk_pixbuf_get_width(pixbuf);
		y = gdk_pixbuf_get_height(pixbuf);

		priv->gtk_theme = FALSE;

		gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
		gtk_widget_set_size_request(GTK_WIDGET(window), x, y);
		set_window_image(window, pixbuf);
		g_object_unref(pixbuf);

		hide_children(window);

		for(i=0; i<LAST_BUTTON; ++i)
		{
			pixbuf = xmr_skin_get_image(xmr_skin, UI_MAIN, ui_main_buttons[i]);
			if (pixbuf == NULL)
				continue ;

			xmr_button_set_image_from_pixbuf(XMR_BUTTON(priv->buttons[i]), pixbuf);
			g_object_unref(pixbuf);
			xmr_button_set_type(XMR_BUTTON(priv->buttons[i]), XMR_BUTTON_SKIN);

			if (xmr_skin_get_position(xmr_skin, UI_MAIN, ui_main_buttons[i], &x, &y))
			{
				gtk_fixed_move(GTK_FIXED(priv->fixed), priv->buttons[i], x, y);
				gtk_widget_show(priv->buttons[i]);
			}
		}

		for(i=0; i<LAST_LABEL; ++i)
		{
			if (xmr_skin_get_position(xmr_skin, UI_MAIN, ui_main_labels[i], &x, &y))
			{
				gtk_fixed_move(GTK_FIXED(priv->fixed), priv->labels[i], x, y);
				gtk_widget_show(priv->labels[i]);
			}
		}

		if (xmr_skin_get_position(xmr_skin, UI_MAIN, "cover_image", &x, &y))
		{
			gtk_fixed_move(GTK_FIXED(priv->fixed), priv->image, x, y);
			gtk_widget_show(priv->image);
		}
	}
	while(0);

	g_free(priv->skin);
	g_object_unref(xmr_skin);
}

static void
hide_children(XmrWindow *window)
{
	XmrWindowPrivate *priv = window->priv;
	gint i;

	for(i=0; i<LAST_BUTTON; ++i)
	  gtk_widget_hide(priv->buttons[i]);

	for(i=0; i<LAST_LABEL; ++i)
	  gtk_widget_hide(priv->labels[i]);

	gtk_widget_hide(priv->image);
}

static void
player_eos(XmrPlayer *player,
			gboolean early,
			XmrWindow *window)
{
	xmr_window_play_next(window);
}

static void
player_error(XmrPlayer *player,
			GError *error,
			XmrWindow *window)
{
	g_warning("Player error: %s\n", error->message);
}

static void
player_tick(XmrPlayer *player,
			gint64 elapsed,
			gint64 duration,
			XmrWindow *window)
{
	glong mins_elapsed;
	glong secs_elapsed;

	glong mins_duration;
	glong secs_duration;

	gint64 secs;
	gchar *time;

	secs = elapsed / G_USEC_PER_SEC / 1000;
	mins_elapsed = secs / 60;
	secs_elapsed = secs % 60;

	secs = duration / G_USEC_PER_SEC / 1000;
	mins_duration = secs / 60;
	secs_duration = secs % 60;

	time = g_strdup_printf("%02ld:%02ld/%02ld:%02ld",
				mins_elapsed, secs_elapsed,
				mins_duration, secs_duration
				);

	if (time == NULL){
		g_error("Failed to alloc memory\n");
	}

	gtk_label_set_text(GTK_LABEL(window->priv->labels[LABEL_TIME]),
				time);

	g_free(time);
}

static void
player_buffering(XmrPlayer *player,
			guint progress,
			XmrWindow *window)
{
	xmr_debug("Buffering: %d\n", progress);
}

static void
player_state_changed(XmrPlayer *player,
			gint old_state,
			gint new_state,
			XmrWindow *window)
{
	if (new_state == GST_STATE_PLAYING)
	{
		gtk_widget_hide(window->priv->buttons[BUTTON_PLAY]);
		gtk_widget_show(window->priv->buttons[BUTTON_PAUSE]);
	}
	else if(new_state == GST_STATE_PAUSED)
	{
		gtk_widget_hide(window->priv->buttons[BUTTON_PAUSE]);
		gtk_widget_show(window->priv->buttons[BUTTON_PLAY]);
	}
}

static gboolean
thread_finish(GThread *thread)
{
#if GLIB_CHECK_VERSION(2, 32, 0)
	g_thread_unref(thread);
#endif

	return FALSE;
}

static gpointer
thread_get_playlist(XmrWindow *window)
{
	XmrWindowPrivate *priv = window->priv;
	gint result = 1;
	gboolean auto_play = g_list_length(priv->playlist) == 0;

	if (priv->playlist_url == NULL){
		result = xmr_service_get_track_list(priv->service, &priv->playlist);
	}else{
		result = xmr_service_get_track_list_by_style(priv->service, &priv->playlist, priv->playlist_url);
	}

	if (result != 0)
	{
		xmr_debug("failed to get track list: %d", result);
	}
	else if (auto_play)
	{
		if (g_list_length(priv->playlist) > 0)
		{
			SongInfo *song = (SongInfo *)priv->playlist->data;
			xmr_player_open(priv->player, song->location, NULL);
			xmr_player_play(priv->player);

			gdk_threads_enter();
			xmr_window_set_track_info(window);
			gdk_threads_leave();

			g_signal_emit(window, signals[TRACK_CHANGED], 0, song);
		}
	}

	g_idle_add((GSourceFunc)thread_finish, g_thread_self());

	return NULL;
}

static gpointer
thread_get_cover_image(XmrWindow *window)
{
	XmrWindowPrivate *priv = window->priv;
	GString *data = NULL;
	GdkPixbuf *pixbuf = NULL;

	do
	{
		SongInfo *track;
		gint result;

		if (priv->playlist == NULL)
			break;

		data = g_string_new("");
		if (data == NULL)
			break;

		// always get first song info
		track = (SongInfo *)priv->playlist->data;
		result = xmr_service_get_url_data(priv->service, track->album_cover, data);
		if (result != 0)
		{
			xmr_debug("xmr_service_get_url_data failed: %d", result);
			break;
		}

		pixbuf = gdk_pixbuf_from_memory(data->str, data->len);
		if (pixbuf == NULL)
		{
			xmr_debug("gdk_pixbuf_from_memory failed");
			break;
		}

		gdk_threads_enter();
		set_cover_image(window, pixbuf);
		gdk_threads_leave();
	}
	while(0);

	if (data) {
		g_string_free(data, TRUE);
	}
	if (pixbuf) {
		g_object_unref(pixbuf);
	}
	g_idle_add((GSourceFunc)thread_finish, g_thread_self());

	return NULL;
}

static void
xmr_window_get_playlist(XmrWindow *window)
{
#if GLIB_CHECK_VERSION(2, 32, 0)
	g_thread_new("playlist", (GThreadFunc)thread_get_playlist, window);
#else
	g_thread_create((GThreadFunc)thread_get_playlist, window, FALSE, NULL);
#endif
}

static void
xmr_window_get_cover_image(XmrWindow *window)
{
	#if GLIB_CHECK_VERSION(2, 32, 0)
	g_thread_new("playlist", (GThreadFunc)thread_get_cover_image, window);
#else
	g_thread_create((GThreadFunc)thread_get_cover_image, window, FALSE, NULL);
#endif
}

static void
xmr_window_set_track_info(XmrWindow *window)
{
	XmrWindowPrivate *priv = window->priv;
	SongInfo *song = (SongInfo *)priv->playlist->data;

	xmr_window_get_cover_image(window);

	gtk_label_set_text(GTK_LABEL(priv->labels[LABEL_SONG_NAME]), song->song_name);
	gtk_label_set_text(GTK_LABEL(priv->labels[LABEL_ARTIST]), song->artist_name);

	gtk_widget_set_tooltip_text(priv->image, song->album_name);
}

void
xmr_window_play_next(XmrWindow *window)
{
	XmrWindowPrivate *priv;
	gpointer data;
	SongInfo *song;

	g_return_if_fail( window != NULL );
	priv = window->priv;

	if (g_list_length(priv->playlist) == 0){
		goto no_more_track;
	}

	data = priv->playlist->data;

	// remove current song
	priv->playlist = g_list_remove(priv->playlist, data);
	song_info_free(data);

	if (g_list_length(priv->playlist) == 0){
		goto no_more_track;
	}

	song = (SongInfo *)priv->playlist->data;
	xmr_player_open(priv->player, song->location, NULL);
	xmr_player_play(priv->player);

	xmr_window_set_track_info(window);

	g_signal_emit(window, signals[TRACK_CHANGED], 0, song);

	// get new playlist if only one song in playlist
	if (g_list_length(priv->playlist) > 1)
	  return ;

no_more_track:
	xmr_window_get_playlist(window);
}

void
xmr_window_pause(XmrWindow *window)
{
	XmrWindowPrivate *priv;

	g_return_if_fail( window != NULL );
	priv = window->priv;

	xmr_player_pause(priv->player);
}

SongInfo *
xmr_window_get_current_song(XmrWindow *window)
{
	g_return_val_if_fail(window != NULL, NULL);

	if (g_list_length(window->priv->playlist) == 0)
		return NULL;

	return (SongInfo *)window->priv->playlist->data;
}

static void
create_popup_menu(XmrWindow *window)
{
	XmrWindowPrivate *priv = window->priv;
	GtkWidget *item = NULL;
	GtkAccelGroup* accel_group;

	accel_group = gtk_accel_group_new();
	gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);

	priv->popup_menu = gtk_menu_new();
	priv->skin_menu = gtk_menu_new();

	item = gtk_image_menu_item_new_with_mnemonic(_("_Gtk Theme"));
	gtk_image_menu_item_set_accel_group(GTK_IMAGE_MENU_ITEM(item), accel_group);
	gtk_menu_shell_append(GTK_MENU_SHELL(priv->skin_menu), item);

	g_signal_connect(item, "activate", G_CALLBACK(on_menu_item_activate), window);

	item = gtk_image_menu_item_new_with_mnemonic(_("_Skin"));
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), priv->skin_menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(priv->popup_menu), item);

	item = gtk_image_menu_item_new_with_mnemonic(_("_Preferences"));
	gtk_image_menu_item_set_accel_group(GTK_IMAGE_MENU_ITEM(item), accel_group);
	gtk_menu_shell_append(GTK_MENU_SHELL(priv->popup_menu), item);

	g_signal_connect(item, "activate", G_CALLBACK(on_menu_item_activate), window);

	item = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(priv->popup_menu), item);

	item = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, accel_group);
	gtk_menu_shell_append(GTK_MENU_SHELL(priv->popup_menu), item);

	g_signal_connect(item, "activate", G_CALLBACK(on_menu_item_activate), window);

	gtk_widget_show_all(priv->popup_menu);
}

static void
on_menu_item_activate(GtkMenuItem *item, XmrWindow *window)
{
	const gchar *menu = gtk_menu_item_get_label(item);

	g_print("menu item: %s\n", menu);

	if (g_strcmp0(menu, _("_Gtk Theme")) == 0)
	{
	}
	else if(g_strcmp0(menu, _("_Preferences")) == 0)
	{
	}
	else if(g_strcmp0(menu, GTK_STOCK_QUIT) == 0)
	{
		gtk_widget_destroy(GTK_WIDGET(window));
	}
}
