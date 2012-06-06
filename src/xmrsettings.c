/** 
 * xmrsettings.c
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
#include "xmrsettings.h"

G_DEFINE_TYPE(XmrSettings, xmr_settings, G_TYPE_SETTINGS);

static void xmr_settings_init(XmrSettings *sett)
{

}

static void xmr_settings_class_init(XmrSettingsClass *klass)
{
}

XmrSettings*
xmr_settings_new()
{
    return XMR_SETTINGS(g_object_new(XMR_TYPE_SETTINGS,
#if GLIB_CHECK_VERSION(2, 32, 0)
					"schema-id", "com.timxx.xmradio",
#else
					"schema", "com.timxx.xmradio",
#endif
					NULL));
}

gboolean 
xmr_settings_get_window_pos(XmrSettings *sett,
			gint *x, gint *y)
{
	g_return_val_if_fail( sett != NULL, FALSE);

	g_settings_set(G_SETTINGS(sett), "window-pos", "(ii)", x, y);

	return TRUE;
}

gboolean
xmr_settings_set_window_pos(XmrSettings *sett,
			gint x, gint y)
{
	g_return_val_if_fail( sett != NULL, FALSE);

	return g_settings_set(G_SETTINGS(sett), "window-pos", "(ii)", x, y);
}

gchar*
xmr_settings_get_theme(XmrSettings *sett)
{
	g_return_val_if_fail(sett != NULL, NULL);

	return g_settings_get_string(G_SETTINGS(sett), "theme");
}

gboolean
xmr_settings_set_theme(XmrSettings *sett,
			const gchar *theme)
{
	g_return_val_if_fail(sett != NULL, FALSE);

	return g_settings_set_string(G_SETTINGS(sett), "theme", theme);
}

void
xmr_settings_get_radio(XmrSettings *sett,
			gchar **name,
			gchar **url)
{
	g_return_if_fail(sett != NULL);

	*name = g_settings_get_string(G_SETTINGS(sett), "radio-name");
	*url = g_settings_get_string(G_SETTINGS(sett), "radio-url");
}

gboolean
xmr_settings_set_radio(XmrSettings *sett,
			const gchar *name,
			const gchar *url)
{
	g_return_val_if_fail(sett != NULL, FALSE);

	if (g_settings_set_string(G_SETTINGS(sett), "radio-name", name) != TRUE)
		return FALSE;

	return g_settings_set_string(G_SETTINGS(sett), "radio-url", url);
}

gboolean
xmr_settings_get_auto_login(XmrSettings *sett)
{
	g_return_val_if_fail(sett != NULL, FALSE);

	return g_settings_get_boolean(G_SETTINGS(sett), "auto-login");
}

gboolean
xmr_settings_set_auto_login(XmrSettings *sett,
			gboolean auto_login)
{
	g_return_val_if_fail(sett != NULL, FALSE);

	return g_settings_set_boolean(G_SETTINGS(sett), "auto-login", auto_login);
}

void
xmr_settings_get_usr_info(XmrSettings *sett,
			gchar **usr,
			gchar **pwd)
{
	g_return_if_fail(sett != NULL);

	*usr = g_settings_get_string(G_SETTINGS(sett), "usr");
	*pwd = g_settings_get_string(G_SETTINGS(sett), "pwd");
}

gboolean
xmr_settings_set_usr_info(XmrSettings *sett,
			const gchar *usr,
			const gchar *pwd)
{
	g_return_val_if_fail(sett != NULL, FALSE);

	if (g_settings_set_string(G_SETTINGS(sett), "usr", usr) != TRUE)
		return FALSE;

	return g_settings_set_string(G_SETTINGS(sett), "pwd", pwd);
}
