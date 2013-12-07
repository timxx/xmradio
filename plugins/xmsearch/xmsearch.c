/** 
 * xmsearch.c
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

#include <glib/gi18n.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <curl/curl.h>

#include "xmrplugin.h"
#include "xmrwindow.h"
#include "lib/songinfo.h"
#include "lib/xmrservice.h"

#define XMR_TYPE_SEARCH_PLUGIN			(xmr_search_plugin_get_type())
#define XMR_SEARCH_PLUGIN(o)			(G_TYPE_CHECK_INSTANCE_CAST((o), XMR_TYPE_SEARCH_PLUGIN, XmrSearchPlugin))
#define XMR_SEARCH_PLUGIN_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), XMR_TYPE_SEARCH_PLUGIN, XmrSearchPluginClass))
#define XMR_IS_SEARCH_PLUGIN(o)			(G_TYPE_CHECK_INSTANCE_TYPE((o), XMR_TYPE_SEARCH_PLUGIN))
#define XMR_IS_SEARCH_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE((k), XMR_TYPE_SEARCH_PLUGIN))
#define XMR_SEARCH_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS((o),  XMR_TYPE_SEARCH_PLUGIN, XmrSearchPluginClass))

typedef struct
{
	PeasExtensionBase parent;
	GAsyncQueue *event_queue;
	guint event_idle_id;
	gint thread_count;
	gint last_thread_count;
	GMutex *mutex;
}XmrSearchPlugin;

typedef struct
{
	PeasExtensionBaseClass parent_class;
}XmrSearchPluginClass;

typedef struct
{
	gchar *keyword;
	XmrSearchPlugin *plugin;
}Data;

XMR_DEFINE_PLUGIN(XMR_TYPE_SEARCH_PLUGIN, XmrSearchPlugin, xmr_search_plugin,)

#define XIAMI_SEARCH_URL "http://www.xiami.com/search?key="
#define XIAMI_INFO_URL "http://www.xiami.com/song/playlist/id/"

#define SONG_PATTERN "class=\"song_name\""
#define ARTIST_PATTERN "class=\"song_artist\""
#define ALBUM_PATTERN "class=\"song_album\""

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

static gboolean
event_poll_idle(XmrSearchPlugin *plugin)
{
	gboolean ret = plugin->thread_count == 0 ? FALSE : TRUE;
	XmrWindow *window = NULL;
	GList *list;

	if (!ret)
	{
		plugin->event_idle_id = 0;
	}
	
	g_object_get(plugin, "object", &window, NULL);
	
	if (plugin->last_thread_count != plugin->thread_count)
	{
		plugin->last_thread_count--;
		xmr_window_decrease_search_music_count(window);
	}

	list = g_async_queue_try_pop(plugin->event_queue);
	if (list == NULL)
	{
		g_object_unref(window);
		return ret;
	}
	
	// update search result list
	xmr_window_set_search_result(window, list, _("XiaMi"), "www.xiami.com");
	g_list_free_full(list, (GDestroyNotify)song_info_free);
		
	g_object_unref(window);
	
	return ret;
}

static char*
get_title_attr(const gchar *content, const gchar **end)
{
	const gchar *p, *q;
	*end = content;
	p = strstr(content, "title=\"");
	if (p == NULL)
		return NULL;

	content = p + strlen("title=\"");
	q = strchr(content, '"');
	if (q == NULL)
		return NULL;

	*end = q + 1;
	return g_strndup(content, q - content);
}

static SongInfo *
get_song_id_info(const gchar *id)
{
	SongInfo *info = NULL;
	XmrService *service = xmr_service_new();
	gchar *url = g_strdup_printf(XIAMI_INFO_URL"%s", id);
	GString *data = g_string_new("");
	gint result = xmr_service_get_url_data(service, url, data);
	g_object_unref(service);

	do
	{
		const gchar *p = data->str;
		const gchar *q;
		gchar *tmp;

		if (result != 0)
			break;
		
		p = strstr(p, "<location>");
		if (p == NULL)
			break;
		p += strlen("<location>");

		q = strstr(p, "</location>");
		if (q == NULL)
			break;

		// skip white space
		while (*p && isspace(*p))
			p++;
		
		while (q > p && isspace(*(q-1)))
			q--;

		info = song_info_new();
		tmp = g_strndup(p, q - p);
		info->location = decode_url(tmp);
		g_free(tmp);
		
		p = strstr(q, "<pic>");
		if (p == NULL)
			break;
		p += strlen("<pic>");

		q = strstr(p, "</pic>");
		if (q == NULL)
			break;
		
		// skip white space
		while (*p && isspace(*p))
			p++;
		
		while (q > p && isspace(*(q-1)))
			q--;
		
		info->album_cover = g_strndup(p, q - p);
		info->song_id = g_strdup(id);
	}while (0);

	decode_url(id);
	
	g_free(url);
	g_string_free(data, TRUE);
	return info;
}

static GList *
parse_result_data(const gchar *data)
{
	GList *list = NULL;

	const gchar *current = data;
	const gchar *p;

	/*
	<td class="song_name">
		<a target="_blank" href="http://www.xiami.com/song/1770779526" title="中国人"><b class="key_red">中国人</b></a>
	</td>
	<td class="song_artist">
		<a target="_blank" href="http://www.xiami.com/artist/648" title="刘德华">刘德华</a>
	</td>
	<td class="song_album">
		<a target="_blank" href="http://www.xiami.com/album/491512" title="Unforgettable 2011中国巡回演唱会">《Unforgettable 2011中国巡回演唱会》</a>
	</td>
	*/
	while ((p = strstr(current, SONG_PATTERN)) != NULL)
	{
		gchar *tmp;
		SongInfo *info;
		current = p + strlen(SONG_PATTERN);
		p = strstr(current, "/song/");
		if (p == NULL)
			break;

		current = p + strlen("/song/");
		if (!isdigit(*current))
			continue;
		
		p = current + 1;
		while (p && isdigit(*p))
			p++;

		// song id
		tmp = g_strndup(current, p - current);
		// get song info
		info = get_song_id_info(tmp);
		g_free(tmp);
		if (!info)
			continue ;

		current = p + 1;
		// song name
		tmp = get_title_attr(current, &current);
		if (tmp == NULL)
		{
			song_info_free(info);
			continue;
		}
		info->song_name = tmp;

		// artist name
		p = strstr(current, ARTIST_PATTERN);
		if (p != NULL)
		{
			tmp = get_title_attr(p, &current);
			if (tmp != NULL)
				info->artist_name = tmp;
		}
		
		current = p + 1;
		// album name
		p = strstr(current, ALBUM_PATTERN);
		if (p != NULL)
		{
			tmp = get_title_attr(p, &current);
			if (tmp != NULL)
				info->album_name = tmp;
		}
		
		list = g_list_append(list, info);
	}
		
	return list;
}

