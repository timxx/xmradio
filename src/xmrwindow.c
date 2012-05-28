
#include <glib/gi18n.h>

#include "xmrwindow.h"
#include "xmrplayer.h"
#include "xmrbutton.h"

G_DEFINE_TYPE(XmrWindow, xmr_window, GTK_TYPE_WINDOW);

enum
{
	BUTTON_CLOSE = 0,	// X button
	BUTTON_MINIMIZE,	// - button
	BUTTON_PLAY,		// play
	BUTTON_NEXT,
	BUTTON_LIKE,
	BUTTON_UNLIKE,
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
	XmrPlayer	*player;
	gchar		*skin;
	gboolean	gtk_theme;	/* TRUE to use gtk theme rather than skin */

	GtkWidget	*buttons[LAST_BUTTON];	/* ui buttons */
	GtkWidget	*labels[LAST_LABEL];
	GtkWidget	*image;	/* album cover image */

	cairo_surface_t	*cs_bkgnd;	/* backgroud image surface */

	GtkWidget	*fixed;	/* #GtkFixed */
};

enum
{
	PROP_0,
	PROP_PLAYER,
	PROP_SKIN,
	PROP_GTK_THEME
};

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
hide_children(XmrWindow *window);

static void 
xmr_window_class_init(XmrWindowClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->set_property = xmr_window_set_property;
	object_class->get_property = xmr_window_get_property;

	object_class->dispose = xmr_window_dispose;
	object_class->finalize = xmr_window_finalize;

	g_object_class_install_property(object_class,
				PROP_PLAYER,
				g_param_spec_object("player",
					"Player",
					"Player object",
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

	g_type_class_add_private(object_class, sizeof(XmrWindowPrivate));
}

static void 
xmr_window_init(XmrWindow *window)
{
	XmrWindowPrivate *priv;
	gint i;

	window->priv = G_TYPE_INSTANCE_GET_PRIVATE(window, XMR_TYPE_WINDOW, XmrWindowPrivate);
	priv = window->priv;

	priv->player = NULL;
	priv->skin = NULL;
	priv->gtk_theme = FALSE;
	priv->cs_bkgnd = NULL;

	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_widget_set_app_paintable(GTK_WIDGET(window), TRUE);
	gtk_widget_add_events(GTK_WIDGET(window), GDK_BUTTON_PRESS_MASK);

	gtk_window_set_default_icon_name("xmradio");

	priv->fixed = gtk_fixed_new();

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

	set_gtk_theme(window);
}

GtkWidget* xmr_window_new()
{
	 return g_object_new(XMR_TYPE_WINDOW,
				"type", GTK_WINDOW_TOPLEVEL,
				"title", _("XMRadio"),
				"height-request", 250,
				"width-request", 540,
				"resizable", FALSE,
				NULL);
}

static void
xmr_window_dispose(GObject *obj)
{
//	XmrWindow *window = XMR_WINDOW(obj);
//	XmrWindowPrivate *priv = window->priv;

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

	case PROP_SKIN:
		g_value_set_string(value, priv->skin);
		break;

	case PROP_GTK_THEME:
		g_value_set_boolean(value, priv->gtk_theme);
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
	case PROP_PLAYER:
		priv->player = g_value_get_object(value);
		break;

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

	return FALSE;
}

static void
on_xmr_button_clicked(GtkWidget *widget, gpointer data)
{
	XmrButton *button = XMR_BUTTON(widget);
	glong id = (glong)data;

	g_print("button %ld clicked\n", id);
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

	gint width, height;
	gint i_w, i_h;

	gtk_widget_get_size_request(priv->image, &width, &height);
	i_w = gdk_pixbuf_get_width(pixbuf);
	i_h = gdk_pixbuf_get_height(pixbuf);

	if (i_w != width || i_h != height) //scale
	{
		GdkPixbuf *pb = gdk_pixbuf_scale_simple(pixbuf, width, height, GDK_INTERP_BILINEAR);
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
