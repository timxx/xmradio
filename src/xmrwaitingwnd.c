/**
 * xmrwaitingwnd.c
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

#include "xmrwaitingwnd.h"
#include "xmrwindow.h"

G_DEFINE_TYPE(XmrWaitingWnd, xmr_waiting_wnd, GTK_TYPE_WINDOW)

#define WAITING_WND_W	350
#define WAITING_WND_H	80

enum
{
	PROP_0,
	PROP_PARENT
};

typedef struct
{
	InfoType type;
	gchar *info;
}Task;

struct _XmrWaitingWndPrivate
{
	GSList *tasks;

	GtkWindow *parent;
};

static gboolean
on_key_release(XmrWaitingWnd *wnd,
			   GdkEventKey	 *event,
			   gpointer		 data);

static void
free_task(Task *task)
{
	g_free(task->info);
	g_free(task);
}

static void
new_task(XmrWaitingWndPrivate *priv, InfoType type, const gchar *info)
{
	GSList *p = priv->tasks;
	while (p)
	{
		Task *t = (Task *)p->data;
		if (t->type == type) // not allow the same 'type' exists
		{
			priv->tasks = g_slist_remove(priv->tasks, t);
			free_task(t);
			break;
		}
		
		p = p->next;
	}
	// prepend to the list
	{
		Task *task = g_new0(Task, 1);
		task->type = type;
		task->info = g_strdup(info);
		priv->tasks = g_slist_prepend(priv->tasks, task);
	}
}

static void
on_parent_hide(XmrWindow *parent, XmrWaitingWnd *wnd)
{
	xmr_waiting_wnd_hide(wnd);
}

static void
on_parent_show(XmrWindow *parent, XmrWaitingWnd *wnd)
{
	if (g_slist_length(wnd->priv->tasks) > 0)
		xmr_waiting_wnd_show(wnd);
}

static void
xmr_waiting_wnd_finalize(GObject *obj)
{
	XmrWaitingWnd *wnd = XMR_WAITING_WND(obj);
	XmrWaitingWndPrivate *priv = wnd->priv;
	
	if (priv->tasks)
	{
		g_slist_free_full(priv->tasks, (GDestroyNotify)free_task);
		priv->tasks = NULL;
	}
}

static void
xmr_waiting_wnd_get_property(GObject *object,
							 guint prop_id,
							 GValue *value,
							 GParamSpec *pspec)
{
	XmrWaitingWnd *window = XMR_WAITING_WND(object);
	XmrWaitingWndPrivate *priv = window->priv;

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
xmr_waiting_wnd_set_property(GObject *object,
							 guint prop_id,
							 const GValue *value,
							 GParamSpec *pspec)
{
	XmrWaitingWnd *window = XMR_WAITING_WND(object);
	XmrWaitingWndPrivate *priv = window->priv;

	switch(prop_id)
	{
	case PROP_PARENT:
		priv->parent = g_value_get_object(value);
		gtk_window_set_transient_for(GTK_WINDOW(object), priv->parent);
		g_signal_connect(priv->parent, "hide", G_CALLBACK(on_parent_hide), window);
		g_signal_connect(priv->parent, "show", G_CALLBACK(on_parent_show), window);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static gboolean
on_draw(XmrWaitingWnd *wnd, cairo_t *cr, gpointer data)
{
	PangoLayout *layout;
	gint w, h;
	XmrWaitingWndPrivate *priv = wnd->priv;
	
	if (priv->tasks == NULL || priv->tasks->data == NULL)
		return FALSE;
	
	cairo_set_source_rgba(cr, 0, 0, 0, 0.7);
	cairo_paint(cr);
	
	layout = gtk_widget_create_pango_layout(GTK_WIDGET(wnd), ((Task *)priv->tasks->data)->info);
	
	pango_layout_get_pixel_size(layout, &w, &h);
	cairo_move_to(cr, (WAITING_WND_W - w) / 2, (WAITING_WND_H - h) / 2);
	cairo_set_source_rgb(cr, 1, 1, 1);
	pango_cairo_show_layout(cr, layout);
	
	g_object_unref(layout);
	
	return GTK_WIDGET_CLASS(xmr_waiting_wnd_parent_class)->draw(GTK_WIDGET(wnd), cr);
}

static void
xmr_waiting_wnd_class_init(XmrWaitingWndClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->finalize = xmr_waiting_wnd_finalize;
	
	object_class->set_property = xmr_waiting_wnd_set_property;
	object_class->get_property = xmr_waiting_wnd_get_property;
	
	g_object_class_install_property(object_class,
				PROP_PARENT,
				g_param_spec_object("parent",
					"Parent",
					"Parent window",
					GTK_TYPE_WINDOW,
					G_PARAM_READWRITE));

	g_type_class_add_private(object_class, sizeof(XmrWaitingWndPrivate));
}

static void
xmr_waiting_wnd_init(XmrWaitingWnd *wnd)
{
	XmrWaitingWndPrivate *priv;
	
	wnd->priv = G_TYPE_INSTANCE_GET_PRIVATE(wnd, XMR_TYPE_WAITING_WND, XmrWaitingWndPrivate);
	priv = wnd->priv;
	
	priv->tasks = NULL;
	priv->parent = NULL;
	
	gtk_widget_set_app_paintable(GTK_WIDGET(wnd), TRUE);
	gtk_widget_set_size_request(GTK_WIDGET(wnd), WAITING_WND_W, WAITING_WND_H);
	gtk_widget_set_opacity(GTK_WIDGET(wnd), 0.7);
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(wnd), TRUE);
	gtk_window_set_decorated(GTK_WINDOW(wnd), FALSE);
	
	gtk_widget_set_events(GTK_WIDGET(wnd), GDK_KEY_RELEASE_MASK);
	g_signal_connect(wnd, "draw", G_CALLBACK(on_draw), NULL);
	g_signal_connect(wnd, "key-release-event", G_CALLBACK(on_key_release), NULL);
}

GtkWidget*
xmr_waiting_wnd_new(GtkWindow *parent)
{
	return g_object_new(XMR_TYPE_WAITING_WND,
						"resizable", FALSE,
						"type", GTK_WINDOW_TOPLEVEL,
						"parent", parent,
						NULL);
}

void
xmr_waiting_wnd_add_task(XmrWaitingWnd *wnd,
						 InfoType type,
						 const gchar *text)
{
	g_return_if_fail(wnd != NULL);
	if (text != NULL)
	{	
		new_task(wnd->priv, type, text);
		
		xmr_waiting_wnd_show(wnd);
		gtk_widget_queue_draw(GTK_WIDGET(wnd));
	}
}

void
xmr_waiting_wnd_show(XmrWaitingWnd *wnd)
{
	GdkWindow *gdk_window;
	gboolean is_iconify = TRUE;

	g_return_if_fail(wnd != NULL);
	
	if (g_slist_length(wnd->priv->tasks) == 0)
	{
		gtk_widget_hide(GTK_WIDGET(wnd));
		return ;
	}
	
	if (wnd->priv->parent == NULL)
	{
		gtk_window_set_position(GTK_WINDOW(wnd), GTK_WIN_POS_CENTER);
	}
	else
	{
		gint px, py;
		gint pw, ph;
		gint x, y;
		gint w, h;
		
		gtk_window_get_position(wnd->priv->parent, &px, &py);
		gtk_window_get_size(wnd->priv->parent, &pw, &ph);
		
		gtk_window_get_size(GTK_WINDOW(wnd), &w, &h);
		
		x = px + (pw - w) / 2;
		y = py + (ph - h) / 2;
		
		gtk_window_move(GTK_WINDOW(wnd), x, y);
	}
	
	// only do this when parent visible
	gdk_window = gtk_widget_get_window(GTK_WIDGET(wnd->priv->parent));

	if (gdk_window)
		is_iconify = gdk_window_get_state(gdk_window) & GDK_WINDOW_STATE_ICONIFIED;

	if (!is_iconify && gtk_widget_get_visible(GTK_WIDGET(wnd->priv->parent)))
	{
		gtk_widget_show(GTK_WIDGET(wnd));
		gtk_widget_queue_draw(GTK_WIDGET(wnd));
	}
}

void
xmr_waiting_wnd_hide(XmrWaitingWnd *wnd)
{
	g_return_if_fail(wnd != NULL);
	
	gtk_widget_hide(GTK_WIDGET(wnd));
}

void
xmr_waiting_wnd_next_task(XmrWaitingWnd *wnd, InfoType type)
{
	g_return_if_fail(wnd != NULL);
	
	// remove the first one
	if (wnd->priv->tasks && wnd->priv->tasks->data)
	{
		Task *t = (Task *)wnd->priv->tasks->data;
		// not the current task
		// just remove from the list
		if (t->type != type)
		{
			GSList *p = wnd->priv->tasks;
			while (p)
			{
				t = (Task *)p->data;
				if (t->type == type)
				{
					wnd->priv->tasks = g_slist_remove(wnd->priv->tasks, t);
					free_task(t);
					break;
				}
				p = p->next;
			}
		}
		else
		{
			wnd->priv->tasks = g_slist_remove(wnd->priv->tasks, t);
			free_task(t);
		}
	}
	
	if (g_slist_length(wnd->priv->tasks) == 0)
		xmr_waiting_wnd_hide(wnd);
	else
		gtk_widget_queue_draw(GTK_WIDGET(wnd));
}

static gboolean
on_key_release(XmrWaitingWnd *wnd,
			   GdkEventKey	 *event,
			   gpointer		 data)
{
	if (event->keyval == GDK_KEY_Escape)
	{
		gtk_widget_hide(GTK_WIDGET(wnd));
	}

	return FALSE;
}

