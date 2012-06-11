/**
 * xmrplugin.h
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

#ifndef __XMR_PLUGIN_H__
#define __XMR_PLUGIN_H__

#include <libpeas/peas.h>

G_BEGIN_DECLS

enum
{
	PROP_0,
	PROP_OBJECT
};

#define XMR_DEFINE_PLUGIN(TYPE_NAME, TypeName, type_name)	\
	GType type_name##_get_type (void) G_GNUC_CONST;			\
	static void impl_activate(PeasActivatable *plugin);			\
	static void impl_deactivate (PeasActivatable *plugin);			\
	static void peas_activatable_iface_init(PeasActivatableInterface *iface); \
 \
 G_DEFINE_DYNAMIC_TYPE_EXTENDED(TypeName,				\
		 		 type_name,				\
		 		 PEAS_TYPE_EXTENSION_BASE,		\
				 0,					\
				 G_IMPLEMENT_INTERFACE_DYNAMIC(PEAS_TYPE_ACTIVATABLE,\
					 			peas_activatable_iface_init))				\
 \
 static void peas_activatable_iface_init(PeasActivatableInterface *iface) \
 {									\
	iface->activate = impl_activate;				\
	iface->deactivate = impl_deactivate;				\
 }									\
 \
 static void set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) \
 {									\
	switch (prop_id) {						\
	case PROP_OBJECT:						\
		g_object_set_data_full (object,				\
					"window",			\
					g_value_dup_object (value),	\
					g_object_unref);		\
		break;							\
	default:							\
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec); \
		break;							\
	}								\
 }									\
 static void get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) \
 {									\
	switch (prop_id) {						\
	case PROP_OBJECT:						\
		g_value_set_object(value, g_object_get_data(object, "window")); \
		break;							\
	default:							\
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec); \
		break;							\
	}								\
 }									\
 static void type_name##_class_init (TypeName##Class *klass)		\
 {									\
	GObjectClass *object_class = G_OBJECT_CLASS (klass);		\
	object_class->set_property = set_property;			\
	object_class->get_property = get_property;			\
	g_object_class_override_property(object_class, PROP_OBJECT, "object"); \
 }									\
 static void type_name##_class_finalize (TypeName##Class *klass)	\
 {									\
 }

G_END_DECLS

#endif /* __XMR_PLUGIN_H__ */

