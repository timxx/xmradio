#ifndef __XMR_DOWNLOADER_H__
#define __XMR_DOWNLOADER_H__
/**
 * xmrdownloader.h
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
#include <glib-object.h>

G_BEGIN_DECLS

#define XMR_TYPE_DOWNLOADER				(xmr_downloader_get_type())
#define XMR_DOWNLOADER(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XMR_TYPE_DOWNLOADER, XmrDownloader))
#define XMR_DOWNLOADER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass),  XMR_TYPE_DOWNLOADER, XmrDownloaderClass))
#define IS_XMR_DOWNLOADER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XMR_TYPE_DOWNLOADER))
#define IS_XMR_DOWNLOADER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass),  XMR_TYPE_DOWNLOADER))
#define XMR_DOWNLOADER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj),  XMR_TYPE_DOWNLOADER, XmrDownloaderClass))

typedef struct _XmrDownloader			XmrDownloader;
typedef struct _XmrDownloaderClass		XmrDownloaderClass;
typedef struct _XmrDownloaderPrivate	XmrDownloaderPrivate;

struct _XmrDownloader
{
	GObject parent;

	XmrDownloaderPrivate *priv;
};

struct _XmrDownloaderClass
{
	GObjectClass parent;

	void (*download_finish)(XmrDownloader *downloader,
							const gchar *url,
							const gchar *file);

	void (*download_progress)(XmrDownloader *downloader,
							  const gchar *url,
							  double progress);

	void (*download_failed)(XmrDownloader *downloader,
							const gchar *url,
							const gchar *message);
};

GType			xmr_downloader_get_type();
XmrDownloader*	xmr_downloader_new();

void
xmr_downloader_add_task(XmrDownloader *downloader,
						const gchar *url,
						const gchar *file);

/**
 * test whether task exists
 */
gboolean
xmr_downloader_test_task(XmrDownloader *downloader,
		const gchar *url,
		const gchar *file);

G_END_DECLS

#endif /* __XMR_DOWNLOADER_H__ */
