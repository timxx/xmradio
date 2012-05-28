#include "xmrbutton.h"

G_DEFINE_TYPE(XmrButton, xmr_button, GTK_TYPE_BUTTON);

struct _XmrButtonPrivate
{
	XmrButtonType type;

	GdkPixbuf *images[LAST_STATE];
	XmrButtonState	state;	/* current state */
};

enum
{
	PROP_0,
	PROP_TYPE
};

static gboolean
on_draw(XmrButton *button, cairo_t *cr, gpointer data);

static gboolean
on_button_mouse_event(XmrButton *button, GdkEvent *event, gpointer data);

static void
xmr_button_unref_images(XmrButton *button)
{
	gint i;

	for(i=0; i<LAST_STATE; ++i)
	{
		if (button->priv->images[i])
		{
			g_object_unref(button->priv->images[i]);
			button->priv->images[i] = NULL;
		}
	}
}

static void
xmr_button_dispose(GObject *obj)
{
	XmrButton *button = XMR_BUTTON(obj);

	xmr_button_unref_images(button);

	G_OBJECT_CLASS(xmr_button_parent_class)->dispose(obj);
}

static void
xmr_button_get_property(GObject *object,
		       guint prop_id,
		       GValue *value,
		       GParamSpec *pspec)
{
	XmrButton *button = XMR_BUTTON(object);
	XmrButtonPrivate *priv = button->priv;

	switch(prop_id)
	{
	case PROP_TYPE:
		g_value_set_int(value, priv->type);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
xmr_button_set_property(GObject *object,
		       guint prop_id,
		       const GValue *value,
		       GParamSpec *pspec)
{
	XmrButton *button = XMR_BUTTON(object);
	XmrButtonPrivate *priv = button->priv;

	switch(prop_id)
	{
	case PROP_TYPE:
		priv->type = g_value_get_int(value);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void 
xmr_button_class_init(XmrButtonClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->get_property = xmr_button_get_property;
	object_class->set_property = xmr_button_set_property;
	object_class->dispose = xmr_button_dispose;

	g_object_class_install_property(object_class,
				PROP_TYPE,
				g_param_spec_int("type",
					"Button type",
					"Button type",
					0, 1,
					1,
					G_PARAM_READWRITE));

	g_type_class_add_private(object_class, sizeof(XmrButtonPrivate));
}

static void 
xmr_button_init(XmrButton *button)
{
	XmrButtonPrivate *priv;
	gint i;

	button->priv = G_TYPE_INSTANCE_GET_PRIVATE(button, XMR_TYPE_BUTTON, XmrButtonPrivate);
	priv = button->priv;

	priv->type = XMR_BUTTON_NORMAL;
	priv->state = STATE_NORMAL;

	for(i=0; i<LAST_STATE; ++i) 
		priv->images[i] = NULL;

	gtk_widget_set_app_paintable(GTK_WIDGET(button), TRUE);

	g_signal_connect(button, "button-press-event",
				G_CALLBACK(on_button_mouse_event), NULL);
    g_signal_connect(button, "button-release-event",
                G_CALLBACK(on_button_mouse_event), NULL);
    g_signal_connect(button, "enter-notify-event",
                G_CALLBACK(on_button_mouse_event), NULL);
    g_signal_connect(button, "leave-notify-event",
                G_CALLBACK(on_button_mouse_event), NULL);

	g_signal_connect(button, "draw",
				G_CALLBACK(on_draw), NULL);
}

GtkWidget*	xmr_button_new(XmrButtonType type)
{
	return g_object_new(XMR_TYPE_BUTTON,
				"type", type,
				"relief", GTK_RELIEF_NONE,
				"focus-on-click", FALSE,
				NULL
				);
}

XmrButtonType
xmr_button_set_type(XmrButton *button, XmrButtonType type)
{
	XmrButtonType old_type;

	g_return_val_if_fail( button != NULL &&
				(type >= XMR_BUTTON_NORMAL && type <= XMR_BUTTON_SKIN),
				type);
	old_type = button->priv->type;

	button->priv->type = type;

	return old_type;
}

XmrButtonState
xmr_button_set_state(XmrButton *button, XmrButtonState state)
{
	XmrButtonState old_state;

	g_return_val_if_fail( button != NULL &&
				(state >= STATE_NORMAL && state < LAST_STATE),
				state);

	old_state = button->priv->state;

	button->priv->state = state;

	gtk_widget_queue_draw(GTK_WIDGET(button));

	return old_state;
}

static gboolean
on_draw(XmrButton *button, cairo_t *cr, gpointer data)
{
	XmrButtonPrivate *priv = button->priv;
	cairo_surface_t *image;
	GdkPixbuf *pixbuf = priv->images[priv->state];
	gint image_width, image_height;

	if (pixbuf == NULL)
		return FALSE;

	image_width = gdk_pixbuf_get_width(pixbuf);
	image_height = gdk_pixbuf_get_height(pixbuf);

	image = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, image_width, image_height);
	if (image == NULL)
		return FALSE;

	cairo_set_source_surface(cr, image, 0, 0);
	gdk_cairo_set_source_pixbuf(cr, pixbuf, 0, 0);
	cairo_paint(cr);
	
	cairo_surface_destroy(image);

	gtk_widget_set_size_request(GTK_WIDGET(button), image_width, image_height);

	return TRUE;
}

static gboolean
on_button_mouse_event(XmrButton *button, GdkEvent *event, gpointer data)
{
    static GdkCursor *cursor = NULL;
	XmrButtonPrivate *priv;

	priv = button->priv;
	if (priv->state == STATE_DISABLE || priv->type == XMR_BUTTON_NORMAL)
		return FALSE;

    switch(event->type)
    {
    case GDK_BUTTON_PRESS:
		priv->state = STATE_PUSH;
        break;

    case GDK_BUTTON_RELEASE:
		priv->state = STATE_NORMAL;
        break;

    case GDK_ENTER_NOTIFY:
        if (NULL == cursor)
            cursor = gdk_cursor_new(GDK_HAND1);
        gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(button)), cursor);
		priv->state = STATE_FOCUS;
        break;

    case GDK_LEAVE_NOTIFY:
        gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(button)), NULL);
		priv->state = STATE_NORMAL;
        break;

    default:
		priv->state = STATE_NORMAL;
        break;
    }

	return FALSE;
}

