/** 
 * xmrpluginengine.h
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
#ifndef __XMR_PLUGIN_ENGINE_H__
#define __XMR_PLUGIN_ENGINE_H__

#include <libpeas/peas-engine.h>

G_BEGIN_DECLS

typedef struct _XmrPluginEngine			XmrPluginEngine;
typedef struct _XmrPluginEngineClass	XmrPluginEngineClass;
typedef struct _XmrPluginEnginePrivate	XmrPluginEnginePrivate;

#define XMR_TYPE_PLUGIN_ENGINE            xmr_plugin_engine_get_type()
#define XMR_PLUGIN_ENGINE(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), XMR_TYPE_PLUGIN_ENGINE, XmrPluginEngine))
#define XMR_PLUGIN_ENGINE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), XMR_TYPE_PLUGIN_ENGINE, XmrPluginEngineClass))
#define XMR_IS_PLUGIN_ENGINE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), XMR_TYPE_PLUGIN_ENGINE))
#define XMR_IS_PLUGIN_ENGINE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), XMR_TYPE_PLUGIN_ENGINE))
#define XMR_PLUGIN_ENGINE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), XMR_TYPE_PLUGIN_ENGINE, XmrPluginEngineClass))

struct _XmrPluginEngine
{
	PeasEngine parent;
	XmrPluginEnginePrivate *priv;
};

struct _XmrPluginEngineClass
{
	PeasEngineClass parent_class;
};

GType xmr_plugin_engine_get_type();

XmrPluginEngine*
xmr_plugin_engine_new();

G_END_DECLS

#endif /* __XMR_PLUGIN_ENGINE_H__ */
