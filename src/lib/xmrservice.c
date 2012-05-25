#include <curl/curl.h>

#include "xmrservice.h"
#include "songinfo.h"

#define XMR_SERVICE_GET_PRIVATE(obj)	\
	(G_TYPE_INSTANCE_GET_PRIVATE((obj), XMR_SERVICE_TYPE, XmrServicePrivate))

#define XMR_LOGIN_URL	"http://www.xiami.com/kuang/login"

G_DEFINE_TYPE(XmrService, xmr_service, G_TYPE_OBJECT);

struct _XmrServicePrivate
{
	gboolean logged;	// user logged in ?

	CURL *curl;
};

static void xmr_service_dispose(GObject *obj)
{
}

static void xmr_service_class_init(XmrServiceClass *klass)
{
	GObjectClass *obj_class = (GObjectClass *)klass;

	obj_class->dispose = xmr_service_dispose;

	g_type_class_add_private(obj_class, sizeof(XmrServicePrivate));
}

static void xmr_service_init(XmrService *xs)
{
	xs->priv = XMR_SERVICE_GET_PRIVATE(xs);

	xs->priv->logged = FALSE;
	xs->priv->curl = NULL;
}

GObject* xmr_service_new()
{
	return g_object_new(XMR_SERVICE_TYPE,
				NULL);
}

gboolean
xmr_service_login(XmrService *xs,
			const gchar *usr,
			const gchar *pwd
			)
{
	g_return_val_if_fail(xs != NULL, FALSE);
	return FALSE;
}

gboolean
xmr_service_is_logged_in(XmrService *xs)
{
	g_return_val_if_fail(xs != NULL, FALSE);

	return FALSE;
}

void
xmr_service_get_track_list(XmrService *xs, GList **list)
{
	g_return_if_fail(xs != NULL);
}
