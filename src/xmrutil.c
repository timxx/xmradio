/** 
 * xmrutil.c
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
#include "xmrutil.h"

GdkPixbuf *
gdk_pixbuf_from_memory(const gchar *buffer, gint len)
{
	GdkPixbuf *pixbuf = NULL;
	GdkPixbufLoader *loader;

	loader = gdk_pixbuf_loader_new();
    if (loader == NULL)
        return NULL;

    if (!gdk_pixbuf_loader_write(loader, buffer, len, NULL))
		return NULL;

    // forces the data to be parsed by the loader
    gdk_pixbuf_loader_close(loader, NULL);

    pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);

	return pixbuf;
}
