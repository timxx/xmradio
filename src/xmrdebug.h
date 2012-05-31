/** 
 * xmrdebug.h
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
#ifndef __XMR_DEBUG_H__
#define __XMR_DEBUG_H__

#include <stdarg.h>
#include <glib.h>

G_BEGIN_DECLS

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#	define xmr_debug(...) xmr_debug_real(__func__, __FILE__, __LINE__, TRUE, __VA_ARGS__)
#elif defined(__GNUC__) && __GNUC__ >= 3
#	define xmr_debug(...) xmr_debug_real(__FUNCTION__, __FILE__, __LINE__, TRUE, __VA_ARGS__)
#else
#	define xmr_debug
#endif

void xmr_debug_enable(gboolean enable);

void xmr_debug_real(const gchar *func,
				const gchar *file,
				gint line,
				gboolean newline,
				const gchar *format, ...) G_GNUC_PRINTF (5, 6);

G_END_DECLS

#endif /* __XMR_DEBUG_H__ */
