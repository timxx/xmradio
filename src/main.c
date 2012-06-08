/** 
 * main.c
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

#include <glib/gi18n.h>
#include <locale.h>
#include <glib.h>
#include <curl/curl.h>
#include <gst/gst.h>

#include "xmrapp.h"
#include "config.h"
#include "xmrdebug.h"

int main(int argc, char **argv)
{
	XmrApp *app;

#if !GLIB_CHECK_VERSION(2, 32, 0)
	g_thread_init(NULL);
#endif

	gdk_threads_init();

	g_type_init();
	gst_init(&argc, &argv);

//	xmr_debug_enable(TRUE);

	setlocale(LC_ALL, NULL);

#ifdef ENABLE_NLS
	/* initialize i18n */
	bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");

	textdomain(GETTEXT_PACKAGE);
#endif

	gdk_threads_init();
	curl_global_init(CURL_GLOBAL_ALL);

	app = xmr_app_new(argc, argv);

	g_application_run(G_APPLICATION(app), argc, argv);

	g_object_unref(app);

	curl_global_cleanup();

	return 0;
}
