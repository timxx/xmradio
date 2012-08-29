/**
 * xmrdownloader.c
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
#include <glib/gi18n.h>
#include <gdk/gdk.h>

#include "xmrdownloader.h"
#include "xmrmarshal.h"

G_DEFINE_TYPE(XmrDownloader, xmr_downloader, G_TYPE_OBJECT)
//==========================================================================
struct _XmrDownloaderPrivate
{
	GSList *tasks;

	GMutex *mutex;
};

typedef struct
{
	XmrDownloader *downloader;
	gchar *url;
	gchar *file;
} Task;

enum
{
	DOWNLOAD_FINISH,
	DOWNLOAD_PROGRESS,
	DOWNLOAD_FAILED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

//==========================================================================

static size_t
my_write_func(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	return fwrite(ptr, size, nmemb, stream);
}

static int
my_progress_func(Task *task,
				 double total,
				 double now,
				 double ultotal,
				 double ulnow)
{
	gdk_threads_enter();
	g_signal_emit(task->downloader, signals[DOWNLOAD_PROGRESS], 0, task->url, now * 100.0 / total);
	gdk_threads_leave();

	return 0;
}

static gpointer
download_thread(gpointer data)
{
	CURL *curl = NULL;
	CURLcode res;
	FILE *fp = NULL;

	Task *task = (Task *)data;

	curl = curl_easy_init();

	if(curl == NULL)
	{
		gdk_threads_enter();
		g_signal_emit(task->downloader, signals[DOWNLOAD_FAILED], 0, task->url, _("curl_easy_init failed"));
		gdk_threads_leave();
		goto _exit;
	}

	fp = fopen(task->file, "w");
	if (fp == NULL)
	{
		gchar *message;

		message = g_strdup_printf(_("Unable to write file: %s"), task->file);
		gdk_threads_enter();
		g_signal_emit(task->downloader, signals[DOWNLOAD_FAILED], 0, task->url, message);
		gdk_threads_leave();

		g_free(message);
		goto _exit;
	}

	curl_easy_setopt(curl, CURLOPT_URL, task->url);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write_func);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, (curl_progress_callback)my_progress_func);
	curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, task);

	res = curl_easy_perform(curl);

	if (res == CURLE_OK)
	{
		gdk_threads_enter();
		g_signal_emit(task->downloader, signals[DOWNLOAD_FINISH], 0, task->url, task->file);
		gdk_threads_leave();
	}
	else
	{
		gchar *message;
		message = g_strdup_printf(_("curl_easy_perform failed: %s"), curl_easy_strerror(res));
		gdk_threads_enter();
		g_signal_emit(task->downloader, signals[DOWNLOAD_FAILED], 0, task->url, message);
		gdk_threads_leave();

		g_free(message);
	}

_exit:
	if (fp)
		fclose(fp);

	if (curl)
		curl_easy_cleanup(curl);

	g_mutex_lock(task->downloader->priv->mutex);
	task->downloader->priv->tasks = g_slist_remove(task->downloader->priv->tasks, task);
	g_mutex_unlock(task->downloader->priv->mutex);

	g_free(task->url);
	g_free(task->file);
	g_free(task);

	return NULL;
}

static void
free_task(Task *task)
{
	g_free(task->url);
	g_free(task->file);
}

static void
xmr_downloader_dispose(GObject *obj)
{
	XmrDownloader *downloader;
	XmrDownloaderPrivate *priv;

	g_return_if_fail (obj != NULL);

	downloader = XMR_DOWNLOADER(obj);
	priv = downloader->priv;

	if (priv->mutex)
	{
#if GLIB_CHECK_VERSION(2, 32, 0)
		g_mutex_clear(priv->mutex);
		g_free(priv->mutex);
#else
		g_mutex_free(priv->mutex);
#endif
		priv->mutex = NULL;
	}

	if (priv->tasks)
		g_slist_free_full(priv->tasks, (GDestroyNotify)free_task);

	G_OBJECT_CLASS(xmr_downloader_parent_class)->dispose(obj);
}

static void
xmr_downloader_class_init(XmrDownloaderClass *klass)
{
	GObjectClass *obj_class = (GObjectClass *)klass;

	obj_class->dispose = xmr_downloader_dispose;

	signals[DOWNLOAD_FINISH] =
		g_signal_new("download-finish",
					G_OBJECT_CLASS_TYPE(obj_class),
					G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
					G_STRUCT_OFFSET(XmrDownloaderClass, download_finish),
					NULL, NULL,
					xmr_marshal_VOID__STRING_STRING,
					G_TYPE_NONE,
					2,
					G_TYPE_STRING, G_TYPE_STRING);

	signals[DOWNLOAD_PROGRESS] =
		g_signal_new("download-progress",
					G_OBJECT_CLASS_TYPE(obj_class),
					G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
					G_STRUCT_OFFSET(XmrDownloaderClass, download_progress),
					NULL, NULL,
					xmr_marshal_VOID__STRING_DOUBLE,
					G_TYPE_NONE,
					2,
					G_TYPE_STRING, G_TYPE_DOUBLE);

	signals[DOWNLOAD_FAILED] =
		g_signal_new("download-failed",
					G_OBJECT_CLASS_TYPE(obj_class),
					G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
					G_STRUCT_OFFSET(XmrDownloaderClass, download_failed),
					NULL, NULL,
					xmr_marshal_VOID__STRING_STRING,
					G_TYPE_NONE,
					2,
					G_TYPE_STRING, G_TYPE_STRING);

	g_type_class_add_private(obj_class, sizeof(XmrDownloaderPrivate));
}

static void
xmr_downloader_init(XmrDownloader *downloader)
{
	XmrDownloaderPrivate *priv;

	downloader->priv = G_TYPE_INSTANCE_GET_PRIVATE(downloader, XMR_TYPE_DOWNLOADER, XmrDownloaderPrivate);
	priv = downloader->priv;

#if GLIB_CHECK_VERSION(2, 32, 0)
	priv->mutex = g_malloc(sizeof(GMutex));
	g_mutex_init(priv->mutex);
#else
	priv->mutex = g_mutex_new();
#endif

	priv->tasks = NULL;
}

XmrDownloader*	xmr_downloader_new()
{
	return g_object_new(XMR_TYPE_DOWNLOADER,
						NULL);
}

void
xmr_downloader_add_task(XmrDownloader *downloader,
						const gchar *url,
						const gchar *file)
{
	Task *task;
	XmrDownloaderPrivate *priv;

	g_return_if_fail(downloader != NULL && url != NULL && file != NULL);
	priv = downloader->priv;

	task = g_new0(Task, 1);

	task->downloader = downloader;
	task->url = g_strdup(url);
	task->file = g_strdup(file);

	priv->tasks = g_slist_prepend(priv->tasks, task);

	// 暂不处理任务数量。。。
#if GLIB_CHECK_VERSION(2, 32, 0)
	g_thread_new("download", (GThreadFunc)download_thread, task);
#else
	g_thread_create((GThreadFunc)download_thread, task, FALSE, NULL);
#endif
}

