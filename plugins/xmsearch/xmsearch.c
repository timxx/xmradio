/** 
 * xmsearch.c
 *
 * Copyright (C) 2013, 2017  Weitian Leung (weitianleung@gmail.com)

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

static SongInfo *
get_song_id_info(XmrSearchPlugin *plugin, const gchar *id)
{
	XmrWindow *window = NULL;
	g_object_get(plugin, "object", &window, NULL);
	g_assert(window != NULL);

	XmrService *service = NULL;
	g_object_get(window, "service", &service, NULL);
	g_assert(service != NULL);

	gchar *url = g_strdup_printf(XIAMI_INFO_URL"%s", id);
	GString *data = g_string_new("");
	xmr_service_get_url_data(service, url, data);
	g_free(url);

	g_object_unref(service);
	g_object_unref(window);

	SongInfo *info = xmr_track_to_songinfo(data);

	g_string_free(data, TRUE);

	return info;
}

static GList *
parse_result_data(XmrSearchPlugin *plugin, const gchar *data, long len)
{
	GList *list = NULL;

	const gchar *current = data;
	const gchar *p;

	p = strstr(current, "<h5>歌曲</h5>");
	if (p == NULL)
		return NULL;

	len -= (p + 15 - current);
	current = p + 15; // <h5>歌曲</h5>

	p = strstr(current, "class=\"result_main\"");
	if (p == NULL)
		return NULL;

	len -= (p + 19 - current);
	current = p + 19; // class="result_main"

#define MOVE_CURRENT(offset)	\
	len -= (p + offset - current);	\
	if (len <= 0) break;		\
	current = p + offset;

	while (len > 0 && (p = strstr(current, "<td class=\"song_name\">")) != NULL)
	{
		gchar *value = NULL;
		SongInfo *info = NULL;

		MOVE_CURRENT(22) // 22 = <td class="song_name">

		// song_act
		do
		{
			const gchar *end = NULL;
			p = strstr(current, "<td class=\"song_act\">");
			if (p == NULL)
				break;

			MOVE_CURRENT(21) // <td class="song_act">

			end = strstr(current, "</td>");

			p = strstr(current, "onclick=\"play('");
			if (p == NULL)
				break;

			MOVE_CURRENT(15) // 15 = onclick="play('

			// maybe the song is disabled
			if (end < p)
				break;

			p = current;

			while (len > 0 && current && *current && *current != '\'')
				++current, --len;

			value = g_strndup(p, current - p);
			info = get_song_id_info(plugin, value);
			g_free(value);
		} while (0);

#undef MOVE_CURRENT

		if (info == NULL)
			continue;

		if (info->song_id == NULL)
			song_info_free(info);
		else
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
		GList *list = parse_result_data(data->plugin, result_data->str, result_data->len);
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
