/** 
 * xmrartist.c
 * This file is part of xmartist
 *
 * Copyright (C) 2013 - 2015 Weitian Leung (weitianleung@gmail.com)

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

#include "xmrartist.h"

G_DEFINE_TYPE(XmrArtist, xmr_artist, GTK_TYPE_BOX)

#define SET_ARTIST(member, val)	\
	g_free(artist->priv->info->member);	\
	artist->priv->info->member = g_strdup(val);

struct _XmrArtistPrivate
{
	GtkWidget *button;
	GtkWidget *image;
	GdkCursor *cursor;
	
	GtkWidget *label_name;
	GtkWidget *label_region;

	Artist *info;
};

enum
{
	PROP_0,
	PROP_ID,
	PROP_NAME,
	PROP_REGION,
	PROP_IMG_SRC
};

enum
{
	CLICKED,
	LAST_SIGNAL
};

guint signals[LAST_SIGNAL] = { 0 };

static gboolean
on_mouse_event(GtkButton *button, GdkEvent *event, XmrArtist *artist);

static void
on_button_clicked(GtkButton *button, XmrArtist *artist);

void
artist_free(Artist *artist)
{
	if (artist)
	{
		g_free(artist->id);
		g_free(artist->name);
		g_free(artist->region);
		g_free(artist->img_src);
		
		g_free(artist);
	}
}

static void
xmr_artist_get_property(GObject *object,
			   guint prop_id,
			   GValue *value,
			   GParamSpec *pspec)
{
	XmrArtist *artist = XMR_ARTIST(object);
	XmrArtistPrivate *priv = artist->priv;

	switch(prop_id)
	{
	case PROP_ID:
		g_value_set_string(value, priv->info->id);
		break;

	case PROP_NAME:
		g_value_set_string(value, priv->info->name);
		break;

	case PROP_REGION:
		g_value_set_string(value, priv->info->region);
		break;
		
	case PROP_IMG_SRC:
		g_value_set_string(value, priv->info->img_src);

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
xmr_artist_set_property(GObject *object,
			   guint prop_id,
			   const GValue *value,
			   GParamSpec *pspec)
{
	XmrArtist *artist = XMR_ARTIST(object);

	gchar *str = NULL;

	switch(prop_id)
	{
	case PROP_ID:
		str = g_value_dup_string(value);
		xmr_artist_set_id(artist, str);
		g_free(str);
		break;

	case PROP_NAME:
		str = g_value_dup_string(value);
		xmr_artist_set_name(artist, str);
		g_free(str);
		break;

	case PROP_REGION:
		str = g_value_dup_string(value);
		xmr_artist_set_region(artist, str);
		g_free(str);
		break;
		
	case PROP_IMG_SRC:
		str = g_value_dup_string(value);
		xmr_artist_set_img_src(artist, str);
		g_free(str);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
xmr_artist_dispose(GObject *obj)
{
	XmrArtist *artist = XMR_ARTIST(obj);
	XmrArtistPrivate *priv = artist->priv;
	
	if (priv->info)
	{
		artist_free(priv->info);
		priv->info = NULL;
	}

	if (priv->cursor)
	{
		g_object_unref(priv->cursor);
		priv->cursor = NULL;
	}

	G_OBJECT_CLASS(xmr_artist_parent_class)->dispose(obj);
}

static void xmr_artist_init(XmrArtist *artist)
{
	XmrArtistPrivate *priv;
	GtkWidget *vbox;
	
	priv = G_TYPE_INSTANCE_GET_PRIVATE(artist, XMR_TYPE_ARTIST, XmrArtistPrivate);
	artist->priv = priv;

	priv->button = gtk_button_new();
	priv->image = gtk_image_new();
	priv->cursor = gdk_cursor_new_for_display(gtk_widget_get_display(GTK_WIDGET(artist)), GDK_HAND1);
	priv->info = g_new0(Artist, 1);
	priv->label_name = gtk_label_new("");
	priv->label_region = gtk_label_new("");

	gtk_button_set_image_position(GTK_BUTTON(priv->button), GTK_POS_TOP);
	gtk_button_set_relief(GTK_BUTTON(priv->button), GTK_RELIEF_NONE);
	gtk_button_set_image(GTK_BUTTON(priv->button), priv->image);

	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
	gtk_box_pack_start(GTK_BOX(vbox), priv->label_name, TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), priv->label_region, TRUE, FALSE, 0);
	
	gtk_box_pack_start(GTK_BOX(artist), priv->button, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(artist), vbox, FALSE, FALSE, 0);
	
	g_signal_connect(priv->button, "enter-notify-event", G_CALLBACK(on_mouse_event), artist);
	g_signal_connect(priv->button, "leave-notify-event", G_CALLBACK(on_mouse_event), artist);
	g_signal_connect(priv->button, "clicked", G_CALLBACK(on_button_clicked), artist);

	gtk_widget_show_all(GTK_WIDGET(artist));
}

static void xmr_artist_class_init(XmrArtistClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->set_property = xmr_artist_set_property;
	object_class->get_property = xmr_artist_get_property;
	object_class->dispose = xmr_artist_dispose;
	
	signals[CLICKED] =
		g_signal_new("clicked",
					G_OBJECT_CLASS_TYPE(object_class),
					G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
					G_STRUCT_OFFSET(XmrArtistClass, clicked),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID,
					G_TYPE_NONE,
					0);

	g_object_class_install_property(object_class,
				PROP_ID,
				g_param_spec_string("id",
					"Artist id",
					"Artist id",
					NULL,
					G_PARAM_READWRITE)
					 );

	g_object_class_install_property(object_class,
				PROP_NAME,
				g_param_spec_string("name",
					"Name",
					"Name of the artist",
					NULL,
					G_PARAM_READWRITE)
					 );

	g_object_class_install_property(object_class,
				PROP_REGION,
				g_param_spec_string("region",
					"Artist region",
					"Region of the artist",
					NULL,
					G_PARAM_READWRITE)
					 );

	g_object_class_install_property(object_class,
				PROP_IMG_SRC,
				g_param_spec_string("img-src",
					"Image source for artist",
					"Image source for artist",
					NULL,
					G_PARAM_READWRITE)
					 );
	
	 g_type_class_add_private(G_OBJECT_CLASS(klass), sizeof(XmrArtistPrivate));
}


static gboolean
on_mouse_event(GtkButton *button, GdkEvent *event, XmrArtist *artist)
{
	switch(event->type)
	{
	case GDK_ENTER_NOTIFY:
		gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(button)), artist->priv->cursor);
		break;

	case GDK_LEAVE_NOTIFY:
		gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(button)), NULL);
		break;

	default:
		break;
	}

	return FALSE;
}

static void
on_button_clicked(GtkButton *button, XmrArtist *artist)
{
	g_signal_emit(artist, signals[CLICKED], 0);
}

GtkWidget *
xmr_artist_new()
{
	return g_object_new(XMR_TYPE_ARTIST,
				NULL);
}

GtkWidget *
xmr_artist_new_with_info(Artist *data)
{
	return g_object_new(XMR_TYPE_ARTIST,
				"id", data->id,
				"name", data->name,
				"region", data->region,
				"img-src", data->img_src,
				NULL);
}

void
xmr_artist_set_id(XmrArtist *artist,
			const gchar *id)
{
	g_return_if_fail(artist != NULL);
	
	SET_ARTIST(id, id);
}

const gchar *
xmr_artist_get_id(XmrArtist *artist)
{
	g_return_val_if_fail(artist != NULL, NULL);
	return artist->priv->info->id;
}

void
xmr_artist_set_name(XmrArtist *artist,
			const gchar *name)
{
	g_return_if_fail(artist != NULL);

	SET_ARTIST(name, name);
	gtk_label_set_text(GTK_LABEL(artist->priv->label_name), name);
}

const gchar *
xmr_artist_get_name(XmrArtist *artist)
{
	g_return_val_if_fail(artist != NULL, NULL);

	return artist->priv->info->name;
}

void
xmr_artist_set_region(XmrArtist *artist,
			const gchar *region)
{
	g_return_if_fail(artist != NULL);
	
	SET_ARTIST(region, region);
	gtk_label_set_text(GTK_LABEL(artist->priv->label_region), region);
}

const gchar *
xmr_artist_get_region(XmrArtist *artist)
{
	g_return_val_if_fail(artist != NULL, NULL);

	return artist->priv->info->region;
}

void
xmr_artist_set_img_src(XmrArtist *artist,
					   const gchar *img_src)
{
	XmrArtistPrivate *priv;
	g_return_if_fail(artist != NULL && img_src != NULL);
	priv = artist->priv;
	
	gtk_image_set_from_file(GTK_IMAGE(priv->image), img_src);
	
}

const gchar *
xmr_artist_get_img_src(XmrArtist *artist)
{
	g_return_val_if_fail(artist != NULL, NULL);
	
	return artist->priv->info->img_src;
}
