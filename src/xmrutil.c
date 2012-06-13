/** 
 * xmrutil.c
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
#include "xmrutil.h"
#include "config.h"

static gchar _config_dir[256] = { 0 };
static gchar _cover_dir[256] = { 0 };

GdkPixbuf *
gdk_pixbuf_from_memory(const gchar *buffer, gint len)
{
	GdkPixbuf *pixbuf = NULL;
	GdkPixbufLoader *loader;

	loader = gdk_pixbuf_loader_new();
    if (loader == NULL)
        return NULL;

    if (!gdk_pixbuf_loader_write(loader, (const guchar *)buffer, len, NULL))
		return NULL;

    // forces the data to be parsed by the loader
    gdk_pixbuf_loader_close(loader, NULL);

    pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);
	if (pixbuf != NULL)
		g_object_ref(pixbuf);

	g_object_unref(loader);

	return pixbuf;
}

void
list_file(const gchar *folder,
			gboolean recursive,
			FileOpFunc opfunc,
			gpointer data)
{
	GDir *dir;
    const gchar *name;
    gchar *full_path;

    if (NULL == folder || NULL == opfunc)
        return ;

    dir = g_dir_open(folder, 0, NULL);
    if (NULL == dir)
        return ;

    while((name = g_dir_read_name(dir)))
    {
        full_path = g_build_filename(folder, name, NULL);
        if (g_file_test(full_path, G_FILE_TEST_IS_DIR))
        {
            if (recursive)
                list_file(full_path, TRUE, opfunc, data);
        }
        else
        {
            opfunc(full_path, data);
        }
        g_free(full_path);
    }

    g_dir_close(dir);
}

const gchar *
xmr_config_dir()
{
	if (_config_dir[0] == 0)
	{
		g_snprintf(_config_dir, 256,
					"%s/%s",
					g_get_user_config_dir(),
					PACKAGE);
		g_mkdir_with_parents(_config_dir, 0755);
	}
	
	return _config_dir;
}

const gchar *
xmr_radio_icon_dir()
{
	if (_cover_dir[0] == 0)
	{
		g_snprintf(_cover_dir, 256,
					"%s/%s/radio/icons",
					g_get_user_config_dir(),
					PACKAGE);

		g_mkdir_with_parents(_cover_dir, 0755);
	}
	
	return _cover_dir;
}

void
xmr_message(GtkWidget *parent,
			const gchar *message,
			const gchar *title)
{
	GtkWidget *dialog;

	dialog = gtk_message_dialog_new(GTK_WINDOW(parent),
				GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_INFO,
                GTK_BUTTONS_OK,
                message,
				NULL);

	gtk_window_set_title(GTK_WINDOW(dialog), title);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

gint
write_memory_to_file(const gchar *file,
			const gpointer data,
			gint len)
{
	FILE *fp;
	gint result = -1;

	fp = fopen(file, "wb");
	if (fp == NULL)
		return -1;

	result = fwrite(data, 1, len, fp);

	fclose(fp);

	return result;
}
