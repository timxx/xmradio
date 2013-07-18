/** 
 * xmrservice.c
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
#include <curl/curl.h>
#include <libxml/parser.h>
#include <string.h>
#include <unistd.h>

#include "xmrservice.h"
#include "songinfo.h"
#include "radioinfo.h"
#include "config.h"

#define XMR_SERVICE_GET_PRIVATE(obj)	\
	(G_TYPE_INSTANCE_GET_PRIVATE((obj), XMR_SERVICE_TYPE, XmrServicePrivate))

#define XMR_USER_AGENT	"XiaMiRadio/0.1"
#define XMR_LOGIN_URL	"http://www.xiami.com/kuang/login"
#define XMR_LOGOUT_URL	"http://www.xiami.com/member/logout"

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

	gchar	*usr_id;
	gchar	*usr_name;

	CURL	*curl;
	
	gchar	*cookie;	// cookie file
};

/**
 * curl data receive handler
 */
static size_t
write_func(void *ptr, size_t size, size_t nmemb, void *data);

static gint
post_url_data(XmrService *xs, const gchar *url, GString *post_data, GString *data);

/**
 * parse login status data
 */
static gint
parse_login_status(XmrService *xs, GString *data, gchar **message);

static xmlNodePtr
xml_first_child(xmlNodePtr root, const xmlChar *child);

static xmlChar*
xml_first_child_content(xmlNodePtr root, const xmlChar *child);

static gint
parse_track_list_data(GString *data, GList **list);

/**
 * get track info
 * and append to list
 */
static void
get_track(xmlNodePtr root, GList **list);

/**
 * decode url
 */
static gchar *
decode_url(const gchar *url);

static gint
parse_radio_list(GString *data, GList **list);

