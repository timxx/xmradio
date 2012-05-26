#include <curl/curl.h>
#include <libxml/parser.h>

#include "xmrservice.h"
#include "songinfo.h"

#define XMR_SERVICE_GET_PRIVATE(obj)	\
	(G_TYPE_INSTANCE_GET_PRIVATE((obj), XMR_SERVICE_TYPE, XmrServicePrivate))

#define XMR_USER_AGENT	"XiaMiRadio/0.1"
#define XMR_LOGIN_URL	"http://www.xiami.com/kuang/login"

G_DEFINE_TYPE(XmrService, xmr_service, G_TYPE_OBJECT);

enum
{
	PROP_0,
	PROP_USR_ID,
	PROP_USR_NAME
};

struct _XmrServicePrivate
{
	gboolean logged;	// user logged in ?

	const gchar	*usr_id;
	const gchar	*usr_name;

	CURL *curl;
};

static size_t
write_func(void *ptr, size_t size, size_t nmemb, void *data);

static gint
get_url_data(XmrService *xs, const gchar *url, GString *data);

static gint
post_url_data(XmrService *xs, const gchar *url, GString *post_data, GString *data);

static gboolean
parse_login_status(XmrService *xs, GString *data);

static xmlNodePtr
xml_first_child(xmlNodePtr root, const xmlChar *child);

static xmlChar*
xml_first_child_content(xmlNodePtr root, const xmlChar *child);

static void
xmr_service_set_property(GObject      *object,
			 guint         prop_id,
			 const GValue *value,
			 GParamSpec   *pspec)
{
	XmrService *xs = XMR_SERVICE(object);

	switch(prop_id)
	{
		case PROP_USR_ID:
			xs->priv->usr_id = g_value_get_string(value);
			break;

		case PROP_USR_NAME:
			xs->priv->usr_name = g_value_get_string(value);
			break;
	}
}

static void
xmr_service_get_property (GObject    *object,
			 guint       prop_id,
			 GValue     *value,
			 GParamSpec *pspec)
{
	XmrService *xs = XMR_SERVICE(object);

	switch (prop_id)
	{
		case PROP_USR_ID:
			g_value_set_string(value, xs->priv->usr_id);
			break;

		case PROP_USR_NAME:
			g_value_set_string(value, xs->priv->usr_name);
			break;
	}
}

static void 
xmr_service_dispose(GObject *obj)
{
	XmrService *xs;
	XmrServicePrivate *priv;

	g_return_if_fail (obj != NULL);

	xs = XMR_SERVICE(obj);
	priv = xs->priv;

	if (priv->curl)
	{
		curl_easy_cleanup(priv->curl);
		priv->curl = NULL;
	}

	G_OBJECT_CLASS(xmr_service_parent_class)->dispose(obj);
}

static void xmr_service_class_init(XmrServiceClass *klass)
{
	GObjectClass *obj_class = (GObjectClass *)klass;

	obj_class->dispose = xmr_service_dispose;
	obj_class->set_property = xmr_service_set_property;
	obj_class->get_property = xmr_service_get_property;

	g_object_class_install_property(obj_class,
					 PROP_USR_ID,
					 g_param_spec_string("usr-id",
							      "User id",
							      "User id",
								  NULL,
								  G_PARAM_READWRITE		|
							      G_PARAM_STATIC_NAME	|
							      G_PARAM_STATIC_NICK	|
							      G_PARAM_STATIC_BLURB
						 ));

	g_object_class_install_property(obj_class,
					 PROP_USR_NAME,
					 g_param_spec_string("usr-name",
							      "User name",
							      "User name",
								  NULL,
								  G_PARAM_READWRITE		|
							      G_PARAM_STATIC_NAME	|
							      G_PARAM_STATIC_NICK	|
							      G_PARAM_STATIC_BLURB
						 ));

	g_type_class_add_private(obj_class, sizeof(XmrServicePrivate));
}

static void xmr_service_init(XmrService *xs)
{
	XmrServicePrivate *priv;

	xs->priv = XMR_SERVICE_GET_PRIVATE(xs);
	priv = xs->priv;

	priv->logged = FALSE;
	priv->usr_id = NULL;
	priv->usr_name = NULL;

	priv->curl = curl_easy_init();
	if (priv->curl)
	{
        curl_easy_setopt(priv->curl, CURLOPT_USERAGENT, XMR_USER_AGENT);
		curl_easy_setopt(priv->curl, CURLOPT_CONNECTTIMEOUT, 10);
		curl_easy_setopt(priv->curl, CURLOPT_NOSIGNAL, 1L);
    }
}

GObject* xmr_service_new()
{
	return g_object_new(XMR_SERVICE_TYPE,
				NULL);
}

