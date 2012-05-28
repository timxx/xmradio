#ifndef __XMR_APP_H__
#define __XMR_APP_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define XMR_TYPE_APP			(xmr_app_get_type())
#define XMR_APP(o)				(G_TYPE_CHECK_INSTANCE_CAST((o), XMR_TYPE_APP, XmrApp))
#define XMR_APP_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), XMR_TYPE_APP, XmrAppClass))
#define XMR_IS_APP(o)			(G_TYPE_CHECK_INSTANCE_TYPE((o), XMR_TYPE_APP))
#define XMR_IS_APP_CLASS(k)		(G_TYPE_CHECK_CLASS_TYPE((k), XMR_TYPE_APP))
#define XMR_APP_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS((o), XMR_TYPE_APP, XmrAppClass))

typedef struct _XmrApp			XmrApp;
typedef struct _XmrAppClass		XmrAppClass;
typedef struct _XmrAppPrivate	XmrAppPrivate;

struct _XmrApp
{
	GtkApplication parent;

	XmrAppPrivate *priv;
};

struct _XmrAppClass
{
	GtkApplicationClass parent_class;
};

GType	xmr_app_get_type();
XmrApp*	xmr_app_new(gint argc, gchar **argv);


G_END_DECLS

#endif /* __XMR_APP_H__ */
