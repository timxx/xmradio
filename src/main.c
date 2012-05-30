#include <glib/gi18n.h>
#include <locale.h>
#include <glib.h>
#include <curl/curl.h>

#include "xmrapp.h"
#include "config.h"
#include "xmrdebug.h"

int main(int argc, char **argv)
{
	XmrApp *app;

#if !GLIB_CHECK_VERSION(2, 32, 0)
	g_thread_init(NULL);
#endif

	g_type_init();
	xmr_debug_enable(TRUE);

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
