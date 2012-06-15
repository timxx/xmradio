/** 
 * xmr-tray-icon.h
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

#ifndef __XMR_TRAY_ICON_H__
#define __XMR_TRAY_ICON_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define XMR_TYPE_TRAY_ICON			(xmr_tray_icon_get_type())
#define XMR_TRAY_ICON(o)			(G_TYPE_CHECK_INSTANCE_CAST((o), XMR_TYPE_TRAY_ICON, XmrTrayIcon))
#define XMR_TRAY_ICON_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), XMR_TYPE_TRAY_ICON, XmrTrayIconClass))
#define XMR_IS_TRAY_ICON(o)	        (G_TYPE_CHECK_INSTANCE_TYPE((o), XMR_TYPE_TRAY_ICON))
#define XMR_IS_TRAY_ICON_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE((k), XMR_TYPE_TRAY_ICON))
#define XMR_TRAY_ICON_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS((o),  XMR_TYPE_TRAY_ICON, XmrTrayIconClass))

typedef struct _XmrTrayIcon XmrTrayIcon;
typedef struct _XmrTrayIconClass XmrTrayIconClass;
typedef struct _XmrTrayIconPrivate XmrTrayIconPrivate;

struct _XmrTrayIcon
{
	GtkStatusIcon parent;
	XmrTrayIconPrivate *priv;
};

struct _XmrTrayIconClass
{
	GtkStatusIconClass parent_class;
};

GType xmr_tray_icon_get_type();

GtkWidget*
xmr_tray_icon_new(GtkWidget *main_window,
			GtkWidget *popup_menu);

void xmr_tray_icon_set_tooltips(XmrTrayIcon *tray, const gchar *text);

G_END_DECLS

#endif /* __XMR_TRAY_ICON_H__ */

