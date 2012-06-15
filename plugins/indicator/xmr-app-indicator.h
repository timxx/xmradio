/** 
 * xmr-app-indicator.h
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
#ifndef __XMR_APP_INDICATOR_H__
#define __XMR_APP_INDICATOR_H__

#include <libappindicator/app-indicator.h>

G_BEGIN_DECLS


#define XMR_TYPE_APP_INDICATOR			(xmr_app_indicator_get_type())
#define XMR_APP_INDICATOR(o)			(G_TYPE_CHECK_INSTANCE_CAST((o), XMR_TYPE_APP_INDICATOR, XmrAppIndicator))
#define XMR_APP_INDICATOR_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), XMR_TYPE_APP_INDICATOR, XmrAppIndicatorClass))
#define XMR_IS_APP_INDICATOR(o)	        (G_TYPE_CHECK_INSTANCE_TYPE((o), XMR_TYPE_APP_INDICATOR))
#define XMR_IS_APP_INDICATOR_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE((k), XMR_TYPE_APP_INDICATOR))
#define XMR_APP_INDICATOR_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS((o),  XMR_TYPE_APP_INDICATOR, XmrAppIndicatorClass))

typedef struct _XmrAppIndicator XmrAppIndicator;
typedef struct _XmrAppIndicatorClass XmrAppIndicatorClass;
typedef struct _XmrAppIndicatorPrivate XmrAppIndicatorPrivate;

struct _XmrAppIndicator
{
	AppIndicator parent;
	XmrAppIndicatorPrivate *priv;
};

struct _XmrAppIndicatorClass
{
	AppIndicatorClass parent_class;
};

XmrAppIndicator *
xmr_app_indicator_new(GtkWidget *popup_menu);

G_END_DECLS

#endif /* __XMR_APP_INDICATOR_H__ */