static gpointer
search_thread(Data *data)
{
	XmrService *service = NULL;
	gchar *url;
	gchar *escape_keyword;
	GString *result_data;
	gint result;
	
	escape_keyword = curl_escape(data->keyword, 0);
	url = g_strdup_printf(XIAMI_SEARCH_URL"%s",escape_keyword);
	result_data = g_string_new("");
	
	service = xmr_service_new();
	result = xmr_service_get_url_data(service, url, result_data);
	g_object_unref(service);

	if (result == 0)
	{
		GList *list = parse_result_data(result_data->str);
		if (list)
			g_async_queue_push(data->plugin->event_queue, list);
	}

	g_mutex_lock(data->plugin->mutex);
	data->plugin->thread_count--;
	g_mutex_unlock(data->plugin->mutex);

	g_free(url);
	curl_free(escape_keyword);
	g_string_free(result_data, TRUE);
	g_free(data->keyword);
	g_free(data);

	return NULL;
}

static void
on_music_search(XmrWindow *window,
				const gchar *keyword,
				XmrSearchPlugin *plugin)
{
	Data *data = g_new(Data, 1);
	data->keyword = g_strdup(keyword);
	data->plugin = plugin;

	plugin->thread_count++;
	plugin->last_thread_count++;
	if (plugin->event_idle_id == 0)
		plugin->event_idle_id = g_timeout_add(200, (GSourceFunc)event_poll_idle, plugin);

	xmr_window_increase_search_music_count(window);

#if GLIB_CHECK_VERSION(2, 32, 0)
	g_thread_new("search_music", (GThreadFunc)search_thread, data);
#else
	g_thread_create((GThreadFunc)search_thread, data, FALSE, NULL);
#endif
}

static void
impl_activate(PeasActivatable *activatable)
{
	XmrSearchPlugin *plugin;
	XmrWindow *window = NULL;

	plugin = XMR_SEARCH_PLUGIN(activatable);
	g_object_get(plugin, "object", &window, NULL);

	if (window)
	{
		g_signal_connect(window, "search-music", G_CALLBACK(on_music_search), plugin);
		g_object_unref(window);
	}
	
	plugin->event_queue = g_async_queue_new();
	plugin->event_idle_id = 0;
	plugin->thread_count = 0;
	plugin->last_thread_count = 0;
#if GLIB_CHECK_VERSION(2, 32, 0)
	plugin->mutex = g_malloc(sizeof(GMutex));
	g_mutex_init(plugin->mutex);
#else
	plugin->mutex = g_mutex_new();
#endif
}

static void
impl_deactivate(PeasActivatable *activatable)
{
	XmrSearchPlugin *plugin;
	XmrWindow *window = NULL;

	plugin = XMR_SEARCH_PLUGIN(activatable);
	g_object_get(plugin, "object", &window, NULL);

	if (window)
	{
		g_signal_handlers_disconnect_by_func(window, on_music_search, plugin);
		g_object_unref(window);
	}
	
	if (plugin->event_idle_id)
		g_source_remove(plugin->event_idle_id);

	g_async_queue_unref(plugin->event_queue);
	
	if (plugin->mutex)
	{
#if GLIB_CHECK_VERSION(2, 32, 0)
		g_mutex_clear(plugin->mutex);
		g_free(plugin->mutex);
#else
		g_mutex_free(plugin->mutex);
#endif
		plugin->mutex = NULL;
	}
}

static void
xmr_search_plugin_init(XmrSearchPlugin *plugin)
{
}

G_MODULE_EXPORT void
peas_register_types(PeasObjectModule *module)
{
	xmr_search_plugin_register_type(G_TYPE_MODULE(module));
	peas_object_module_register_extension_type(module,
							PEAS_TYPE_ACTIVATABLE,
							XMR_TYPE_SEARCH_PLUGIN);
}
