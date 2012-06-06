/** 
 * xmrsettins.h
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
#ifndef __XMR_SETTINGS_H__
#define __XMR_SETTINGS_H__

#include <gio/gio.h>

G_BEGIN_DECLS

#define XMR_TYPE_SETTINGS				(xmr_settings_get_type())
#define XMR_SETTINGS(inst)				(G_TYPE_CHECK_INSTANCE_CAST((inst),	XMR_TYPE_SETTINGS, XmrSettings))
#define XMR_SETTINGS_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XMR_TYPE_SETTINGS, XmrSettingsClass))
#define XMR_IS_SETTINGS(inst)			(G_TYPE_CHECK_INSTANCE_TYPE((inst), XMR_TYPE_SETTINGS))
#define XMR_IS_SETTINGS_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XMR_TYPE_SETTINGS))
#define XMR_SETTINGS_GET_CLASS(inst)	(G_TYPE_INSTANCE_GET_CLASS((inst), XMR_TYPE_SETTINGS, XmrSettingsClass))

typedef struct _XmrSettings			XmrSettings;
typedef struct _XmrSettingsClass	XmrSettingsClass;

struct _XmrSettings
{
	GSettings parent;
};

struct _XmrSettingsClass
{
	GSettingsClass parent_class;
};

XmrSettings*
xmr_settings_new();

gboolean
xmr_settings_get_window_pos(XmrSettings *sett,
			gint *x, gint *y);

gboolean
xmr_settings_set_window_pos(XmrSettings *sett,
			gint x, gint y);

gchar*
xmr_settings_get_theme(XmrSettings *sett);

gboolean
xmr_settings_set_theme(XmrSettings *sett,
			const gchar *theme);

void
xmr_settings_get_radio(XmrSettings *sett,
			gchar **name,
			gchar **url);

gboolean
xmr_settings_set_radio(XmrSettings *sett,
			const gchar *name,
			const gchar *url);

gboolean
xmr_settings_get_auto_login(XmrSettings *sett);

gboolean
xmr_settings_set_auto_login(XmrSettings *sett,
			gboolean auto_login);

void
xmr_settings_get_usr_info(XmrSettings *sett,
			gchar **usr,
			gchar **pwd);

gboolean
xmr_settings_set_usr_info(XmrSettings *sett,
			const gchar *usr,
			const gchar *pwd);

G_END_DECLS

#endif /* __XMR_SETTINGS_H__ */
