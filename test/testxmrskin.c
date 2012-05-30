
#include "../src/xmrskin.h"

static void
print_skin_info(SkinInfo *info);

int main(int argc, char **argv)
{
	XmrSkin *skin;
	SkinInfo *info;
	GdkPixbuf *pixbuf;
	gint x, y;

	g_type_init();
	xmr_debug_enable(TRUE);

	skin = xmr_skin_new();

	if (!xmr_skin_load(skin, "../data/skin/pure.skn"))
	{
		g_print("xmr_skin_load failed\n");
		g_object_unref(skin);
		return 1;
	}

	info = xmr_skin_info_new();

	xmr_skin_get_info(skin, info);
	print_skin_info(info);
	xmr_skin_info_free(info);

	if (!xmr_skin_get_position(skin, UI_MAIN, "next", &x, &y))
		g_print("get next position failed\n");
	else
		g_print("next (%d, %d)\n", x, y);

	pixbuf = xmr_skin_get_image(skin, UI_MAIN, NULL);
	if (pixbuf == NULL)
	{
		g_print("xmr_skin_get_image failed\n");
	}
	else
	{
		gint w, h;
		w = gdk_pixbuf_get_width(pixbuf);
		h = gdk_pixbuf_get_height(pixbuf);

		g_print("image size(%d, %d)\n", w, h);

		g_object_unref(pixbuf);
	}

	pixbuf = xmr_skin_get_image(skin, UI_MAIN, "play");
	if (pixbuf == NULL)
	{
		g_print("xmr_skin_get_image failed\n");
	}
	else
	{
		gint w, h;
		w = gdk_pixbuf_get_width(pixbuf);
		h = gdk_pixbuf_get_height(pixbuf);

		g_print("image size(%d, %d)\n", w, h);

		g_object_unref(pixbuf);
	}

	g_object_unref(skin);

	return 0;
}

static void
print_skin_info(SkinInfo *info)
{
	g_print("Name: %s\n"
			"Version: %s\n"
			"Author: %s\n"
			"Email: %s\n"
			"Url: %s\n"
			"\n",
			info->name,
			info->version,
			info->author,
			info->email,
			info->url
		   );
}