gint
xmr_service_login(XmrService *xs,
			const gchar *usr,
			const gchar *pwd
			)
{
	GString *data;
	GString *post_data = NULL;
	gint result = 1;

	g_return_val_if_fail(xs != NULL, FALSE);

	data = g_string_new("");
	g_return_val_if_fail(data != NULL, FALSE);

	do
	{
		post_data = g_string_new("");
		if (post_data == NULL)
			break;

		g_string_printf(post_data, "password=%s&email=%s", pwd, usr);

		if (post_url_data(xs, XMR_LOGIN_URL, post_data, data) != CURLE_OK)
			break;

		result = parse_login_status(xs, data);
	}
	while(0);

	if (result == 0){
		xs->priv->logged = TRUE;
	}

	g_string_free(data, TRUE);
	if (post_data){
		g_string_free(post_data, TRUE);
	}

	return result;
}

gboolean
xmr_service_is_logged_in(XmrService *xs)
{
	g_return_val_if_fail(xs != NULL, FALSE);

	return xs->priv->logged;
}

void
xmr_service_get_track_list(XmrService *xs, GList **list)
{
	g_return_if_fail(xs != NULL);

	get_url_data(xs, NULL, NULL);
}

static gint
get_url_data(XmrService *xs, const gchar *url, GString *data)
{
	XmrServicePrivate *priv;
	gint ret = -1;

	priv = xs->priv;

    curl_easy_setopt(priv->curl, CURLOPT_URL, url);
	curl_easy_setopt(priv->curl, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(priv->curl, CURLOPT_FOLLOWLOCATION, 1);

	curl_easy_setopt(priv->curl, CURLOPT_WRITEFUNCTION, write_func);
	curl_easy_setopt(priv->curl, CURLOPT_WRITEDATA, data);

	//curl_easy_setopt(priv->curl, CURLOPT_VERBOSE, 1L);

    ret = curl_easy_perform(priv->curl);

	return ret;
}

static size_t
write_func(void *ptr, size_t size, size_t nmemb, void *data)
{
    size_t real_size = size * nmemb;
	GString *g_string;

    if (ptr == NULL)
        return 0;

	g_string = (GString *)data;

	g_string = g_string_append(g_string, (gchar *)ptr);

    return real_size;
}

static gint
post_url_data(XmrService *xs, const gchar *url, GString *post_data, GString *data)
{
	XmrServicePrivate *priv;
	gint result = -1;

	priv = xs->priv;
	
	curl_easy_setopt(priv->curl, CURLOPT_URL, url);
    curl_easy_setopt(priv->curl, CURLOPT_POST, 1L);

	curl_easy_setopt(priv->curl, CURLOPT_POSTFIELDS, post_data->str);
    curl_easy_setopt(priv->curl, CURLOPT_POSTFIELDSIZE, post_data->len);

	curl_easy_setopt(priv->curl, CURLOPT_WRITEFUNCTION, write_func);
	curl_easy_setopt(priv->curl, CURLOPT_WRITEDATA, data);

    //curl_easy_setopt(priv->curl, CURLOPT_VERBOSE, 1L);

	result = curl_easy_perform(priv->curl);

	return result;
}

static gint
parse_login_status(XmrService *xs, GString *data)
{
	gboolean result = 1;
	xmlDocPtr doc = NULL;
	xmlNodePtr node = NULL;

	doc = xmlReadMemory(data->str, data->len, NULL, NULL,
				XML_PARSE_RECOVER | XML_PARSE_NOERROR);
	if (doc == NULL)
		return FALSE;

	do
	{
		xmlNodePtr child;
		xmlChar *value;

		node = xmlDocGetRootElement(doc);
		if (node == NULL)
			break;

		 child = xml_first_child(node, BAD_CAST "status");
		 if (child == NULL)
			break;

		 value = xmlNodeGetContent(child);
		 if (value == NULL)
			break;
		 
		result = g_strtod((gchar *)value, NULL);
		xmlFree(value);

		// get user info
		if (result == 0)
		{
			node = xml_first_child(node, BAD_CAST "user");
			if (node == NULL)
				break;

			value = xml_first_child_content(node, BAD_CAST "user_id");
			if (value)
			{
				g_object_set(xs, "usr-id", value, NULL);
				xmlFree(value);
			}

			value = xml_first_child_content(node, BAD_CAST "nick_name");
			if (value)
			{
				g_object_set(xs, "usr-name", value, NULL);
				xmlFree(value);
			}
		}
		 
	}
	while(0);

	xmlFreeDoc(doc);

	return result;
}

static xmlNodePtr
xml_first_child(xmlNodePtr root, const xmlChar *child)
{
	xmlNodePtr p = (root == NULL ? NULL : root->children);

	for( ; p ; p = p->next)
	{
		if (xmlStrEqual(p->name, child))
			return p;
	}

	return NULL;
}

static xmlChar*
xml_first_child_content(xmlNodePtr root, const xmlChar *child)
{
	xmlNodePtr p = xml_first_child(root, child);
	if (p){
		return xmlNodeGetContent(p);
	}

	return NULL;
}
