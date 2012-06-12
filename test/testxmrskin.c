/** 
 * testxmrskin.c
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
#include "../src/xmrskin.h"
#include "../src/xmrdebug.h"

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
