/** 
 * xmrutil.h
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

#ifndef __XMR_UTIL_H__
#define __XMR_UTIL_H__

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

/**
 * create #GdkPixbuf from memory
 */
GdkPixbuf *
gdk_pixbuf_from_memory(const gchar *buffer, gint len);

typedef void (*FileOpFunc)(const gchar *, gpointer);

/**
 * list @folder files
 * @recursive, recursive dirs
 * @opfunc, func to deal with the file
 * @data, pass to @opfunc
 */
void
list_file(const gchar *folder,
			gboolean recursive,
			FileOpFunc opfunc,
			gpointer data);

/**
 * return package config dir
 */
const gchar *
xmr_config_dir();

void
xmr_message(GtkWidget *parent,
			const gchar *message,
			const gchar *title);

G_END_DECLS

#endif /* __XMR_UTIL_H__ */
