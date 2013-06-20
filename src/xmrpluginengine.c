/** 
 * xmrpluginengine.c
 * This file is part of xmradio
 *
 * Copyright (C) 2012-2013  Weitian Leung (weitianleung@gmail.com)

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

#include "xmrpluginengine.h"
#include "config.h"
#include "xmrutil.h"

G_DEFINE_TYPE(XmrPluginEngine, xmr_plugin_engine, PEAS_TYPE_ENGINE)

struct _XmrPluginEnginePrivate
{
    GSettings *plugins_settings;
};

static void
xmr_plugin_engine_dispose(GObject *object)
{
	XmrPluginEngine *engine = XMR_PLUGIN_ENGINE(object);

	if (engine->priv->plugins_settings != NULL)
	{
		g_object_unref(engine->priv->plugins_settings);
		engine->priv->plugins_settings = NULL;
	}

	G_OBJECT_CLASS(xmr_plugin_engine_parent_class)->dispose (object);
}

static void
xmr_plugin_engine_class_init(XmrPluginEngineClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = xmr_plugin_engine_dispose;

	g_type_class_add_private(klass, sizeof(XmrPluginEnginePrivate));
}

static void
xmr_plugin_engine_init(XmrPluginEngine *engine)
{
	engine->priv = G_TYPE_INSTANCE_GET_PRIVATE(engine,
						    XMR_TYPE_PLUGIN_ENGINE,
						    XmrPluginEnginePrivate);

	engine->priv->plugins_settings = g_settings_new("com.timxx.xmradio.plugins");
}

XmrPluginEngine *
xmr_plugin_engine_new()
{
	XmrPluginEngine *engine;
	gchar *user_plugin_path;
	GError *error = NULL;

	if (g_irepository_require(g_irepository_get_default(),
				   "Peas", "1.0", 0, &error) == NULL)
	{
		g_warning("Error loading Peas typelib: %s\n", error->message);
		g_clear_error(&error);
	}

	if (g_irepository_require(g_irepository_get_default(),
				   "PeasGtk", "1.0", 0, &error) == NULL)
	{
		g_warning("Error loading PeasGtk typelib: %s\n", error->message);
		g_clear_error(&error);
	}

	engine = g_object_new(XMR_TYPE_PLUGIN_ENGINE,
						  NULL);

	peas_engine_enable_loader(PEAS_ENGINE(engine), "python");

	// application binary path plugins
	user_plugin_path = g_build_filename(xmr_app_dir(), "plugins/", NULL);
	peas_engine_add_search_path(PEAS_ENGINE(engine), user_plugin_path, user_plugin_path);
	g_free(user_plugin_path);

	// per-user plugins
	user_plugin_path = g_build_filename(xmr_config_dir(), "plugins/", NULL);
	peas_engine_add_search_path(PEAS_ENGINE(engine), user_plugin_path, user_plugin_path);
	g_free(user_plugin_path);

	// system-wide plugins
	peas_engine_add_search_path(PEAS_ENGINE(engine), PLUGIN_DIR, PLUGIN_DATA_DIR);

	g_settings_bind(engine->priv->plugins_settings,
			 "active-plugins",
			 engine,
			 "loaded-plugins",
			 G_SETTINGS_BIND_DEFAULT);

	return engine;
}
