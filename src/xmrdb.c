/** 
 * xmrdb.c
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

#include <sqlite3.h>

#include "xmrdb.h"
#include "config.h"
#include "xmrdebug.h"

G_DEFINE_TYPE(XmrDb, xmr_db, G_TYPE_OBJECT);

struct _XmrDbPrivate
{
	sqlite3 *db;
};

static gint
exe_sql(sqlite3 *db, const gchar *sql);

/**
 * try to create database if target
 * table not exits
 */
static void
init_db(sqlite3 *db);

typedef void (*SQL_QUERY_FUNC)(gpointer, const gchar **, gint, gint);

/**
 * do a query
 * @sql, sql statement
 * @deal_func, func to deal with result
 * @data, data pass to @deal_func
 */
static gint
sql_query(sqlite3 *db,
			const gchar *sql,
			SQL_QUERY_FUNC deal_func,
			gpointer data);


/**
 * @data, GSList pointer pointer
 */
static void
get_radio_list(gpointer data,
			const gchar **result,
			gint row,
			gint col);

static void
query_db_count(gpointer data,
			const gchar **result,
			gint row,
			gint col);
static void
xmr_db_dispose(GObject *obj)
{
	XmrDb *db = XMR_DB(obj);
	XmrDbPrivate *priv = db->priv;

	if (priv->db)
	{
		sqlite3_close(priv->db);
		priv->db = NULL;
	}

	G_OBJECT_CLASS(xmr_db_parent_class)->dispose(obj);
}

static void xmr_db_class_init(XmrDbClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->dispose = xmr_db_dispose;

	g_type_class_add_private(G_OBJECT_CLASS(klass), sizeof(XmrDbPrivate));
}

static void xmr_db_init(XmrDb *db)
{
	XmrDbPrivate *priv;
	GString *db_file = NULL;

	priv = G_TYPE_INSTANCE_GET_PRIVATE(db, XMR_TYPE_DB, XmrDbPrivate);
	db->priv = priv;
	priv->db = NULL;

	db_file = g_string_new("");
	if (db_file == NULL)
		g_error("No more memory ??\n");

	g_string_printf(db_file, "%s/%s/radio",
				g_get_user_config_dir(),
				PACKAGE);

	// ensure dir exists
	g_mkdir_with_parents(db_file->str, 0755);

	db_file = g_string_append(db_file, "/xmradio.db");

	if (sqlite3_open(db_file->str, &priv->db) == SQLITE_OK){
		init_db(priv->db);
	}else{
		g_warning("Couldn't open database\n");
	}

	g_string_free(db_file, TRUE);
}

static gint
exe_sql(sqlite3 *db, const gchar *sql)
{
	gint result;
	gchar *msg = NULL;

	result = sqlite3_exec(db, sql, NULL, NULL, &msg);

	if (result != SQLITE_OK)
	{
		xmr_debug("Error: %s", msg);
		sqlite3_free(msg);

		return result;
	}

	return SQLITE_OK;
}

static void
init_db(sqlite3 *db)
{
	gchar *sql;

	sql =  "CREATE TABLE IF NOT EXISTS radio_list("
		"rid INTEGER PRIMARY KEY,"
		"id TEXT, "
		"name TEXT, "
		"logo TEXT, "
		"url TEXT, "
		"style TEXT"
		")";

	exe_sql(db, sql);
}

static gint
sql_query(sqlite3 *db,
			const gchar *sql,
			SQL_QUERY_FUNC deal_func,
			gpointer data)
{
	gchar **result = NULL;
	gint row = 0, col = 0;

	gint ret = -1;
	gchar *msg = NULL;

	if (deal_func == NULL)
		return -1;

	do
	{
		ret = sqlite3_get_table(db, sql, &result, &row, &col, &msg);
		if (ret != SQLITE_OK)
		{
			xmr_debug("sql query failed with code: %d\nmessage: %s\n", ret, msg);
			break;
		}

		deal_func(data, (const gchar **)result, row, col);
	}
	while(0);

	if (msg) {
		sqlite3_free(msg);
	}

	if (result){
		sqlite3_free_table(result);
	}

	return ret;
}

static void
get_radio_list(gpointer data,
			const gchar **result,
			gint row,
			gint col)
{
	GSList **list = (GSList **)data;
	gint index = col;
	gint i;

	if (result == NULL || row == 0 || col != 4)
		return ;

	for(i=0; i<row; ++i)
	{
		const gchar *id, *name, *logo, *url;
		RadioInfo *radio;

		radio = radio_info_new();
		if (radio == NULL){
			g_error("No more memory ???\n");
		}

		id		= result[index++];
		name	= result[index++];
		logo	= result[index++];
		url		= result[index++];

		radio->id	= g_strdup(id);
		radio->name = g_strdup(name);
		radio->logo = g_strdup(logo);
		radio->url	= g_strdup(url);

		*list = g_slist_append(*list, radio);
	}
}

static void
query_db_count(gpointer data,
			const gchar **result,
			gint row,
			gint col)
{
	gint *count = (gint *)data;

	if (result == NULL)
		return ;

	*count = g_strtod(result[col], NULL);
}

XmrDb *
xmr_db_new()
{
	return g_object_new(XMR_TYPE_DB,
				NULL);
}

void
xmr_db_begin(XmrDb *db)
{
	g_return_if_fail(db != NULL);

	exe_sql(db->priv->db, "BEGIN;");
}

void
xmr_db_commit(XmrDb *db)
{
	g_return_if_fail(db != NULL);

	exe_sql(db->priv->db, "COMMIT;");
}

gint
xmr_db_add_radio(XmrDb *db,
			RadioInfo *radio,
			const gchar *style)
{
	gchar *sql;
	gint result = -1;

	g_return_val_if_fail(db != NULL && radio != NULL, -1);

	sql = g_strdup_printf("INSERT INTO radio_list(id, name, logo, url, style) "
				"VALUES('%s', '%s', '%s', '%s', '%s')",
				radio->id, radio->name, radio->logo, radio->url, style);

	if (sql == NULL){
		g_error("No more memory ???\n");
	}

	result = exe_sql(db->priv->db, sql);

	g_free(sql);

	return result;
}

GSList *
xmr_db_get_radio_list(XmrDb *db,
			const gchar *style)
{
	GSList *list = NULL;
	gchar *sql = NULL;

	g_return_val_if_fail(db != NULL && style != NULL, NULL);

	sql = g_strdup_printf("SELECT id, name, logo, url FROM radio_list "
				"WHERE style='%s'", style);

	if (sql == NULL){
		g_error("No more memory ???\n");
	}

	sql_query(db->priv->db, sql, get_radio_list, &list);

	g_free(sql);

	return list;
}

gboolean
xmr_db_is_empty(XmrDb *db)
{
	gint count = 0;

	g_return_val_if_fail(db != NULL, TRUE);

	sql_query(db->priv->db,
				"SELECT COUNT(*) FROM radio_list",
				query_db_count, &count);

	return count == 0;
}
