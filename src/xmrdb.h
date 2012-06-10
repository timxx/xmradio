/** 
 * xmrdb.h
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

#ifndef __XMR_DB_H__
#define __XMR_DB_H__

#include <glib-object.h>

#include "lib/radioinfo.h"

G_BEGIN_DECLS

#define XMR_TYPE_DB				(xmr_db_get_type())
#define XMR_DB(inst)			(G_TYPE_CHECK_INSTANCE_CAST((inst),	XMR_TYPE_DB, XmrDb))
#define XMR_DB_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XMR_TYPE_DB, XmrDbClass))
#define XMR_IS_DB(inst)			(G_TYPE_CHECK_INSTANCE_TYPE((inst), XMR_TYPE_DB))
#define XMR_IS_DB_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XMR_TYPE_DB))
#define XMR_DB_GET_CLASS(inst)	(G_TYPE_INSTANCE_GET_CLASS((inst), XMR_TYPE_DB, XmrDbClass))

typedef struct _XmrDb			XmrDb;
typedef struct _XmrDbClass		XmrDbClass;
typedef struct _XmrDbPrivate	XmrDbPrivate;

struct _XmrDb
{
	GObject parent;
	XmrDbPrivate *priv;
};

struct _XmrDbClass
{
	GObjectClass parent_class;
};

GType xmr_db_get_type();

XmrDb *
xmr_db_new();

void
xmr_db_begin(XmrDb *db);

void
xmr_db_commit(XmrDb *db);

/**
 * insert @radio into database
 */
gint
xmr_db_add_radio(XmrDb *db,
			RadioInfo *radio,
			const gchar *style);

/**
 * get a full list of radio which style is @style
 */
GSList *
xmr_db_get_radio_list(XmrDb *db,
			const gchar *style);

/**
 * test if db is empty set
 */
gboolean
xmr_db_is_empty(XmrDb *db);

G_END_DECLS

#endif /* __XMR_DB_H__ */