void
xmr_button_set_image_from_uri(XmrButton *button, const gchar *uri)
{	
	GdkPixbuf *pixbuf;

	g_return_if_fail( button != NULL && uri != NULL);

	pixbuf = gdk_pixbuf_new_from_file(uri, NULL);
	if (pixbuf)
	{
		xmr_button_set_image_from_pixbuf(button, pixbuf);
		g_object_unref(pixbuf);
	}
}

void
xmr_button_set_image_from_pixbuf(XmrButton *button, GdkPixbuf *pixbuf)
{
	XmrButtonPrivate *priv;
	gint width, height;
	gint i;

	g_return_if_fail( button != NULL && pixbuf != NULL);

	priv = button->priv;

	width = gdk_pixbuf_get_width(pixbuf)/LAST_STATE;
	height = gdk_pixbuf_get_height(pixbuf);
	 
	for(i=0; i<LAST_STATE; i++)
		priv->images[i] = gdk_pixbuf_new_subpixbuf(pixbuf, width*i, 0, width, height);

	gtk_widget_queue_draw(GTK_WIDGET(button));
}

void
xmr_button_set_image_from_stock(XmrButton *button, const gchar *stock)
{
	GtkWidget *image;

	g_return_if_fail( button != NULL);

	if (stock == NULL)
		return ;

	image = gtk_button_get_image(GTK_BUTTON(button));
	if (image == NULL)
	{
		image = gtk_image_new_from_stock(stock, GTK_ICON_SIZE_BUTTON);

		gtk_button_set_image(GTK_BUTTON(button), image);
	}
	else
	{
		gtk_image_set_from_stock(GTK_IMAGE(image), stock, GTK_ICON_SIZE_BUTTON);
	}
}
