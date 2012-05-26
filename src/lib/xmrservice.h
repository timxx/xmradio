#ifndef __XMR_SERVICE_H__
#define __XMR_SERVICE_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define XMR_SERVICE_TYPE            (xmr_service_get_type())
#define XMR_SERVICE(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), XMR_SERVICE_TYPE, XmrService))
#define XMR_SERVICE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  XMR_SERVICE_TYPE, XmrServiceClass))
#define IS_XMR_SERVICE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), XMR_SERVICE_TYPE))
#define IS_XMR_SERVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  XMR_SERVICE_TYPE))
#define XMR_SERVICE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  XMR_SERVICE_TYPE, XmrServiceClass))

typedef struct _XmrService			XmrService;
typedef struct _XmrServiceClass		XmrServiceClass;
typedef struct _XmrServicePrivate	XmrServicePrivate;

struct _XmrService
{
	GObject parent;

	XmrServicePrivate *priv;
};

struct _XmrServiceClass
{
	GObjectClass parent;
};

GType		xmr_service_get_type();
GObject*	xmr_service_new();

gint
xmr_service_login(XmrService *xs,
			const gchar *usr,
			const gchar *pwd
			);

gboolean
xmr_service_is_logged_in(XmrService *xs);

void
xmr_service_get_track_list(XmrService *xs, GList **list);

G_END_DECLS

#endif /* __XMR_SERVICE_H__ */
