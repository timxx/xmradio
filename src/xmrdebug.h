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
