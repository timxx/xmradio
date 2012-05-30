#ifndef __XMR_SKIN_H__
#define __XMR_SKIN_H__

#include <glib-object.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

G_BEGIN_DECLS

#define XMR_TYPE_SKIN			(xmr_skin_get_type())
#define XMR_SKIN(o)				(G_TYPE_CHECK_INSTANCE_CAST((o), XMR_TYPE_SKIN, XmrSkin))
#define XMR_SKIN_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), XMR_TYPE_SKIN, XmrSkinClass))
#define XMR_IS_SKIN(o)			(G_TYPE_CHECK_INSTANCE_TYPE((o), XMR_TYPE_SKIN))
#define XMR_IS_SKIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE((k), XMR_TYPE_SKIN))
#define XMR_SKIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS((o), XMR_TYPE_SKIN, XmrSkinClass))

typedef struct _XmrSkin			XmrSkin;
typedef struct _XmrSkinClass	XmrSkinClass;
typedef struct _XmrSkinPrivate	XmrSkinPrivate;

struct _XmrSkin
{
	GObject parent;

	XmrSkinPrivate *priv;
};

struct _XmrSkinClass
{
	GObjectClass parent_class;
};

typedef struct
{
	gchar *name;		// name of the skin
	gchar *version;		// version of the skin
	gchar *author;		// skin maker
	gchar *url;			// url
	gchar *email;		// email
	gchar *file;		// the skin file path

}SkinInfo;

GType		xmr_skin_get_type();
XmrSkin*	xmr_skin_new();

/**
 * call once only
 * @param uri skin file path
 */
gboolean
xmr_skin_load(XmrSkin *skin, const gchar *uri);


SkinInfo*
xmr_skin_info_new();

void
xmr_skin_info_free(SkinInfo *info);

typedef enum
{
	UI_MAIN
}SkinUi;

/**
 * fill skin info to @info
 */
void
xmr_skin_get_info(XmrSkin *skin, SkinInfo *info);

/**
 * get 'position' element value
 */
gboolean
xmr_skin_get_position(XmrSkin *skin,
			SkinUi ui,
			const gchar *name,
			gint *x, gint *y);

/**
 * @param name when @name is NULL,
 * #XmrSkin will return the root node image,
 * otherwise get child which named @name's image
 */
GdkPixbuf *
xmr_skin_get_image(XmrSkin *skin,
			SkinUi ui,
			const gchar *name);

G_END_DECLS

#endif /* __XMR_SKIN_H__ */
