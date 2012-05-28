#ifndef __XMR_BUTTON_H__
#define __XMR_BUTTON_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define XMR_TYPE_BUTTON			(xmr_button_get_type())
#define XMR_BUTTON(o)			(G_TYPE_CHECK_INSTANCE_CAST((o), XMR_TYPE_BUTTON, XmrButton))
#define XMR_BUTTON_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), XMR_TYPE_BUTTON, XmrButtonClass))
#define XMR_IS_BUTTON(o)		(G_TYPE_CHECK_INSTANCE_TYPE((o), XMR_TYPE_BUTTON))
#define XMR_IS_BUTTON_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE((k), XMR_TYPE_BUTTON))
#define XMR_BUTTON_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS((o), XMR_TYPE_BUTTON, XmrButtonClass))

typedef struct _XmrButton			XmrButton;
typedef struct _XmrButtonClass		XmrButtonClass;
typedef struct _XmrButtonPrivate	XmrButtonPrivate;

struct _XmrButton
{
	GtkButton parent;
	XmrButtonPrivate *priv;
};

struct _XmrButtonClass
{
	GtkButtonClass parent_class;
};

typedef enum
{
	XMR_BUTTON_NORMAL,
	XMR_BUTTON_SKIN
}
XmrButtonType;

typedef enum
{
	STATE_NORMAL = 0,
	STATE_FOCUS,
	STATE_PUSH,
	STATE_DISABLE,
	LAST_STATE
}
XmrButtonState;

GType		xmr_button_get_type();
GtkWidget*	xmr_button_new(XmrButtonType type);

XmrButtonType
xmr_button_set_type(XmrButton *button, XmrButtonType type);

XmrButtonState
xmr_button_set_state(XmrButton *button, XmrButtonState state);

void
xmr_button_set_image_from_uri(XmrButton *button, const gchar *uri);

void
xmr_button_set_image_from_pixbuf(XmrButton *button, GdkPixbuf *pixbuf);

void
xmr_button_set_image_from_stock(XmrButton *button, const gchar *stock);

G_END_DECLS

#endif /* __XMR_BUTTON_H__ */
