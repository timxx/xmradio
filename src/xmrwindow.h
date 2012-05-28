#ifndef __XMR_WINDOW_H__
#define __XMR_WINDOW_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define XMR_TYPE_WINDOW			(xmr_window_get_type())
#define XMR_WINDOW(o)			(G_TYPE_CHECK_INSTANCE_CAST((o), XMR_TYPE_WINDOW, XmrWindow))
#define XMR_WINDOW_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), XMR_TYPE_WINDOW, XmrWindowClass))
#define XMR_IS_WINDOW(o)		(G_TYPE_CHECK_INSTANCE_TYPE((o), XMR_TYPE_WINDOW))
#define XMR_IS_WINDOW_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE((k), XMR_TYPE_WINDOW))
#define XMR_WINDOW_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS((o), XMR_TYPE_WINDOW, XmrWindowClass))

typedef struct _XmrWindow			XmrWindow;
typedef struct _XmrWindowClass		XmrWindowClass;
typedef struct _XmrWindowPrivate	XmrWindowPrivate;

struct _XmrWindow
{
	GtkWindow parent;
	XmrWindowPrivate *priv;
};

struct _XmrWindowClass
{
	GtkWindowClass parent_class;
};

GType		xmr_window_get_type();
GtkWidget*	xmr_window_new();

G_END_DECLS

#endif /* __XMR_WINDOW_H__ */
