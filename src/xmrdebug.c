/** 
 * xmrdebug.c
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
#include <time.h>

#include "xmrdebug.h"

static gboolean enable_debug = FALSE;

static void
xmr_debug_print(
			const gchar *func,
			const gchar *file,
			gint		 line,
			gboolean	 newline,
			const gchar *buffer
			)
{
	gchar str_time[20];
	time_t cur_time;

	time(&cur_time);
	strftime(str_time, 19, "%H:%M:%S", localtime(&cur_time));

	g_printerr(newline ? "(%s) [%p] [%s] %s:%d: %s\n" : "(%s) [%p] [%s] %s:%d: %s",
		    str_time, g_thread_self(), func, file, line, buffer);
}

void xmr_debug_enable(gboolean enable)
{
	enable_debug = enable;
}

void
xmr_debug_real(
			const gchar *func,
			const gchar *file,
			gint		 line,
			gboolean	 newline,
			const gchar *format,
			...
			)
{
	va_list args;
	gchar buffer[1025];

	if (!enable_debug)
		return;

	va_start(args, format);

	g_vsnprintf(buffer, 1024, format, args);

	va_end(args);

	xmr_debug_print(func, g_path_get_basename(file), line, newline, buffer);
}