static void
get_radio(xmlNodePtr root, GList **list);

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
			g_free(xs->priv->usr_id);
			xs->priv->usr_id = g_value_dup_string(value);
			break;

		case PROP_USR_NAME:
			g_free(xs->priv->usr_name);
			xs->priv->usr_name = g_value_dup_string(value);
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

	if (priv->usr_id)
		g_free(priv->usr_id);
	if (priv->usr_name)
		g_free(priv->usr_name);

	if (priv->curl)
	{
		curl_easy_cleanup(priv->curl);
		priv->curl = NULL;
	}
	
	if (priv->cookie)
	{
		g_free(priv->cookie);
		priv->cookie = NULL;
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
								  G_PARAM_READWRITE
						 ));

	g_object_class_install_property(obj_class,
					 PROP_USR_NAME,
					 g_param_spec_string("usr-name",
							      "User name",
							      "User name",
								  NULL,
								  G_PARAM_READWRITE
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
	priv->cookie = NULL;
	{
		gint fd = g_file_open_tmp(NULL, &priv->cookie, NULL);
		if (fd != -1)
			close(fd);
	}

	priv->curl = curl_easy_init();
	if (priv->curl)
	{
        curl_easy_setopt(priv->curl, CURLOPT_USERAGENT, XMR_USER_AGENT);
		curl_easy_setopt(priv->curl, CURLOPT_CONNECTTIMEOUT, 10);
		curl_easy_setopt(priv->curl, CURLOPT_NOSIGNAL, 1L);
    }
}

XmrService* xmr_service_new()
{
	return g_object_new(XMR_SERVICE_TYPE,
				NULL);
}

gint
xmr_service_login(XmrService *xs,
			const gchar *usr,
			const gchar *pwd,
			gchar **message
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

		result = post_url_data(xs, XMR_LOGIN_URL, post_data, data);
		if (result != CURLE_OK)
		{
			*message = g_strdup(curl_easy_strerror(result));
			break;
		}

		result = parse_login_status(xs, data, message);
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
xmr_service_logout(XmrService *xs)
{
	GString *data;
	g_return_if_fail(xs != NULL);

	// not logged in, ignore
	if (!xs->priv->logged)
		return ;

	xs->priv->logged = FALSE;
	data = g_string_new("");

	xmr_service_get_url_data(xs, XMR_LOGOUT_URL, data);

	g_string_free(data, TRUE);
}

gint
xmr_service_get_track_list_by_id(XmrService *xs, GList **list, int type)
{
	gchar *url = NULL;
	gint result;

	g_return_val_if_fail(xs != NULL, 1);

	if (!xs->priv->logged || xs->priv->usr_id == NULL)
	{
		g_warning("You should login first\n");
		return 1;
	}

	switch(type)
	{
	case 0: // 我的电台
		url = g_strdup_printf("http://www.xiami.com/radio/xml/type/4/id/%s", xs->priv->usr_id);
		break;

	case 1: // 虾米猜
		url = g_strdup_printf("http://www.xiami.com/radio/xml/type/2/id/%s", xs->priv->usr_id);
		break;
	}

	result = xmr_service_get_track_list_by_style(xs, list, url);

	g_free(url);

	return result;
}

gint
xmr_service_get_track_list_by_style(XmrService *xs, GList **list, const gchar *url)
{
	gint result = 1;
	GString *data;

	g_return_val_if_fail(xs != NULL, 1);

	data = g_string_new("");

	result = xmr_service_get_url_data(xs, url, data);
	if (result == 0){
		parse_track_list_data(data, list);
	}

	g_string_free(data, TRUE);

	return result;
}

gint
xmr_service_get_radio_list(XmrService *xs, GList **list, gint style)
{
	gint result = 1;
	GString *data;
	gchar *url;

	g_return_val_if_fail(xs != NULL, 1);	

	data = g_string_new("");
	if (data == NULL)
		return result;

	url = g_strdup_printf("http://www.xiami.com/kuang/radio/c/%d", style);
	if (url == NULL)
	{
		g_string_free(data, TRUE);
		return result;
	}

	result = xmr_service_get_url_data(xs, url, data);
	if (result == 0){
		parse_radio_list(data, list);
	}

	g_free(url);
	g_string_free(data, TRUE);

	return result;
}

gint
xmr_service_get_url_data(XmrService *xs, const gchar *url, GString *data)
{
	XmrServicePrivate *priv;
	gint ret = -1;

	priv = xs->priv;

    curl_easy_setopt(priv->curl, CURLOPT_URL, url);
	curl_easy_setopt(priv->curl, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(priv->curl, CURLOPT_FOLLOWLOCATION, 1);

	curl_easy_setopt(priv->curl, CURLOPT_WRITEFUNCTION, write_func);
	curl_easy_setopt(priv->curl, CURLOPT_WRITEDATA, data);
	
	if (priv->cookie)
		curl_easy_setopt(priv->curl, CURLOPT_COOKIEFILE, priv->cookie);

#ifdef _DEBUG
	curl_easy_setopt(priv->curl, CURLOPT_VERBOSE, 1L);
#endif

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

	// GString doesn't support '\0'
	// g_string = g_string_append(g_string, (gchar *)ptr);
	g_string->str = (gchar *)g_realloc(g_string->str, g_string->len + real_size + 1);
    if (g_string->str == NULL)
        return 0;

    if (memcpy(&(g_string->str[g_string->len]), ptr, real_size) == NULL)
        return 0;

    g_string->len += real_size;
    g_string->str[g_string->len] = 0;

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
	
	if (priv->cookie)
		curl_easy_setopt(priv->curl, CURLOPT_COOKIEJAR, priv->cookie);

#ifdef _DEBUG
	curl_easy_setopt(priv->curl, CURLOPT_VERBOSE, 1L);
#endif

	result = curl_easy_perform(priv->curl);

	return result;
}

static gint
parse_login_status(XmrService *xs, GString *data, gchar **message)
{
	gboolean result = 1;
	xmlDocPtr doc = NULL;
	xmlNodePtr node = NULL;

	doc = xmlReadMemory(data->str, data->len, NULL, NULL,
				XML_PARSE_RECOVER | XML_PARSE_NOERROR);
	if (doc == NULL)
		return result;

	do
	{
		xmlNodePtr child;
		xmlChar *value;

		node = xmlDocGetRootElement(doc);
		if (node == NULL)
			break;

		if (message)
		{
			child = xml_first_child(node, BAD_CAST "message");
			if (child)
			{
				value = xmlNodeGetContent(child);
				*message = (gchar *)value;
			}
		}

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

static gint
parse_track_list_data(GString *data, GList **list)
{
	gint result = 1;
	xmlDocPtr doc = NULL;

	doc = xmlReadMemory(data->str, data->len, NULL, NULL,
				XML_PARSE_RECOVER | XML_PARSE_NOERROR);

	if (doc == NULL)
		return result;

	do
	{
		xmlNodePtr root;
		xmlNodePtr track_list;
		xmlNodePtr p;

		root = xmlDocGetRootElement(doc);
		if (root == NULL)
			break;

		track_list = xml_first_child(root, BAD_CAST "trackList");
		if (track_list == NULL)
			break;

		for(p = xmlFirstElementChild(track_list); p ; p = xmlNextElementSibling(p))
			get_track(p, list);

		result = 0;
	}
	while(0);

	xmlFreeDoc(doc);

	return result;
}

static void
get_track(xmlNodePtr root, GList **list)
{
	SongInfo *song;
	gchar *url;

	song = song_info_new();
	if (song == NULL)
		return ;

	do
	{
		song->song_id		= (gchar *)xml_first_child_content(root, BAD_CAST "song_id");
		if (song->song_id == NULL)
			break;

		song->song_name		= (gchar *)xml_first_child_content(root, BAD_CAST "title");
		song->album_id		= (gchar *)xml_first_child_content(root, BAD_CAST "album_id");
		song->album_name	= (gchar *)xml_first_child_content(root, BAD_CAST "album_name");
		song->artist_id		= (gchar *)xml_first_child_content(root, BAD_CAST "artist_id");
		song->artist_name	= (gchar *)xml_first_child_content(root, BAD_CAST "artist");
		song->album_cover	= (gchar *)xml_first_child_content(root, BAD_CAST "pic");
		url					= (gchar *)xml_first_child_content(root, BAD_CAST "location");
		if (url == NULL)
			break;

		song->location		= decode_url(url);
		g_free(url);

		song->grade			= (gchar *)xml_first_child_content(root, BAD_CAST "grade");

		*list = g_list_append(*list, song);

		return ;
	}
	while(0);

	song_info_free(song);
}

static gchar *
decode_url(const gchar *url)
{
	int col = 0;
	int row;
	int x;
	int len;
	int i, j, k;
	gchar *array;
	gchar *decode_url = NULL;
	gchar *p;

	if (url == NULL)
		return NULL;

	row = *url - '0';
	len = strlen(url + 1);
	col = len / row;
	x = len % row;

	if (x != 0)
	  col += 1;

	array = (char *)calloc(len + sizeof(char), sizeof(char));

	k = 1;
	for(i=0; i<row; i++)
	{
		for(j=0; j<col; j++, k++)
			array[i + row * j] = url[k];

		if (x > 0)
		{
			x--;
			if(x == 0)
			 col -= 1;
		}
	}

	decode_url = curl_easy_unescape(NULL, array, 0, NULL);
	p = decode_url;

	while(*p)
	{
		if (*p == '^')
			*p = '0';
		p++;
	}
	free(array);

	return decode_url;
}

static gint
parse_radio_list(GString *data, GList **list)
{
	gint result = 1;
	xmlDocPtr doc;

	if (data->str == NULL || data->len == 0)
		return 1;

	doc = xmlReadMemory(data->str, data->len, NULL, NULL,
				XML_PARSE_RECOVER | XML_PARSE_NOERROR);

	if (doc == NULL)
		return result;

	do
	{
		xmlNodePtr radioList;
		xmlNodePtr p;

		radioList = xmlDocGetRootElement(doc);
		if (radioList == NULL)
			break;

		for(p=xmlFirstElementChild(radioList); p; p=xmlNextElementSibling(p))
			get_radio(p, list);

		result = 0;
	}
	while(0);

	xmlFreeDoc(doc);

	return result;
}

static void
get_radio(xmlNodePtr root, GList **list)
{
	RadioInfo *radio;

	radio = radio_info_new();
	if (radio == NULL)
		return ;

	radio->id	= (gchar *)xml_first_child_content(root, BAD_CAST "radio_id");
	radio->name = (gchar *)xml_first_child_content(root, BAD_CAST "radio_name");
	radio->logo = (gchar *)xml_first_child_content(root, BAD_CAST "radio_logo");
	radio->url	= (gchar *)xml_first_child_content(root, BAD_CAST "radio_url");

	*list = g_list_append(*list, radio);
}

gint
xmr_service_like_song(XmrService *xs, const gchar *song_id, gboolean like)
{
	gint result = 1;
	gchar *url;
	GString *data;

	g_return_val_if_fail(xs != NULL, 1);

	if (like){
		url = g_strdup_printf("http://www.xiami.com/kuang/like/id/%s", song_id);
	}else{
		url = g_strdup_printf("http://www.xiami.com/kuang/unlike/id/%s", song_id);
	}
	if (url == NULL)
		return result;

	data = g_string_new("");

	result = xmr_service_get_url_data(xs, url, data);

	g_string_free(data, TRUE);

	g_free(url);

	return result;
}

gint
xmr_service_get_lyric(XmrService *xs, const gchar *song_id, GString *data)
{
	gint result = 1;
	gchar *url;

	g_return_val_if_fail(xs != NULL && data != NULL, 1);

	url = g_strdup_printf("http://www.xiami.com/radio/lyric/sid/%s", song_id);
	if (url == NULL)
		return result;

	result = xmr_service_get_url_data(xs, url, data);

	g_free(url);

	return result;
}

const gchar *
xmr_service_get_usr_id(XmrService *xs)
{
	g_return_val_if_fail(xs != NULL, NULL);

	return xs->priv->usr_id;
}

const gchar *
xmr_service_get_usr_name(XmrService *xs)
{
	g_return_val_if_fail(xs != NULL, NULL);

	return xs->priv->usr_name;
}

const gchar *
xmr_service_get_error_str(gint code)
{
	return curl_easy_strerror(code);
}
