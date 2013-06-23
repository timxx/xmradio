/**
 * xmrwindow.c
 * This file is part of xmradio
 *
 * Copyright (C) 2012 - 2013  Weitian Leung (weitianleung@gmail.com)

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
#include <glib/gi18n.h>
#include <stdlib.h>
#include <libpeas-gtk/peas-gtk-plugin-manager.h>
#include <libpeas/peas-extension-set.h>
#include <libpeas/peas-activatable.h>

#include "xmrwindow.h"
#include "xmrplayer.h"
#include "xmrbutton.h"
#include "xmrskin.h"
#include "xmrdebug.h"
#include "xmrplayer.h"
#include "lib/xmrservice.h"
#include "config.h"
#include "xmrutil.h"
#include "xmrsettings.h"
#include "xmrchooser.h"
#include "xmrdb.h"
#include "xmrmarshal.h"
#include "xmrapp.h"
#include "xmrpluginengine.h"
#include "xmrvolumebutton.h"
#include "xmrlabel.h"
#include "xmrdownloader.h"
#include "xmrapp.h"
#include "xmrwaitingwnd.h"
#include "icon_enter_xpm.h"
#include "xmrlist.h"
#include "xmrsearchbox.h"
#include "xmrradio.h"
#include "xmrartist.h"

G_DEFINE_TYPE(XmrWindow, xmr_window, GTK_TYPE_WINDOW)

#define DEFAULT_RADIO_URL	"http://www.xiami.com/kuang/xml/type/6/id/0"
#define DEFAULT_RADIO_NAME	_("新歌电台")

#define WAITING_INFO_LOGIN		_("Login...")
#define WAITING_INFO_BUFFERING	_("Buffering...")
#define WAITING_INFO_PLAYLIST	_("Getting playlist...")

#define XMR_EVENT_INTERVAL	50

#define COVER_WIDTH	100
#define COVER_HEIGHT 100

enum
{
	BUTTON_CLOSE = 0,	// X button
	BUTTON_MINIMIZE,	// - button
	BUTTON_PLAY,		// play
	BUTTON_PAUSE,
	BUTTON_NEXT,
	BUTTON_LIKE,
	BUTTON_DISLIKE,
	BUTTON_VOLUME,
	BUTTON_LYRIC,
	BUTTON_DOWNLOAD,
	BUTTON_SHARE,
	BUTTON_SIREN,
	BUTTON_FENGGE,
	BUTTON_XINGZUO,
	BUTTON_NIANDAI,
	LAST_BUTTON
};

enum
{
	LABEL_RADIO = 0,
	LABEL_SONG_NAME,
	LABEL_ARTIST,
	LABEL_TIME,
	LABEL_ALBUM,
	LAST_LABEL
};

enum
{
	XMR_EVENT_LOGIN_FINISH,
	XMR_EVENT_LOGOUT,
	XMR_EVENT_PLAYLIST,
	XMR_EVENT_COVER,
	XMR_EVENT_APPEND_RADIO,
	XMR_EVENT_PLAYER_EOS,
	XMR_EVENT_PLAYER_STATE_CHANGED,
	XMR_EVENT_PLAYER_TICK,
	XMR_EVENT_PLAYER_BUFFERING
};

typedef struct
{
	gchar *message;
	gchar *title;
}Message;

typedef struct
{
	guint type;
	gpointer data;
}XmrEvent;

typedef struct
{
	gchar *uri;
	gint idx;
	RadioInfo *info;
}AppendRadio;

typedef struct
{
	XmrWindow *window;
	gint result;
	GList *list;
}FetchPlaylist;

typedef struct
{
	XmrWindow *window;
	gboolean ok;
	gchar *message;
}LoginFinish;

typedef enum
{
	Radio_Type_MyRadio,
	Radio_Type_XMGuess,
	Radio_Type_Custom
}RadioType;

struct _XmrWindowPrivate
{
	gchar		*skin;
	gboolean	gtk_theme;	/* TRUE to use gtk theme rather than skin */

	GtkWidget	*buttons[LAST_BUTTON];	/* ui buttons */
	GtkWidget	*labels[LAST_LABEL];
	GtkWidget	*image;	/* album cover image */
	GtkWidget	*search_box;

	cairo_surface_t	*cs_bkgnd;	/* backgroud image surface */

	GtkWidget	*fixed;			/* #GtkFixed */
	GtkWidget	*popup_menu;	/* #GtkMenu */
	GtkWidget	*skin_menu;

	GtkWidget	*menu_playlist;
	GtkWidget	*menu_login;
	GtkWidget	*menu_logout;
	
	GtkWidget	*menu_item_ontop;

	/**
	 * #XmrChooser
	 * 0 - FengGe
	 * 1 - XingZuo
	 * 2 - NianDai
	 */
	GtkWidget	*chooser[3];

	XmrPlayer *player;
	XmrService *service; /* should only use for private radio */

	GList *playlist;
	gchar *playlist_url;

	RadioType radio_type;

	SongInfo *current_song;

	XmrSettings *settings;
	GList *skin_list;

	GSList *skin_item_group; /* skin menu item list */

	GdkPixbuf *pb_cover;	/* default album cover image pixbuf */
	GdkPixbuf *pb_cover_current; /* current album cover image */

	gint cover_request_width;
	gint cover_request_height;

	GtkBuilder *ui_pref;
	GtkBuilder *ui_login;

	gchar *usr;
	gchar *pwd;

	/**
	 * flag to state when login success whether 
	 * switch to siren radio or not
	 */
	gboolean switch_radio;

	XmrPluginEngine		*plugin_engine;
	PeasExtensionSet	*extensions;

	GMutex*	mutex;

	GtkWidget *dialog_login;

	gboolean syncing_volume;

	Message message;

	XmrDownloader *downloader;

	GAsyncQueue *queue_fetch_playlist;
	GAsyncQueue *queue_fetch_cover;
	GAsyncQueue *queue_event;

	guint xmr_event_timer;
	guint buffering_timer;	/* show/hide buffering window */
	
	GtkWidget *waiting_wnd;

	GtkWidget *xmr_searchlist;
	
	GtkWidget *radio_search_box; /* for search artist radio */
	
	gboolean use_gst_buffering;
	
	gint search_music_count;
	guint search_music_idle_id;
};
/* end of struct _XmrWindowPrivate */

enum
{
	PROP_0,
	PROP_PLAYER,
	PROP_SERVICE,
	PROP_SKIN,
	PROP_GTK_THEME,
	PROP_MENU_POPUP,
	PROP_LAYOUT,
	PROP_PLAYLIST
};

enum
{
	THEME_CHANGED,
	TRACK_CHANGED,
	RADIO_CHANGED,
	LOGIN_FINISH,
	LOGOUT,
	FETCH_PLAYLIST_FINISH,
	FETCH_COVER_FINISH,
	SEARCH_MUSIC,
	PLAYLIST_CHANGED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static const gchar *radio_style[]=
{
	"风格电台", "星座电台", "年代电台"
};

static const gchar *radio_names[] =
{
	N_("私人电台"), N_("虾米猜"), N_("风格电台"), N_("星座电台"), N_("年代电台"),
	N_("艺人电台")
};

#define RADIO_COUNT 6

//=======================================================================

static void
xmr_window_dispose(GObject *obj);

static void
xmr_window_finalize(GObject *obj);

static void
xmr_window_get_property(GObject *object,
		       guint prop_id,
		       GValue *value,
		       GParamSpec *pspec);

static void
xmr_window_set_property(GObject *object,
		       guint prop_id,
		       const GValue *value,
		       GParamSpec *pspec);

static gboolean
on_draw(XmrWindow *window, cairo_t *cr, gpointer data);

static gboolean
on_button_press(XmrWindow *window, GdkEventButton *event, gpointer data);

static void
on_xmr_button_clicked(GtkWidget *widget, gpointer data);

static void
radio_selected(XmrChooser *chooser,
			XmrRadio *radio,
			XmrWindow *window);

static void
set_window_image(XmrWindow *window, GdkPixbuf *pixbuf);

static void
set_cover_image(XmrWindow *window, GdkPixbuf *pixbuf);

static void
set_gtk_theme(XmrWindow *window);

static void
set_skin(XmrWindow *window, SkinInfo *info);

static void
hide_children(XmrWindow *window);

/**
 * player signals
 */
static void
player_eos(XmrPlayer *player,
			gboolean early,
			XmrWindow *window);

static void
player_error(XmrPlayer *player,
			GError *error,
			XmrWindow *window);

static void
player_tick(XmrPlayer *player,
			gint64 elapsed,
			gint64 duration,
			XmrWindow *window);

static void
player_buffering(XmrPlayer *player,
			guint progress,
			XmrWindow *window);

static void
player_state_changed(XmrPlayer *player,
			gint old_state,
			gint new_state,
			XmrWindow *window);

static void
player_volume_changed(XmrPlayer *player,
			float volume,
			XmrWindow *window);

/**
 * threads
 */

static gpointer
thread_get_playlist(XmrWindow *window);

static gpointer
thread_get_cover_image(XmrWindow *window);

static gpointer
thread_login(XmrWindow *window);

static gpointer
thread_logout(XmrWindow *window);

static gpointer
thread_update_radio_list(XmrWindow *window);

static gpointer
thread_like_song(XmrWindow *window);

static gpointer
thread_dislike_song(XmrWindow *window);

static void
xmr_window_get_playlist(XmrWindow *window);

static void
xmr_window_get_cover_image(XmrWindow *window);

static void
xmr_window_set_track_info(XmrWindow *window);

static void
update_radio_list(XmrWindow *window);

static void
create_popup_menu(XmrWindow *window);

/**
 * popup menu message
 */
static void
on_menu_item_activate(GtkMenuItem *item, XmrWindow *window);

/**
 * for skin menu item only
 */
static void
on_skin_menu_item_activate(GtkMenuItem *item, SkinInfo *skin);

/**
 * Radio Menu
 */
static void
on_radio_menu_item_activate(GtkMenuItem *item, XmrWindow *window);

// static void
// on_playlist_menu_item_activate(GtkMenuItem *item, XmrWindow *window);

static void
load_settings(XmrWindow *window);

static void
load_skin(XmrWindow *window);

static void
load_radio(XmrWindow *window);

/**
 * read @skin information
 * if OK, then append to skin_list
 * @data, skin_list pointer pointer
 */
static void
append_skin(const gchar *skin,
			gpointer data);

static GtkWidget *
append_skin_to_menu(XmrWindow *window, SkinInfo *info);

static void
append_skin_to_pref(XmrWindow *window, SkinInfo *info);

static GtkBuilder *
create_builder_with_file(const gchar *path,
						 const gchar *filename);

static void
init_pref_window(XmrWindow *window,
			GtkWidget *pref);

static void
set_login_dialog_entry_editable(XmrWindow *window,
			const gchar *entry_name,
			gboolean editable);

static void
init_login_dialog(XmrWindow *window);

/**
 * login to server
 */
static void
do_login(XmrWindow *window);

/*
static void
do_logout(XmrWindow *window);
*/


/**
 * set @url to NULL to change to siren radio
 */
static void
change_radio(XmrWindow *window,
			const gchar *name,
			const gchar *url);

/**
 * login dialog >login button clicked
 */
static gboolean
on_login_dialog_button_login_clicked(GtkButton *button,
			GdkEvent  *event,
			XmrWindow *window);

/**
 * test if @text is empty
 * or just contains blank chars
 */
static gboolean
is_text_empty(const gchar *text);

static gchar *
radio_logo_url_to_uri(RadioInfo *radio);

static gboolean
on_delete_event(XmrWindow *window,
			GdkEvent *event,
			gpointer data);

static void
on_extension_added(PeasExtensionSet *set,
		    PeasPluginInfo   *info,
		    PeasActivatable  *activatable,
		    gpointer data);

static void
on_combo_box_changed(GtkComboBox *widget, XmrWindow *window);

static void
on_extension_removed(PeasExtensionSet *set,
		      PeasPluginInfo   *info,
			  PeasActivatable  *activatable,
			  gpointer data);

static void
like_current_song(XmrWindow *window, gboolean like);


/**
 * @new_style:
 * 0 - SiRen
 * 1 - FengGe
 * 2 - XingZuo
 * 3 - NianDai
 */
static void
change_radio_style(XmrWindow *window,
			gint new_style);

static void
on_volume_button_value_changed(GtkScaleButton *button,
			gdouble value,
			XmrWindow *window);

static gboolean
show_message_idle(XmrWindow *window);

static void
login_finish(XmrWindow *window,
			 gboolean ok,
			 const gchar *message,
			 gpointer data);

static void
on_logout(XmrWindow *window);

static void
fetch_playlist_finish(XmrWindow *window,
					  gint status,
					  GList *list,
					  gpointer data);

static void
fetch_cover_finish(XmrWindow *window,
				   GdkPixbuf *pixbuf,
				   gpointer data);

// downloader signals
static void
download_finish(XmrDownloader *downloader,
				const gchar *url,
				const gchar *file,
				XmrWindow *window);

static void
download_progress(XmrDownloader *downloader,
				  const gchar *url,
				  double progress,
				  XmrWindow *window);

static void
download_failed(XmrDownloader *downloader,
				const gchar *url,
				const gchar *message,
				XmrWindow *window);

static gboolean
is_track_downloaded(SongInfo *track);

static gboolean
is_file_exists(const gchar *file);

// use @track info gen a local file name
static gchar *
make_track_file(SongInfo *track);

static void
xmr_event_send(XmrWindow *window, guint type, gpointer data);

static gboolean
xmr_event_poll(XmrWindow *window);

/**
 * Func to check downloading status
 */
static gboolean
buffering_timeout(XmrWindow *window);

static void
start_buffering_timer(XmrWindow *window);

static void
stop_buffering_timer(XmrWindow *window);

static void
on_search_box_activate(GtkEntry  *entry,
					   XmrWindow *window);

static gboolean
on_search_box_focus_in(GtkWidget *widget,
					   GdkEvent  *event,
					   gpointer   data);

static gboolean
on_search_box_focus_out(GtkWidget *widget,
					   GdkEvent  *event,
					   gpointer   data);

static gint
get_search_box_font_width(GtkWidget *search_box);

static void
update_playlist_menu_items(XmrWindow *window);

static void
on_playlist_changed(XmrWindow *window, gpointer data);

static void
on_settings_changed(GSettings *settings,
					const char *key,
					XmrWindow *window);

static gboolean
search_music_progress_idle(GtkEntry *entry);

static void
search_music_progress_done(GtkEntry *entry);
//=========================================================================
static void
install_properties(GObjectClass *object_class)
{
	g_object_class_install_property(object_class,
				PROP_PLAYER,
				g_param_spec_object("player",
					"Player",
					"Player object",
					G_TYPE_OBJECT,
					G_PARAM_READABLE));

	g_object_class_install_property(object_class,
				PROP_SERVICE,
				g_param_spec_object("service",
					"Service",
					"XmrService object",
					G_TYPE_OBJECT,
					G_PARAM_READABLE));

	g_object_class_install_property(object_class,
				PROP_SKIN,
				g_param_spec_string("skin",
					"Skin",
					"Skin",
					NULL,
					G_PARAM_READWRITE));

	g_object_class_install_property(object_class,
				PROP_GTK_THEME,
				g_param_spec_boolean("gtk-theme",
					"Gtk theme",
					"Use gtk theme or not",
					FALSE,
					G_PARAM_READWRITE));

	g_object_class_install_property(object_class,
				PROP_GTK_THEME,
				g_param_spec_object("layout",
					"Layout",
					"Window layout",
					G_TYPE_OBJECT,
					G_PARAM_READABLE));

	g_object_class_install_property(object_class,
				PROP_MENU_POPUP,
				g_param_spec_object("menu-popup",
					"Popup menu",
					"Popup menu",
					G_TYPE_OBJECT,
					G_PARAM_READABLE));

	g_object_class_install_property(object_class,
				PROP_PLAYLIST,
				g_param_spec_pointer("playlist",
					"playlist",
					"Current playlist",
					G_PARAM_READABLE));
}

static void
create_signals(XmrWindowClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	/**
	 * emit when user changed skin
	 * when @new_theme is NULL, it means
	 * UI use GTK theme
	 */
	signals[THEME_CHANGED] =
		g_signal_new("theme-changed",
					G_OBJECT_CLASS_TYPE(object_class),
					G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
					G_STRUCT_OFFSET(XmrWindowClass, theme_changed),
					NULL, NULL,
					g_cclosure_marshal_VOID__STRING,
					G_TYPE_NONE,
					1,
					G_TYPE_STRING);

	/**
	 * emit when current track changed
	 */
	signals[TRACK_CHANGED] =
		g_signal_new("track-changed",
					G_OBJECT_CLASS_TYPE(object_class),
					G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
					G_STRUCT_OFFSET(XmrWindowClass, track_changed),
					NULL, NULL,
					g_cclosure_marshal_VOID__POINTER,
					G_TYPE_NONE,
					1,
					G_TYPE_POINTER);

	/**
	 * emit when current radio changed
	 */
	signals[RADIO_CHANGED] =
		g_signal_new("radio-changed",
					G_OBJECT_CLASS_TYPE(object_class),
					G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
					G_STRUCT_OFFSET(XmrWindowClass, radio_changed),
					NULL, NULL,
					xmr_marshal_VOID__STRING_STRING,
					G_TYPE_NONE,
					2,
					G_TYPE_STRING, G_TYPE_STRING);

	signals[LOGIN_FINISH] =
			g_signal_new("login-finish",
						 G_OBJECT_CLASS_TYPE(object_class),
						 G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
						 G_STRUCT_OFFSET(XmrWindowClass, login_finish),
						 NULL, NULL,
						 xmr_marshal_VOID__BOOLEAN_STRING,
						 G_TYPE_NONE,
						 2,
						 G_TYPE_BOOLEAN, G_TYPE_STRING);

	signals[LOGOUT] =
			g_signal_new("logout",
						 G_OBJECT_CLASS_TYPE(object_class),
						 G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
						 G_STRUCT_OFFSET(XmrWindowClass, logout),
						 NULL, NULL,
						 g_cclosure_marshal_VOID__VOID,
						 G_TYPE_NONE,
						 0);

	signals[FETCH_PLAYLIST_FINISH] =
			g_signal_new("fetch-playlist-finish",
						 G_OBJECT_CLASS_TYPE(object_class),
						 G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
						 G_STRUCT_OFFSET(XmrWindowClass, fetch_playlist_finish),
						 NULL, NULL,
						 xmr_marshal_VOID__INT_POINTER,
						 G_TYPE_NONE,
						 2,
						 G_TYPE_INT, G_TYPE_POINTER);

	signals[FETCH_COVER_FINISH] =
			g_signal_new("fetch-cover-finish",
						 G_OBJECT_CLASS_TYPE(object_class),
						 G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
						 G_STRUCT_OFFSET(XmrWindowClass, fetch_cover_finish),
						 NULL, NULL,
						 g_cclosure_marshal_VOID__POINTER,
						 G_TYPE_NONE,
						 1,
						 G_TYPE_POINTER);
	
	signals[SEARCH_MUSIC] =
			g_signal_new("search-music",
						 G_OBJECT_CLASS_TYPE(object_class),
						 G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
						 G_STRUCT_OFFSET(XmrWindowClass, search_music),
						 NULL, NULL,
						 g_cclosure_marshal_VOID__STRING,
						 G_TYPE_NONE,
						 1,
						 G_TYPE_STRING);
	
	signals[PLAYLIST_CHANGED] =
			g_signal_new("playlist-changed",
						 G_OBJECT_CLASS_TYPE(object_class),
						 G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
						 G_STRUCT_OFFSET(XmrWindowClass, playlist_changed),
						 NULL, NULL,
						 g_cclosure_marshal_VOID__VOID,
						 G_TYPE_NONE,
						 0);
}

static void 
xmr_window_class_init(XmrWindowClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->set_property = xmr_window_set_property;
	object_class->get_property = xmr_window_get_property;

	object_class->dispose = xmr_window_dispose;
	object_class->finalize = xmr_window_finalize;

	install_properties(object_class);
	create_signals(klass);

	g_type_class_add_private(object_class, sizeof(XmrWindowPrivate));
}

static void 
xmr_window_init(XmrWindow *window)
{
	XmrWindowPrivate *priv;
	gint i;

	window->priv = G_TYPE_INSTANCE_GET_PRIVATE(window, XMR_TYPE_WINDOW, XmrWindowPrivate);
	priv = window->priv;

	priv->player = xmr_player_new();
	priv->service = xmr_service_new();
	priv->skin = NULL;
	priv->gtk_theme = FALSE;
	priv->cs_bkgnd = NULL;
	priv->playlist = NULL;
	priv->playlist_url = NULL;
	priv->settings = xmr_settings_new();
	priv->skin_list = NULL;
	priv->skin_item_group = NULL;
	priv->ui_pref = create_builder_with_file(UIDIR, "pref.ui");
	priv->ui_login = NULL;
	priv->usr = NULL;
	priv->pwd = NULL;
	priv->switch_radio = FALSE;
	priv->pb_cover = NULL;
	priv->pb_cover_current = NULL;
	priv->cover_request_width = COVER_WIDTH;
	priv->cover_request_height = COVER_HEIGHT;
	priv->plugin_engine = xmr_plugin_engine_new();
	priv->extensions = peas_extension_set_new(PEAS_ENGINE(priv->plugin_engine),
						   PEAS_TYPE_ACTIVATABLE,
						   "object", window,
						   NULL);
	peas_extension_set_foreach(priv->extensions,
                              (PeasExtensionSetForeachFunc)on_extension_added,
                              NULL);

#if GLIB_CHECK_VERSION(2, 32, 0)
	priv->mutex = g_malloc(sizeof(GMutex));
	g_mutex_init(priv->mutex);
#else
	priv->mutex = g_mutex_new();
#endif

	priv->dialog_login = NULL;
	priv->syncing_volume = FALSE;

	priv->message.message = NULL;
	priv->message.title = NULL;

	priv->downloader = xmr_downloader_new();
	priv->waiting_wnd = xmr_waiting_wnd_new(GTK_WINDOW(window));
	
	priv->xmr_searchlist = NULL;
	priv->radio_search_box = NULL;

	priv->current_song = NULL;
	priv->radio_type = Radio_Type_Custom;

	priv->queue_fetch_playlist = g_async_queue_new();
	priv->queue_fetch_cover = g_async_queue_new();
	priv->queue_event = g_async_queue_new();
	priv->xmr_event_timer = g_timeout_add(XMR_EVENT_INTERVAL, (GSourceFunc)xmr_event_poll, window);
	priv->buffering_timer = 0;
	priv->use_gst_buffering = FALSE;
	priv->search_music_count = 0;
	priv->search_music_idle_id = 0;

	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_widget_set_app_paintable(GTK_WIDGET(window), TRUE);
	gtk_widget_add_events(GTK_WIDGET(window), GDK_BUTTON_PRESS_MASK);

	gtk_window_set_default_icon_name("xmradio");

	priv->fixed = gtk_fixed_new();
	create_popup_menu(window);

	for(i=0; i<LAST_BUTTON; ++i)
	{
		if (i == BUTTON_VOLUME){
			priv->buttons[i] = xmr_volume_button_new(XMR_BUTTON_SKIN);
		}else{
			priv->buttons[i] = xmr_button_new(XMR_BUTTON_SKIN);
		}
		gtk_fixed_put(GTK_FIXED(priv->fixed), priv->buttons[i], 0, 0);

		g_signal_connect(priv->buttons[i], "clicked",
					G_CALLBACK(on_xmr_button_clicked), (gpointer)(glong)i);
	}

	for(i=0; i<LAST_LABEL; ++i)
	{
		priv->labels[i] = xmr_label_new("");
		gtk_fixed_put(GTK_FIXED(priv->fixed), priv->labels[i], 0, 0);
	}

	priv->image = gtk_image_new();
	gtk_fixed_put(GTK_FIXED(priv->fixed), priv->image, 0, 0);
	
	priv->search_box = gtk_entry_new();
	gtk_entry_set_placeholder_text(GTK_ENTRY(priv->search_box), _("Music search ..."));
	gtk_entry_set_icon_from_stock(GTK_ENTRY(priv->search_box), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_FIND);

	gtk_fixed_put(GTK_FIXED(priv->fixed), priv->search_box, 0, 0);

	gtk_container_add(GTK_CONTAINER(window), priv->fixed);

	gtk_widget_show(priv->fixed);

	priv->chooser[0] = xmr_chooser_new(_("风格电台"), GTK_ORIENTATION_HORIZONTAL);
	priv->chooser[1] = xmr_chooser_new(_("星座电台"), GTK_ORIENTATION_HORIZONTAL);
	priv->chooser[2] = xmr_chooser_new(_("年代电台"), GTK_ORIENTATION_HORIZONTAL);

	g_signal_connect(priv->settings, "changed", G_CALLBACK(on_settings_changed), window);

	g_signal_connect(window, "draw", G_CALLBACK(on_draw), NULL);
	g_signal_connect(window, "button-press-event", G_CALLBACK(on_button_press), NULL);

	g_signal_connect(priv->player, "eos", G_CALLBACK(player_eos), window);
	g_signal_connect(priv->player, "error", G_CALLBACK(player_error), window);
	g_signal_connect(priv->player, "tick", G_CALLBACK(player_tick), window);
	g_signal_connect(priv->player, "buffering", G_CALLBACK(player_buffering), window);
	g_signal_connect(priv->player, "state-changed", G_CALLBACK(player_state_changed), window);
	g_signal_connect(priv->player, "volume-changed", G_CALLBACK(player_volume_changed), window);

	g_signal_connect(window, "login-finish", G_CALLBACK(login_finish), NULL);
	g_signal_connect(window, "logout", G_CALLBACK(on_logout), NULL);
	g_signal_connect(window, "fetch-playlist-finish", G_CALLBACK(fetch_playlist_finish), NULL);
	g_signal_connect(window, "fetch-cover-finish", G_CALLBACK(fetch_cover_finish), NULL);
	g_signal_connect(window, "playlist-changed", G_CALLBACK(on_playlist_changed), NULL);

	g_signal_connect(priv->downloader, "download-finish", G_CALLBACK(download_finish), window);
	g_signal_connect(priv->downloader, "download-progress", G_CALLBACK(download_progress), window);
	g_signal_connect(priv->downloader, "download-failed", G_CALLBACK(download_failed), window);

	g_signal_connect(priv->search_box, "activate", G_CALLBACK(on_search_box_activate), window);
	g_signal_connect(priv->search_box, "focus-in-event", G_CALLBACK(on_search_box_focus_in), NULL);
	g_signal_connect(priv->search_box, "focus-out-event", G_CALLBACK(on_search_box_focus_out), NULL);

	for(i=0; i<3; ++i)
	  g_signal_connect(priv->chooser[i], "widget-selected",
				  G_CALLBACK(radio_selected), window);

	g_signal_connect_after(window, "delete-event", G_CALLBACK(on_delete_event), NULL);

	g_signal_connect(priv->extensions, "extension-added",
			  G_CALLBACK(on_extension_added), NULL);

	g_signal_connect(priv->extensions, "extension-removed",
			  G_CALLBACK(on_extension_removed), NULL);

	g_signal_connect(priv->buttons[BUTTON_VOLUME], "value-changed",
			  G_CALLBACK(on_volume_button_value_changed),
			  window);

	g_settings_bind(G_SETTINGS(priv->settings),
				"volume",
				priv->buttons[BUTTON_VOLUME], "value",
				G_SETTINGS_BIND_DEFAULT);
	g_settings_bind(G_SETTINGS(priv->settings),
				"ontop",
				priv->menu_item_ontop, "active",
				G_SETTINGS_BIND_DEFAULT);

	gtk_widget_hide(priv->buttons[BUTTON_PAUSE]);

#if GLIB_CHECK_VERSION(2, 32, 0)
	g_thread_new("playlist", (GThreadFunc)thread_get_playlist, window);
	g_thread_new("gcimage", (GThreadFunc)thread_get_cover_image, window);
#else
	g_thread_create((GThreadFunc)thread_get_playlist, window, FALSE, NULL);
	g_thread_create((GThreadFunc)thread_get_cover_image, window, FALSE, NULL);
#endif

	load_settings(window);
}

GtkWidget* xmr_window_new()
{
	 return g_object_new(XMR_TYPE_WINDOW,
				"type", GTK_WINDOW_TOPLEVEL,
				"title", _("XMRadio"),
				"icon-name", "xmradio",
				"resizable", FALSE,
				NULL);
}

static void
xmr_window_dispose(GObject *obj)
{
	XmrWindow *window = XMR_WINDOW(obj);
	XmrWindowPrivate *priv = window->priv;

	if (priv->queue_fetch_playlist)
	{
		while (g_async_queue_try_pop(priv->queue_fetch_playlist) != NULL)
			;

		g_async_queue_unref(priv->queue_fetch_playlist);
		priv->queue_fetch_playlist = NULL;
	}

	if (priv->queue_fetch_cover)
	{
		gpointer p = NULL;
		while ((p = g_async_queue_try_pop(priv->queue_fetch_cover)) != NULL)
			g_free(p);
		
		g_async_queue_unref(priv->queue_fetch_cover);
		priv->queue_fetch_cover = NULL;
	}

	if (priv->xmr_event_timer != 0)
	{
		g_source_remove(priv->xmr_event_timer);
		priv->xmr_event_timer = 0;
	}

	if (priv->buffering_timer != 0)
	{
		g_source_remove(priv->buffering_timer);
		priv->buffering_timer = 0;
	}
	
	if (priv->search_music_idle_id)
	{
		g_source_remove(priv->search_music_idle_id);
		priv->search_music_idle_id = 0;
	}

	if (priv->queue_event)
	{
		g_async_queue_unref(priv->queue_event);
		priv->queue_event = NULL;
	}

	peas_engine_garbage_collect(PEAS_ENGINE(priv->plugin_engine));
	
	if (priv->extensions != NULL)
	{
		g_object_unref (priv->extensions);
		priv->extensions = NULL;
	}

	if (priv->ui_pref)
	{
		g_object_unref(priv->ui_pref);
		priv->ui_pref = NULL;
	}

	if (priv->ui_login)
	{
		g_object_unref(priv->ui_login);
		priv->ui_login = NULL;
	}

	if (priv->pb_cover)
	{
		g_object_unref(priv->pb_cover);
		priv->pb_cover = NULL;
	}

	if (priv->downloader)
	{
		g_signal_handlers_disconnect_by_func(priv->downloader, G_CALLBACK(download_finish), window);
		g_signal_handlers_disconnect_by_func(priv->downloader, G_CALLBACK(download_progress), window);
		g_signal_handlers_disconnect_by_func(priv->downloader, G_CALLBACK(download_failed), window);

		g_object_unref(priv->downloader);
		priv->downloader = NULL;
	}

	if (priv->settings)
	{
		g_signal_handlers_disconnect_by_func(priv->settings, G_CALLBACK(on_settings_changed), window);
		g_object_unref(priv->settings);
		priv->settings = NULL;
	}

	if (priv->player != NULL)
	{
		g_signal_handlers_disconnect_by_func(priv->player, G_CALLBACK(player_eos), window);
		g_signal_handlers_disconnect_by_func(priv->player, G_CALLBACK(player_error), window);
		g_signal_handlers_disconnect_by_func(priv->player, G_CALLBACK(player_tick), window);
		g_signal_handlers_disconnect_by_func(priv->player, G_CALLBACK(player_buffering), window);
		g_signal_handlers_disconnect_by_func(priv->player, G_CALLBACK(player_state_changed), window);
		g_signal_handlers_disconnect_by_func(priv->player, G_CALLBACK(player_volume_changed), window);

		g_object_unref(priv->player);
		priv->player = NULL;
	}

	if (priv->service != NULL)
	{
		g_object_unref(priv->service);
		priv->service = NULL;
	}

	G_OBJECT_CLASS(xmr_window_parent_class)->dispose(obj);
}

static void
xmr_window_finalize(GObject *obj)
{
	XmrWindow *window = XMR_WINDOW(obj);
	XmrWindowPrivate *priv = window->priv;

	if (priv->skin)
		g_free(priv->skin);

	if (priv->cs_bkgnd)
		cairo_surface_destroy(priv->cs_bkgnd);

	if (priv->playlist_url)
		g_free(priv->playlist_url);

	if (priv->playlist)
		g_list_free_full(priv->playlist, (GDestroyNotify)song_info_free);

	if (priv->skin_list)
		g_list_free_full(priv->skin_list, (GDestroyNotify)xmr_skin_info_free);

	if (priv->usr)
		g_free(priv->usr);
	if (priv->pwd)
		g_free(priv->pwd);

	if (priv->mutex)
	{
#if GLIB_CHECK_VERSION(2, 32, 0)
		g_mutex_clear(priv->mutex);
		g_free(priv->mutex);
#else
		g_mutex_free(priv->mutex);
#endif
		priv->mutex = NULL;
	}

	if (priv->current_song)
	{
		song_info_free(priv->current_song);
		priv->current_song = NULL;
	}

	G_OBJECT_CLASS(xmr_window_parent_class)->finalize(obj);
}

static void
xmr_window_get_property(GObject *object,
		       guint prop_id,
		       GValue *value,
		       GParamSpec *pspec)
{
	XmrWindow *window = XMR_WINDOW(object);
	XmrWindowPrivate *priv = window->priv;

	switch(prop_id)
	{
	case PROP_PLAYER:
		g_value_set_object(value, priv->player);
		break;

	case PROP_SERVICE:
		g_value_set_object(value, priv->service);
		break;

	case PROP_SKIN:
		g_value_set_string(value, priv->skin);
		break;

	case PROP_GTK_THEME:
		g_value_set_boolean(value, priv->gtk_theme);
		break;

	case PROP_LAYOUT:
		g_value_set_object(value, priv->fixed);
		break;

	case PROP_MENU_POPUP:
		g_value_set_object(value, priv->popup_menu);
		break;

	case PROP_PLAYLIST:
		g_value_set_pointer(value, priv->playlist);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void
xmr_window_set_property(GObject *object,
		       guint prop_id,
		       const GValue *value,
		       GParamSpec *pspec)
{
	XmrWindow *window = XMR_WINDOW(object);
	XmrWindowPrivate *priv = window->priv;

	switch(prop_id)
	{
	case PROP_SKIN:
		g_free(priv->skin);
		priv->skin = g_value_dup_string(value);
		break;

	case PROP_GTK_THEME:
		priv->gtk_theme = g_value_get_boolean(value);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static gboolean
on_draw(XmrWindow *window, cairo_t *cr, gpointer data)
{
	XmrWindowPrivate *priv = window->priv;

	if (priv->gtk_theme || priv->cs_bkgnd == NULL)
	{
		cairo_set_source_rgb(cr, 128.0/255.0, 128.0/255.0, 128.0/255.0);
		cairo_rectangle(cr, 0, 60, 500, 1);

		cairo_fill(cr);

		return GTK_WIDGET_CLASS(xmr_window_parent_class)->draw(GTK_WIDGET(window), cr);
	}

	cairo_set_source_surface(cr, priv->cs_bkgnd, 0, 0);
	cairo_paint(cr);
	
	return FALSE;
}

static gboolean
on_button_press(XmrWindow *window, GdkEventButton *event, gpointer data)
{
	if (event->button == 1)  
    {
		if (window->priv->gtk_theme) 	// ignore if using gtk theme
			return FALSE;
		gtk_window_begin_move_drag(GTK_WINDOW(window),
					event->button,
					event->x_root, event->y_root,
					event->time);
    }
	else if (event->button == 3)
	{
		GtkMenu *menu = GTK_MENU(window->priv->popup_menu);
		gtk_menu_popup(menu, NULL, NULL, NULL, NULL,
					event->button, event->time);
		return TRUE;
	}

	return FALSE;
}

static void
on_xmr_button_clicked(GtkWidget *widget, gpointer data)
{
	XmrWindow *window = XMR_WINDOW(gtk_widget_get_toplevel(widget));
	XmrWindowPrivate *priv = window->priv;
	glong id = (glong)data;

	switch(id)
	{
	case BUTTON_CLOSE:
		{
			gboolean retv = FALSE;
			g_signal_emit_by_name(window, "delete-event", NULL, &retv);
		}
		break;

	case BUTTON_MINIMIZE:
		gtk_window_iconify(GTK_WINDOW(window));
		break;

	case BUTTON_PLAY:
		xmr_window_play(window);
		break;

	case BUTTON_PAUSE:
		xmr_window_pause(window);
		break;

	case BUTTON_NEXT:
		xmr_window_play_next(window);
		break;

	case BUTTON_LIKE:
		// FIRST time click MARK as like
		// SECOND time mark as NOT like
		like_current_song(window, TRUE);
		if (xmr_button_is_toggled(XMR_BUTTON(priv->buttons[BUTTON_LIKE])))
		{
			xmr_button_toggle_state_off(XMR_BUTTON(priv->buttons[BUTTON_LIKE]));
		}
		else
		{
			// should check if logged in
			// and wait for like_current_song return state
			if (xmr_service_is_logged_in(priv->service))
				xmr_button_toggle_state_on(XMR_BUTTON(priv->buttons[BUTTON_LIKE]), STATE_FOCUS);
		}
		break;

	case BUTTON_DISLIKE:
		like_current_song(window, FALSE);
		xmr_button_toggle_state_off(XMR_BUTTON(priv->buttons[BUTTON_LIKE]));
		break;

	case BUTTON_LYRIC:
	case BUTTON_DOWNLOAD:
	{
		gint ret;
		SongInfo *song = xmr_window_get_current_song(window);
		gchar *command = NULL;
		if (song == NULL)
		{
			xmr_debug("Playlist empty");
			break;
		}
		if (id == BUTTON_LYRIC){
			command = g_strdup_printf("xdg-open http://www.xiami.com/radio/lyric/sid/%s", song->song_id);
		}else{
			command = g_strdup_printf("xdg-open http://www.xiami.com/song/%s", song->song_id);
		}
		if (command == NULL)
			g_error("No more memory\n");

		ret = system(command);
		xmr_debug("call system: %d", ret);

		g_free(command);
	}
		break;

	case BUTTON_SHARE:
		xmr_debug("Not implemented yet");
		break;
	
	case BUTTON_SIREN:
		change_radio_style(window, 0);
		break;

	case BUTTON_FENGGE:
		change_radio_style(window, 2);
		break;

	case BUTTON_XINGZUO:
		change_radio_style(window, 3);
		break;

	case BUTTON_NIANDAI:
		change_radio_style(window, 4);
		break;

	case BUTTON_VOLUME:
		break;
	}
}

static void
radio_selected(XmrChooser *chooser,
			XmrRadio *radio,
			XmrWindow *window)
{
	const gchar *name = xmr_radio_get_name(radio);
	const gchar *url = xmr_radio_get_url(radio);

	window->priv->radio_type = Radio_Type_Custom;
	change_radio(window, name, url);
}

static void
set_window_image(XmrWindow *window, GdkPixbuf *pixbuf)
{
	cairo_surface_t *image;
	cairo_t *cr;
	cairo_region_t *mask;
	gint image_width, image_height;
	XmrWindowPrivate *priv;
	
	g_return_if_fail(window != NULL && pixbuf != NULL);
	priv = window->priv;

	image_width = gdk_pixbuf_get_width(pixbuf);
	image_height = gdk_pixbuf_get_height(pixbuf);

	image = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, image_width, image_height);
	if (image == NULL)
		return ;

	cr = cairo_create(image);

	gdk_cairo_set_source_pixbuf(cr, pixbuf, 0, 0);
	cairo_paint(cr);

	cairo_destroy(cr);

	if (priv->cs_bkgnd){
		cairo_surface_destroy(priv->cs_bkgnd);
	}

	priv->cs_bkgnd = image;
	mask = gdk_cairo_region_create_from_surface(priv->cs_bkgnd);
	if (mask == NULL)
		return ;
	gtk_widget_shape_combine_region(GTK_WIDGET(window), mask);

	cairo_region_destroy(mask);

	gtk_widget_set_size_request(GTK_WIDGET(window), image_width, image_height);

//	gtk_widget_queue_draw(GTK_WIDGET(window));
}

static void
set_cover_image(XmrWindow *window, GdkPixbuf *pixbuf)
{
	XmrWindowPrivate *priv = window->priv;

	gint i_w, i_h;

	i_w = gdk_pixbuf_get_width(pixbuf);
	i_h = gdk_pixbuf_get_height(pixbuf);

	if (i_w != priv->cover_request_width ||
		i_h != priv->cover_request_height) // need scale
	{
		GdkPixbuf *pb = gdk_pixbuf_scale_simple(pixbuf,
							priv->cover_request_width,
							priv->cover_request_height,
							GDK_INTERP_BILINEAR);
		if (pb)
		{
			gtk_image_set_from_pixbuf(GTK_IMAGE(priv->image), pb);
			g_object_unref(pb);
		}
	}
	else
	{
		gtk_image_set_from_pixbuf(GTK_IMAGE(priv->image), pixbuf);
	}
}

static void
set_gtk_theme(XmrWindow *window)
{
	XmrWindowPrivate *priv = window->priv;
	struct Pos { gint x, y; };
	gint i;

	static struct Pos label_pos[LAST_LABEL] =
	{
		{140, 20}, {135, 82},
		{135, 132}, {135, 157}, {135, 107}
	};

	priv->gtk_theme = TRUE;
	priv->cover_request_width = COVER_WIDTH;
	priv->cover_request_height = COVER_HEIGHT;

	gtk_window_set_decorated(GTK_WINDOW(window), TRUE);
	gtk_widget_set_size_request(GTK_WIDGET(window), 500, 200);

	// remove any existing shape
	gtk_widget_shape_combine_region(GTK_WIDGET(window), NULL);

	hide_children(window);

	gtk_fixed_move(GTK_FIXED(priv->fixed), priv->buttons[BUTTON_PLAY], 25, 5);
	xmr_button_set_type(XMR_BUTTON(priv->buttons[BUTTON_PLAY]), XMR_BUTTON_NORMAL);
	xmr_button_set_image_from_stock(XMR_BUTTON(priv->buttons[BUTTON_PLAY]), GTK_STOCK_MEDIA_PLAY);

	gtk_fixed_move(GTK_FIXED(priv->fixed), priv->buttons[BUTTON_PAUSE], 25, 5);
	xmr_button_set_type(XMR_BUTTON(priv->buttons[BUTTON_PAUSE]), XMR_BUTTON_NORMAL);
	xmr_button_set_image_from_stock(XMR_BUTTON(priv->buttons[BUTTON_PAUSE]), GTK_STOCK_MEDIA_PAUSE);

	gtk_fixed_move(GTK_FIXED(priv->fixed), priv->buttons[BUTTON_NEXT], 75, 5);
	xmr_button_set_type(XMR_BUTTON(priv->buttons[BUTTON_NEXT]), XMR_BUTTON_NORMAL);
	xmr_button_set_image_from_stock(XMR_BUTTON(priv->buttons[BUTTON_NEXT]), GTK_STOCK_MEDIA_NEXT);
	gtk_widget_show(priv->buttons[BUTTON_NEXT]);

	if (xmr_player_playing(priv->player))
		gtk_widget_show(priv->buttons[BUTTON_PAUSE]);
	else
		gtk_widget_show(priv->buttons[BUTTON_PLAY]);

	for(i=0; i<LAST_LABEL; ++i)
	{
		gtk_fixed_move(GTK_FIXED(priv->fixed), priv->labels[i], label_pos[i].x, label_pos[i].y);
		xmr_label_set_size(XMR_LABEL(priv->labels[i]), 120, 20);
		gtk_widget_show(priv->labels[i]);

		xmr_label_set_color(XMR_LABEL(priv->labels[i]), NULL);
		xmr_label_set_font(XMR_LABEL(priv->labels[i]), NULL);
	}

	gtk_fixed_move(GTK_FIXED(priv->fixed), priv->image, 25, 80);
	// update cover size
	if (priv->pb_cover_current != NULL)
		set_cover_image(window, priv->pb_cover_current);

	gtk_widget_show(priv->image);
	
	{
		gint size = get_search_box_font_width(priv->search_box);
		gint chars = 200 / size + 1;

		gtk_entry_set_width_chars(GTK_ENTRY(priv->search_box), chars);
	}
	gtk_fixed_move(GTK_FIXED(priv->fixed), priv->search_box, 320, 15);
	gtk_widget_show(priv->search_box);

	xmr_settings_set_theme(priv->settings, "");

	gtk_widget_queue_draw(GTK_WIDGET(window));
}

static void
set_skin(XmrWindow *window, SkinInfo *info)
{
	XmrWindowPrivate *priv = window->priv;
	gint x, y;
	GdkPixbuf *pixbuf;
	gint i;
	XmrSkin *xmr_skin;
	static const gchar *ui_main_buttons[] =
	{
		"close", "minimize", "play", "pause",
		"next", "like", "dislike", "volume",
		"lyric", "download", "share", "siren",
		"fengge", "xingzuo", "niandai"
	};

	static const gchar *ui_main_labels[] =
	{
		"radio_name", "song_name",
		"artist", "progress", "album"
	};

	xmr_skin = xmr_skin_new();

	do
	{
		if (!xmr_skin_load(xmr_skin, info->file))
		{
			xmr_debug("failed to load skin: %s", info->file);
			break;
		}

		pixbuf = xmr_skin_get_image(xmr_skin, UI_MAIN, NULL);
		if (pixbuf == NULL)
		{
			xmr_debug("Missing background image ?");
			break;
		}

		priv->gtk_theme = FALSE;

		gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
		set_window_image(window, pixbuf);
		g_object_unref(pixbuf);

		hide_children(window);

		for(i=0; i<LAST_BUTTON; ++i)
		{
			pixbuf = xmr_skin_get_image(xmr_skin, UI_MAIN, ui_main_buttons[i]);
			if (pixbuf == NULL)
				continue ;

			if (i == BUTTON_VOLUME)
			{
				xmr_volume_button_set_image_from_pixbuf(XMR_VOLUME_BUTTON(priv->buttons[i]), pixbuf);
				xmr_volume_button_set_type(XMR_VOLUME_BUTTON(priv->buttons[i]), XMR_BUTTON_SKIN);
			}
			else
			{
				xmr_button_set_image_from_pixbuf(XMR_BUTTON(priv->buttons[i]), pixbuf);
				xmr_button_set_type(XMR_BUTTON(priv->buttons[i]), XMR_BUTTON_SKIN);
			}
			g_object_unref(pixbuf);


			if (xmr_skin_get_position(xmr_skin, UI_MAIN, ui_main_buttons[i], &x, &y))
			{
				gtk_fixed_move(GTK_FIXED(priv->fixed), priv->buttons[i], x, y);
				gtk_widget_show(priv->buttons[i]);
			}
		}

		for(i=0; i<LAST_LABEL; ++i)
		{
			gchar *value = NULL;
			if (xmr_skin_get_position(xmr_skin, UI_MAIN, ui_main_labels[i], &x, &y))
			{
				gtk_fixed_move(GTK_FIXED(priv->fixed), priv->labels[i], x, y);
				gtk_widget_show(priv->labels[i]);
			
				if (xmr_skin_get_size(xmr_skin, UI_MAIN, ui_main_labels[i], &x, &y)){
					xmr_label_set_size(XMR_LABEL(priv->labels[i]), x, y);
				} else {
					xmr_label_set_size(XMR_LABEL(priv->labels[i]), -1, -1);
				}
			}

			xmr_skin_get_color(xmr_skin, UI_MAIN, ui_main_labels[i], &value);
			xmr_label_set_color(XMR_LABEL(priv->labels[i]), value);
			g_free(value);

			value = NULL;
			xmr_skin_get_font(xmr_skin, UI_MAIN, ui_main_labels[i], &value);
			xmr_label_set_font(XMR_LABEL(priv->labels[i]), value);
			g_free(value);
		}

		if (xmr_skin_get_position(xmr_skin, UI_MAIN, "cover", &x, &y))
		{
			gtk_fixed_move(GTK_FIXED(priv->fixed), priv->image, x, y);
			gtk_widget_show(priv->image);
		}
		
		if (xmr_skin_get_size(xmr_skin, UI_MAIN, "cover", &x, &y))
		{
			priv->cover_request_width = x;
			priv->cover_request_height = y;
		}
		else
		{
			priv->cover_request_width = COVER_WIDTH;
			priv->cover_request_height = COVER_HEIGHT;
		}

		if (priv->pb_cover)
			g_object_unref(priv->pb_cover);

		priv->pb_cover = xmr_skin_get_image(xmr_skin, UI_MAIN, "cover");
		if (priv->pb_cover && priv->pb_cover_current == NULL)
			set_cover_image(window, priv->pb_cover);
		else if (priv->pb_cover_current != NULL)
			set_cover_image(window, priv->pb_cover_current);
		
		gtk_widget_hide(priv->search_box);
		if (xmr_skin_get_position(xmr_skin, UI_MAIN, "search_box", &x, &y))
		{
			gtk_widget_show(priv->search_box);
			gtk_entry_set_width_chars(GTK_ENTRY(priv->search_box), 15);
			gtk_fixed_move(GTK_FIXED(priv->fixed), priv->search_box, x, y);
		}
		
		if (xmr_skin_get_size(xmr_skin, UI_MAIN, "search_box", &x, &y))
		{
			gint size = get_search_box_font_width(priv->search_box);
			gint chars = x / size + 1;
	
			gtk_entry_set_width_chars(GTK_ENTRY(priv->search_box), chars);
		}

		// save to settings
		{
			xmr_settings_set_theme(priv->settings, info->name);

			g_free(priv->skin);
			priv->skin = g_strdup(info->name);
		}

		gtk_widget_queue_draw(GTK_WIDGET(window));
		g_signal_emit(window, signals[THEME_CHANGED], 0, info->file);

		if (xmr_player_playing(priv->player))
			gtk_widget_hide(priv->buttons[BUTTON_PLAY]);
		else
			gtk_widget_hide(priv->buttons[BUTTON_PAUSE]);
	}
	while(0);

	g_object_unref(xmr_skin);
}

static void
hide_children(XmrWindow *window)
{
	XmrWindowPrivate *priv = window->priv;
	gint i;

	for(i=0; i<LAST_BUTTON; ++i)
	  gtk_widget_hide(priv->buttons[i]);

	for(i=0; i<LAST_LABEL; ++i)
	  gtk_widget_hide(priv->labels[i]);

	gtk_widget_hide(priv->image);
}

static void
player_eos(XmrPlayer *player,
			gboolean early,
			XmrWindow *window)
{
	if (!early) {
		xmr_event_send(window, XMR_EVENT_PLAYER_EOS, NULL);
	}
}

static void
player_error(XmrPlayer *player,
			GError *error,
			XmrWindow *window)
{
	xmr_debug("Player error(%d): %s\n", error->code, error->message);
	// play next if "Stream contains no data" occurres
	if (error->code == GST_STREAM_ERROR_TYPE_NOT_FOUND) {
		xmr_event_send(window, XMR_EVENT_PLAYER_EOS, NULL);
	}
}

static void
player_tick(XmrPlayer *player,
			gint64 elapsed,
			gint64 duration,
			XmrWindow *window)
{
	glong mins_elapsed;
	glong secs_elapsed;

	glong mins_duration;
	glong secs_duration;

	gint64 secs;
	gchar *time;

	secs = elapsed / G_USEC_PER_SEC / 1000;
	mins_elapsed = secs / 60;
	secs_elapsed = secs % 60;

	secs = duration / G_USEC_PER_SEC / 1000;
	mins_duration = secs / 60;
	secs_duration = secs % 60;

	time = g_strdup_printf("%02ld:%02ld/%02ld:%02ld",
				mins_elapsed, secs_elapsed,
				mins_duration, secs_duration
				);

	xmr_event_send(window, XMR_EVENT_PLAYER_TICK, time);
}

static void
player_buffering(XmrPlayer *player,
			guint progress,
			XmrWindow *window)
{
	// to avoid too many buffering event
	if (progress % 25 == 0)
		xmr_event_send(window, XMR_EVENT_PLAYER_BUFFERING, GINT_TO_POINTER(progress));
}

static void
player_state_changed(XmrPlayer *player,
			gint old_state,
			gint new_state,
			XmrWindow *window)
{
	xmr_event_send(window, XMR_EVENT_PLAYER_STATE_CHANGED, GINT_TO_POINTER(new_state));
}

static void
player_volume_changed(XmrPlayer *player,
			float volume,
			XmrWindow *window)
{
	window->priv->syncing_volume = TRUE;
	gtk_scale_button_set_value(GTK_SCALE_BUTTON(window->priv->buttons[BUTTON_VOLUME]), volume);
	window->priv->syncing_volume = FALSE;
}

static gpointer
thread_get_playlist(XmrWindow *window)
{
	XmrWindowPrivate *priv = window->priv;

	while (priv->queue_fetch_playlist)
	{
		gint result = 1;
		GList *list = NULL;

		g_async_queue_pop(priv->queue_fetch_playlist);

		// empty queue
		do {} while (g_async_queue_try_pop(priv->queue_fetch_playlist) != NULL);

		xmr_debug("[BEGIN] thread_get_playlist");

		if (xmr_service_is_logged_in(priv->service))
		{
			if (priv->radio_type != Radio_Type_Custom)
			{
				g_mutex_lock(priv->mutex);
				result = xmr_service_get_track_list_by_id(priv->service, &list, priv->radio_type);
				g_mutex_unlock(priv->mutex);
			}
			else
			{
				g_mutex_lock(priv->mutex);
				result = xmr_service_get_track_list_by_style(priv->service, &list, priv->playlist_url);
				g_mutex_unlock(priv->mutex);
			}
		}
		else
		{
			XmrService *service = NULL;

			service = xmr_service_new();
			if (service)
			{
				result = xmr_service_get_track_list_by_style(service, &list, priv->playlist_url);
				g_object_unref(service);
			}
		}

		{
			FetchPlaylist *p = g_new(FetchPlaylist, 1);
			p->window = window;
			p->result = result;
			p->list = list;

			xmr_event_send(window, XMR_EVENT_PLAYLIST, p);
		}

		xmr_debug("[END] thread_get_playlist");
	}

	return NULL;
}

static gpointer
thread_get_cover_image(XmrWindow *window)
{
	XmrWindowPrivate *priv = window->priv;

	while (priv->queue_fetch_cover)
	{
		XmrService *service = NULL;
		gchar *url = NULL;

		url = g_async_queue_pop(priv->queue_fetch_cover);

		// empty queue
		// get the last one only
		while (1)
		{
			gchar *t = g_async_queue_try_pop(priv->queue_fetch_cover);
			if (t == NULL)
				break;

			g_free(url);
			url = t;
		}

		xmr_debug("[BEGIN] fetching: %s", url);

		do
		{
			gint result;
			GString *data = NULL;

			data = g_string_new("");
			if (data == NULL)
				break;

			service = xmr_service_new();
			if (service == NULL)
			{
				xmr_debug("xmr_service_new failed\n");
				break;
			}

			result = xmr_service_get_url_data(service, url, data);
			if (result != 0)
			{
				xmr_debug("xmr_service_get_url_data failed: %d", result);
				break;
			}

			xmr_event_send(window, XMR_EVENT_COVER, data);
		}
		while(0);

		xmr_debug("[END] fetched: %s", url);

		if (service) {
			g_object_unref(service);
		}

		g_free(url);
	} // end of while

	return NULL;
}

static gpointer
thread_login(XmrWindow *window)
{
	XmrWindowPrivate *priv = window->priv;
	gint result;
	gchar *login_message = NULL;

	xmr_debug("[BEGIN] thread_login");
	g_mutex_lock(priv->mutex);
	result = xmr_service_login(priv->service, priv->usr, priv->pwd, &login_message);
	g_mutex_unlock(priv->mutex);

	{
		LoginFinish *l = g_new(LoginFinish, 1);
		l->window = window;
		l->ok = (result == 0);
		l->message = login_message;

		xmr_event_send(window, XMR_EVENT_LOGIN_FINISH, l);
	}

	xmr_debug("[END] thread_login");

	return NULL;
}

static gpointer
thread_logout(XmrWindow *window)
{
	XmrWindowPrivate *priv = window->priv;

	xmr_debug("[BEGIN] thread_logout");

	g_mutex_lock(priv->mutex);
	xmr_service_logout(priv->service);
	g_mutex_unlock(priv->mutex);

	xmr_event_send(window, XMR_EVENT_LOGOUT, NULL);

	xmr_debug("[END] thread_logout");

	return NULL;
}

static gpointer
thread_update_radio_list(XmrWindow *window)
{
	gint style[3] = { Style_FengGe, Style_XingZuo, Style_NianDai };
	gint i;
	XmrService *service;
	XmrDb *db = xmr_db_new();

	xmr_debug("[BEGIN] thread_update_radio_list");
	// use a new service rather than priv->service
	// it will block other data receiving
	service = xmr_service_new();

	// use BEGIN.. COMMIT to speed up writing
	xmr_db_begin(db);

	for(i=0; i<3; ++i)
	{
		GList *list = NULL;
		GList *p;
		gint result;

		result = xmr_service_get_radio_list(service, &list, style[i]);
		if (result != 0)
		{
			xmr_debug("xmr_service_get_radio_list: %d", result);
			continue ;
		}

		p = list;

		while(p)
		{
			RadioInfo *radio_info = (RadioInfo *)p->data;
			GString *data = g_string_new("");

			result = xmr_service_get_url_data(service, radio_info->logo, data);
			if (result != 0)
			{
				xmr_debug("xmr_service_get_url_data: %d", result);
			}
			else
			{
				AppendRadio *append_radio;
				gchar *uri = radio_logo_url_to_uri(radio_info);

				g_free(radio_info->logo);
				radio_info->logo = uri;

				// save cover to local file
				write_memory_to_file(uri, data->str, data->len);

				// save to database
				xmr_db_add_radio(db, radio_info, radio_style[i]);

				append_radio = g_new(AppendRadio, 1);
				append_radio->uri = g_strdup(uri);
				append_radio->idx = i;
				append_radio->info = radio_info_dup(radio_info);

				xmr_event_send(window, XMR_EVENT_APPEND_RADIO, append_radio);
			}

			g_string_free(data, TRUE);
			p = p->next;
		} // end of while(p)

		g_list_free_full(list, (GDestroyNotify)radio_info_free);
	}

	xmr_db_commit(db);

	g_object_unref(service);
	g_object_unref(db);

	xmr_debug("[END] thread_update_radio_list");

	return NULL;
}

static gpointer
thread_like_song(XmrWindow *window)
{
	XmrWindowPrivate *priv = window->priv;
	SongInfo *song;

	xmr_debug("[BEGIN] thread_like_song");

	song = xmr_window_get_current_song(window);
	if (song)
	{
		g_mutex_lock(priv->mutex);
		xmr_service_like_song(priv->service, song->song_id, TRUE);
		g_mutex_unlock(priv->mutex);
	}

	xmr_debug("[END] thread_like_song");

	return NULL;
}

static gpointer
thread_dislike_song(XmrWindow *window)
{
	XmrWindowPrivate *priv = window->priv;
	SongInfo *song;

	xmr_debug("[BEGIN] thread_dislike_song");

	song = xmr_window_get_current_song(window);
	if (song)
	{
		g_mutex_lock(priv->mutex);
		xmr_service_like_song(priv->service, song->song_id, FALSE);
		g_mutex_unlock(priv->mutex);
	}

	xmr_debug("[END] thread_dislike_song");

	return NULL;
}

static void
xmr_window_get_playlist(XmrWindow *window)
{
	if (window && window->priv->queue_fetch_playlist)
	{
		xmr_waiting_wnd_add_task(XMR_WAITING_WND(window->priv->waiting_wnd), INFO_PLAYLIST, WAITING_INFO_PLAYLIST);
		g_async_queue_push(window->priv->queue_fetch_playlist, window);
	}
}

static void
xmr_window_get_cover_image(XmrWindow *window)
{
	if (window && window->priv->queue_fetch_cover)
	{
		if (window->priv->current_song)
		{
			gchar *url = g_strdup(window->priv->current_song->album_cover);
			if (url) {
				g_async_queue_push(window->priv->queue_fetch_cover, url);
			}
		}
	}
}

void
xmr_window_login(XmrWindow *window)
{
	g_return_if_fail(window != NULL);

	xmr_waiting_wnd_add_task(XMR_WAITING_WND(window->priv->waiting_wnd), INFO_LOGIN, WAITING_INFO_LOGIN);
#if GLIB_CHECK_VERSION(2, 32, 0)
	g_thread_new("login", (GThreadFunc)thread_login, window);
#else
	g_thread_create((GThreadFunc)thread_login, window, FALSE, NULL);
#endif
}

void
xmr_window_logout(XmrWindow *window)
{
#if GLIB_CHECK_VERSION(2, 32, 0)
	g_thread_new("logout", (GThreadFunc)thread_logout, window);
#else
	g_thread_create((GThreadFunc)thread_logout, window, FALSE, NULL);
#endif
}

static void
xmr_window_set_track_info(XmrWindow *window)
{
	XmrWindowPrivate *priv = window->priv;
	SongInfo *song = priv->current_song;

	if (song == NULL)
		return ;

	xmr_window_get_cover_image(window);

	xmr_label_set_text(XMR_LABEL(priv->labels[LABEL_SONG_NAME]), song->song_name);
	xmr_label_set_text(XMR_LABEL(priv->labels[LABEL_ARTIST]), song->artist_name);
	xmr_label_set_text(XMR_LABEL(priv->labels[LABEL_TIME]), "00:00/00:00");
	xmr_label_set_text(XMR_LABEL(priv->labels[LABEL_ALBUM]), song->album_name);

	gtk_widget_set_tooltip_text(priv->image, song->album_name);

	// update fav button status
	{
		gint grade = 0;
		if (song->grade)
			grade = (gint)g_strtod(song->grade, NULL);
		if (grade > 0)
			xmr_button_toggle_state_on(XMR_BUTTON(priv->buttons[BUTTON_LIKE]), STATE_FOCUS);
		else
			xmr_button_toggle_state_off(XMR_BUTTON(priv->buttons[BUTTON_LIKE]));
	}
}

static void
update_radio_list(XmrWindow *window)
{
#if GLIB_CHECK_VERSION(2, 32, 0)
	g_thread_new("radio", (GThreadFunc)thread_update_radio_list, window);
#else
	g_thread_create((GThreadFunc)thread_update_radio_list, window, FALSE, NULL);
#endif
}

void
xmr_window_play(XmrWindow *window)
{
	XmrWindowPrivate *priv;

	g_return_if_fail(window != NULL);
	priv = window->priv;

	if (!xmr_player_playing(priv->player))
	{
		// playlist empty
		if (g_list_length(priv->playlist) == 0){
			xmr_window_get_playlist(window);
		}else{
			xmr_player_resume(priv->player);
		}
	}
}

void
xmr_window_play_next(XmrWindow *window)
{
	XmrWindowPrivate *priv;
	gpointer data;
	SongInfo *song;

	g_return_if_fail( window != NULL );
	priv = window->priv;

	// change to default cover image first
	if (priv->pb_cover)
		set_cover_image(window, priv->pb_cover);

	if (g_list_length(priv->playlist) == 0){
		goto no_more_track;
	}

	data = priv->playlist->data;

	// remove current song
	priv->playlist = g_list_remove(priv->playlist, data);
	song_info_free(data);
	song_info_free(priv->current_song);
	priv->current_song = NULL;
	
	g_signal_emit(window, signals[PLAYLIST_CHANGED], 0);

	if (g_list_length(priv->playlist) == 0){
		goto no_more_track;
	}

	song = (SongInfo *)priv->playlist->data;
	priv->current_song = song_info_copy(song);

	// using gstreamer buffering
	if (priv->use_gst_buffering)
	{
		xmr_debug("play next song: %s", song->song_name);
		xmr_player_open(priv->player, song->location, NULL);
		xmr_player_play(priv->player);
	}
	else
	{
		gchar *file = make_track_file(song);
	
		if (is_track_downloaded(song))
		{
			xmr_debug("play next song: %s", file);
			xmr_player_open(priv->player, file, NULL);
			xmr_player_play(priv->player);
		}
		else
		{
			xmr_player_close(priv->player);
			xmr_downloader_add_task(priv->downloader, song->location, file);
			start_buffering_timer(window);
		}

		g_free(file);
	}

	xmr_window_set_track_info(window);
	g_signal_emit(window, signals[TRACK_CHANGED], 0, priv->current_song);

	// get new playlist if only one song in playlist
	if (g_list_length(priv->playlist) > 1)
		return ;

no_more_track:
	xmr_window_get_playlist(window);
}

void
xmr_window_pause(XmrWindow *window)
{
	XmrWindowPrivate *priv;

	g_return_if_fail( window != NULL );
	priv = window->priv;

	xmr_player_pause(priv->player);
	stop_buffering_timer(window);
}

SongInfo *
xmr_window_get_current_song(XmrWindow *window)
{
	g_return_val_if_fail(window != NULL, NULL);

	return window->priv->current_song;
}

static void
create_popup_menu(XmrWindow *window)
{
	XmrWindowPrivate *priv = window->priv;
	GtkWidget *item = NULL;
	GtkAccelGroup* accel_group;
	GtkWidget *menu_radio;
	gint i;

	accel_group = gtk_accel_group_new();
	gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);

	priv->popup_menu = gtk_menu_new();
	priv->skin_menu = gtk_menu_new();

	item = gtk_radio_menu_item_new_with_mnemonic(priv->skin_item_group, _("_Gtk Theme"));
	priv->skin_item_group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM(item));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
	gtk_menu_shell_append(GTK_MENU_SHELL(priv->skin_menu), item);

	g_signal_connect(item, "activate", G_CALLBACK(on_menu_item_activate), window);

	item = gtk_image_menu_item_new_with_mnemonic(_("_Skin"));
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), priv->skin_menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(priv->popup_menu), item);

	menu_radio = gtk_menu_new();

	for(i=0; i<RADIO_COUNT; ++i)
	{
		item = gtk_menu_item_new_with_label(radio_names[i]);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu_radio), item);

		g_signal_connect(item, "activate", G_CALLBACK(on_radio_menu_item_activate), window);
	}

	item = gtk_menu_item_new_with_mnemonic(_("_Radio"));
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), menu_radio);
	gtk_menu_shell_append(GTK_MENU_SHELL(priv->popup_menu), item);
	
	priv->menu_playlist = gtk_menu_new();
	
	item = gtk_menu_item_new_with_mnemonic(_("Playlist"));
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), priv->menu_playlist);
	gtk_menu_shell_append(GTK_MENU_SHELL(priv->popup_menu), item);

	item = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(priv->popup_menu), item);

	item = gtk_check_menu_item_new_with_mnemonic(_("Always on _Top"));
	gtk_menu_shell_append(GTK_MENU_SHELL(priv->popup_menu), item);
	priv->menu_item_ontop = item;
	g_signal_connect(item, "activate", G_CALLBACK(on_menu_item_activate), window);

	item = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(priv->popup_menu), item);

	item = gtk_image_menu_item_new_from_stock(GTK_STOCK_PREFERENCES, accel_group);
	gtk_image_menu_item_set_accel_group(GTK_IMAGE_MENU_ITEM(item), accel_group);
	gtk_menu_shell_append(GTK_MENU_SHELL(priv->popup_menu), item);

	g_signal_connect(item, "activate", G_CALLBACK(on_menu_item_activate), window);

	item = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, accel_group);
	gtk_menu_shell_append(GTK_MENU_SHELL(priv->popup_menu), item);

	g_signal_connect(item, "activate", G_CALLBACK(on_menu_item_activate), window);

	item = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(priv->popup_menu), item);

	priv->menu_login = item = gtk_menu_item_new_with_label(_("Login"));
	gtk_menu_shell_append(GTK_MENU_SHELL(priv->popup_menu), item);
	g_signal_connect(item, "activate", G_CALLBACK(on_menu_item_activate), window);

	priv->menu_logout = item = gtk_menu_item_new_with_label(_("Logout"));
	gtk_menu_shell_append(GTK_MENU_SHELL(priv->popup_menu), item);
	g_signal_connect(item, "activate", G_CALLBACK(on_menu_item_activate), window);

	item = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(priv->popup_menu), item);

	item = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, accel_group);
	gtk_menu_shell_append(GTK_MENU_SHELL(priv->popup_menu), item);

	g_signal_connect(item, "activate", G_CALLBACK(on_menu_item_activate), window);

	gtk_widget_show_all(priv->popup_menu);

	// do not show Logout menu
	gtk_widget_hide(priv->menu_logout);
}

static void
on_menu_item_activate(GtkMenuItem *item, XmrWindow *window)
{
	XmrWindowPrivate *priv = window->priv;
	const gchar *menu = gtk_menu_item_get_label(item);

	if (g_strcmp0(menu, _("_Gtk Theme")) == 0 && gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(item)))
	{
		GtkWidget *box;
		box = GTK_WIDGET(gtk_builder_get_object(priv->ui_pref, "cb_skin"));
		gtk_combo_box_set_active(GTK_COMBO_BOX(box), 0);

		set_gtk_theme(window);
	}
	else if(g_strcmp0(menu, GTK_STOCK_PREFERENCES) == 0)
	{
		GtkWidget *pref_window = NULL;
		pref_window = GTK_WIDGET(gtk_builder_get_object(priv->ui_pref, "window_pref"));
		if (pref_window){
			gtk_widget_show_all(pref_window);
		}
	}
	else if(g_strcmp0(menu, GTK_STOCK_ABOUT) == 0)
	{
		GtkBuilder *builder = NULL;
		static GtkWidget *dialog_about;

		if (dialog_about == NULL)
		{
			builder = create_builder_with_file(UIDIR, "about.ui");
			if (builder == NULL)
			{
				g_warning("Missing about.ui file!\n");
				return ;
			}

			dialog_about = GTK_WIDGET(gtk_builder_get_object(builder, "dialog_about"));

			gtk_window_set_transient_for(GTK_WINDOW(dialog_about), GTK_WINDOW(window));
			gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog_about), VERSION);

			g_object_unref(builder);
		}

		gtk_dialog_run(GTK_DIALOG(dialog_about));
		gtk_widget_hide(dialog_about);
	}
	else if ((GtkWidget *)item == priv->menu_login)
	{
		priv->switch_radio = FALSE;
		do_login(window);
	}
	else if ((GtkWidget *)item == priv->menu_logout)
	{
		xmr_window_logout(window);
	}
	else if(g_strcmp0(menu, GTK_STOCK_QUIT) == 0)
	{
		xmr_window_quit(window);
	}
	else if (GTK_WIDGET(item) == priv->menu_item_ontop)
	{
		gboolean active = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(item));
		gtk_window_set_keep_above(GTK_WINDOW(window), active);
	}
}

static void
on_skin_menu_item_activate(GtkMenuItem *item, SkinInfo *skin)
{
	GtkWidget *box;
	XmrWindow *window = (XmrWindow *)skin->data;

	// only change skin when item was checked
	if (skin == NULL || !gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(item)))
		return ;

	box = GTK_WIDGET(gtk_builder_get_object(window->priv->ui_pref, "cb_skin"));
	gtk_combo_box_set_active_id(GTK_COMBO_BOX(box), skin->file);
}

static void
on_radio_menu_item_activate(GtkMenuItem *item, XmrWindow *window)
{
	gint i;
	const gchar *menu = gtk_menu_item_get_label(item);
	
	// 艺人电台
	if (g_strcmp0(menu, radio_names[RADIO_COUNT-1]) == 0)
	{
		xmr_window_search_radio(window);
		return ;
	}

	for(i=0; i<RADIO_COUNT-1; ++i)
	{
		if (g_strcmp0(menu, radio_names[i]) == 0)
		{
			change_radio_style(window, i);
			break;
		}
	}
}

//static void
//on_playlist_menu_item_activate(GtkMenuItem *item, XmrWindow *window)
//{
	
//}

static void
load_settings(XmrWindow *window)
{
	XmrWindowPrivate *priv = window->priv;
	gchar *radio_name = NULL;
	gchar *radio_url = NULL;
	gint x = -1, y = -1;
	GtkWidget *pref_window;

	pref_window = GTK_WIDGET(gtk_builder_get_object(priv->ui_pref, "window_pref"));
	init_pref_window(window, pref_window);
	gtk_window_set_transient_for(GTK_WINDOW(pref_window), GTK_WINDOW(window));

	// call before load_skin
	priv->skin = xmr_settings_get_theme(priv->settings);

	load_skin(window);
	
	priv->use_gst_buffering = g_settings_get_boolean(G_SETTINGS(priv->settings), "gst-buffering");

	xmr_settings_get_radio(priv->settings, &radio_name, &radio_url);
	if (g_strcmp0(radio_names[0], radio_name) == 0)
		priv->radio_type = Radio_Type_MyRadio;
	else if (g_strcmp0(radio_names[1], radio_name) == 0)
		priv->radio_type = Radio_Type_XMGuess;
	else
		priv->radio_type = Radio_Type_Custom;

	xmr_settings_get_window_pos(priv->settings, &x, &y);
	if (x !=- 1 && y != -1)
		gtk_window_move(GTK_WINDOW(window), x, y);
	
	gtk_window_set_keep_above(GTK_WINDOW(window),
		g_settings_get_boolean(G_SETTINGS(priv->settings), "ontop"));

	xmr_settings_get_usr_info(priv->settings, &priv->usr, &priv->pwd);
	if (xmr_settings_get_auto_login(priv->settings))
	{
		if (priv->usr != NULL && priv->pwd != NULL &&
			*priv->usr != 0 && *priv->pwd != 0)
		{
			if (!radio_url || *radio_url == 0)
				priv->switch_radio = TRUE;
			xmr_window_login(window);
		}
	}
	else
	{
		// load last radio
		if (priv->radio_type == Radio_Type_Custom && radio_url && *radio_url != 0)
		{
			priv->playlist_url = g_strdup(radio_url);
			xmr_label_set_text(XMR_LABEL(priv->labels[LABEL_RADIO]), radio_name);
		}
		else // switch to default radio
		{
			priv->playlist_url = g_strdup(DEFAULT_RADIO_URL);
			priv->radio_type = Radio_Type_Custom;
			xmr_label_set_text(XMR_LABEL(priv->labels[LABEL_RADIO]), DEFAULT_RADIO_NAME);
		}

		xmr_window_get_playlist(window);
	}

	load_radio(window);

	g_free(radio_name);
	g_free(radio_url);
}

static void
load_skin(XmrWindow *window)
{
	XmrWindowPrivate *priv = window->priv;
	gchar *skin_dir;
	
	// load application location
	skin_dir = g_strdup_printf("%s/skin", xmr_app_dir());
	if (skin_dir)
	{
		list_file(skin_dir, FALSE, append_skin, &priv->skin_list);
		g_free(skin_dir);
	}

	// load user config location
	skin_dir = g_strdup_printf("%s/skin", xmr_config_dir());
	if (skin_dir)
	{
		list_file(skin_dir, FALSE, append_skin, &priv->skin_list);
		g_free(skin_dir);
	}

	// load install location
	list_file(SKINDIR, FALSE, append_skin, &priv->skin_list);
	
	// always set gtk theme first
	set_gtk_theme(window);

	if (g_list_length(priv->skin_list) > 0)
	{
		GList *p = priv->skin_list;
		gboolean no_skin_match = TRUE;
		gint idx = 0;
		GtkWidget *box;

		box = GTK_WIDGET(gtk_builder_get_object(priv->ui_pref, "cb_skin"));

		while(p)
		{
			GtkWidget *item;
			SkinInfo *skin_info = (SkinInfo *)p->data;
			skin_info->data = window;
			item = append_skin_to_menu(window, skin_info);
			append_skin_to_pref(window, skin_info);

			if (no_skin_match && g_strcmp0(skin_info->name, priv->skin) == 0)
			{
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
				if (box)
				{
					// the first one always remains Gtk Theme
					gtk_combo_box_set_active(GTK_COMBO_BOX(box), idx + 1);
				}
	
				set_skin(window, skin_info);

				no_skin_match = FALSE;
			}

			p = p->next;
			idx ++;
		}

		if (no_skin_match && !is_text_empty(priv->skin))
		{
			set_skin(window, ((SkinInfo *)priv->skin_list->data));
		}
	}
}

static void
load_radio(XmrWindow *window)
{
	XmrWindowPrivate *priv = window->priv;
	XmrDb *db = xmr_db_new();

	if (xmr_db_is_empty(db))
	{
		update_radio_list(window);
	}
	else
	{
		gint i;

		for(i=0; i<3; ++i)
		{
			GSList *list = NULL;
			GSList *p;

			list = xmr_db_get_radio_list(db, radio_style[i]);
			if (g_slist_length(list) == 0)
				continue ;

			p = list;
			while(p)
			{
				RadioInfo *radio_info = (RadioInfo *)p->data;

				XmrRadio *xmr_radio = xmr_radio_new_with_info(radio_info->logo,
							radio_info->name, radio_info->url);

				xmr_chooser_append(XMR_CHOOSER(priv->chooser[i]), GTK_WIDGET(xmr_radio));

				p = p->next;
			}

			g_slist_free_full(list, (GDestroyNotify)radio_info_free);
		}
	}

	g_object_unref(db);
}

static void
append_skin(const gchar *skin,
			gpointer data)
{
	GList **list = (GList **)data;
	SkinInfo *skin_info = xmr_skin_info_new();

	g_return_if_fail(skin_info != NULL);

	XmrSkin *xmr_skin = xmr_skin_new();

	do
	{
		if (!xmr_skin_load(xmr_skin, skin))
		{
			xmr_debug("Failed to load skin: %s", skin);
			break;
		}

		xmr_skin_get_info(xmr_skin, skin_info);

		*list = g_list_append(*list, skin_info);
	}
	while(0);

	g_object_unref(xmr_skin);
}

static GtkWidget *
append_skin_to_menu(XmrWindow *window, SkinInfo *info)
{
	XmrWindowPrivate *priv = window->priv;
	GtkWidget *item = NULL;

	if (info->name == NULL)
	{
		gchar *name = g_path_get_basename(info->file);
		if (name == NULL)
			return NULL;
		item = gtk_radio_menu_item_new_with_label(priv->skin_item_group, name);
		g_free(name);
	}
	else
	{
		item = gtk_radio_menu_item_new_with_label(priv->skin_item_group, info->name);
	}

	gtk_widget_show(item);
	priv->skin_item_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));

	gtk_menu_shell_append(GTK_MENU_SHELL(priv->skin_menu), item);

	g_signal_connect(item, "activate", G_CALLBACK(on_skin_menu_item_activate), info);

	return item;
}

static void
append_skin_to_pref(XmrWindow *window, SkinInfo *info)
{
	XmrWindowPrivate *priv = window->priv;
	GtkWidget *box = NULL;

	g_return_if_fail(priv->ui_pref != NULL);

	box = GTK_WIDGET(gtk_builder_get_object(priv->ui_pref, "cb_skin"));
	if (box == NULL)
		return ;

	if (info->name == NULL)
	{
		gchar *name = g_path_get_basename(info->file);
		if (name == NULL)
			return ;
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(box), info->file, name);
		g_free(name);
	}
	else
	{
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(box), info->file, info->name);
	}
}

static GtkBuilder *
create_builder_with_file(const gchar *path,
						 const gchar *filename)
{
	GtkBuilder *builder = NULL;
	gchar *filePath;

	builder = gtk_builder_new();
	g_return_val_if_fail(builder != NULL, NULL);
	
	filePath = g_build_filename(xmr_app_dir(), "ui", filename, NULL);

	gtk_builder_set_translation_domain(builder, GETTEXT_PACKAGE);
	
	// always try to load from application dir first
	if (gtk_builder_add_from_file(builder, filePath, NULL) == 0)
	{
		g_free(filePath);
		filePath = g_build_filename(path, filename, NULL);
		gtk_builder_add_from_file(builder, filePath, NULL);
	}

	gtk_builder_connect_signals(builder, NULL);
	
	g_free(filePath);

	return builder;
}

static void
init_pref_window(XmrWindow *window,
			GtkWidget *pref)
{
	XmrWindowPrivate *priv = window->priv;
	GtkWidget *notebook;
	GtkWidget *widget;

	notebook = GTK_WIDGET(gtk_builder_get_object(priv->ui_pref, "nb_pref"));
	g_return_if_fail(notebook != NULL);

	widget = GTK_WIDGET(gtk_builder_get_object(priv->ui_pref, "box_skin"));

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget,
				gtk_label_new(_("Skin")));

	widget = peas_gtk_plugin_manager_new(NULL);

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget,
				gtk_label_new(_("Plugins")));
	
	widget = GTK_WIDGET(gtk_builder_get_object(priv->ui_pref, "box_other"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget,
							 gtk_label_new(_("Other")));
	
	widget = GTK_WIDGET(gtk_builder_get_object(priv->ui_pref, "cbUseGstBuffering"));
	g_settings_bind(G_SETTINGS(priv->settings), "gst-buffering", widget, "active", G_SETTINGS_BIND_DEFAULT);

	widget = GTK_WIDGET(gtk_builder_get_object(priv->ui_pref, "cb_skin"));
	if (widget)
	{
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(widget), "gtk", _("Gtk Theme"));
		gtk_combo_box_set_active(GTK_COMBO_BOX(widget), 0);
		g_signal_connect(widget, "changed",
					G_CALLBACK(on_combo_box_changed), window);
	}
}

static void
init_login_dialog(XmrWindow *window)
{
	XmrWindowPrivate *priv = window->priv;

	GtkWidget *button_login;
	GtkWidget *entry_usr, *entry_pwd;
	GtkWidget *checkbox;

	priv->ui_login = create_builder_with_file(UIDIR, "login.ui");
	g_return_if_fail(priv->ui_login != NULL);

	priv->dialog_login = GTK_WIDGET(gtk_builder_get_object(priv->ui_login, "dialog_login"));
	gtk_window_set_transient_for(GTK_WINDOW(priv->dialog_login), GTK_WINDOW(window));

	button_login = GTK_WIDGET(gtk_builder_get_object(priv->ui_login, "button_login"));

	g_signal_connect(button_login, "button-release-event",
				G_CALLBACK(on_login_dialog_button_login_clicked), window);

	entry_usr = GTK_WIDGET(gtk_builder_get_object(priv->ui_login, "entry_usr"));
	entry_pwd = GTK_WIDGET(gtk_builder_get_object(priv->ui_login, "entry_pwd"));
	checkbox = GTK_WIDGET(gtk_builder_get_object(priv->ui_login, "cb_auto_login"));

	gtk_entry_set_text(GTK_ENTRY(entry_usr), priv->usr);
	gtk_entry_set_text(GTK_ENTRY(entry_pwd), priv->pwd);

	g_settings_bind(G_SETTINGS(priv->settings), "auto-login",
				checkbox, "active",
				G_SETTINGS_BIND_DEFAULT);
}

static void
set_login_dialog_entry_editable(XmrWindow *window,
			const gchar *entry_name,
			gboolean editable)
{
	GtkWidget *entry;

	entry = GTK_WIDGET(gtk_builder_get_object(window->priv->ui_login, entry_name));
	if (entry){
		g_object_set(entry, "editable", editable, NULL);
	}
}

static void
do_login(XmrWindow *window)
{
	XmrWindowPrivate *priv = window->priv;
	GtkWidget *button_login;

	if (priv->dialog_login == NULL){
		init_login_dialog(window);
	}

	button_login = GTK_WIDGET(gtk_builder_get_object(priv->ui_login, "button_login"));
	gtk_button_set_label(GTK_BUTTON(button_login), _("Login"));
	set_login_dialog_entry_editable(window, "entry_usr", TRUE);
	set_login_dialog_entry_editable(window, "entry_pwd", TRUE);

	gtk_dialog_run(GTK_DIALOG(priv->dialog_login));
	gtk_widget_hide(priv->dialog_login);
}
/*
static void
do_logout(XmrWindow *window)
{
	XmrWindowPrivate *priv = window->priv;
	GtkWidget *button_login;

	if (priv->dialog_login == NULL){
		init_login_dialog(window);
	}

	button_login = GTK_WIDGET(gtk_builder_get_object(priv->ui_login, "button_login"));
	gtk_button_set_label(GTK_BUTTON(button_login), _("Logout"));
	set_login_dialog_entry_editable(window, "entry_usr", FALSE);
	set_login_dialog_entry_editable(window, "entry_pwd", FALSE);

	gtk_dialog_run(GTK_DIALOG(priv->dialog_login));
	gtk_widget_hide(priv->dialog_login);
}
*/
static void
change_radio(XmrWindow *window,
			const gchar *name,
			const gchar *url)
{
	XmrWindowPrivate *priv = window->priv;
	const gchar *radio_name = name;

	if (xmr_player_playing(priv->player))
		xmr_window_pause(window);

	g_list_free_full(priv->playlist, (GDestroyNotify)song_info_free);
	priv->playlist = NULL;

	g_free(priv->playlist_url);

	priv->playlist_url = (url == NULL ? NULL : g_strdup(url));

	if (priv->radio_type == Radio_Type_MyRadio)
		radio_name = radio_names[0];
	else if (priv->radio_type == Radio_Type_XMGuess)
		radio_name = radio_names[1];

	xmr_label_set_text(XMR_LABEL(priv->labels[LABEL_RADIO]), radio_name);
	xmr_window_get_playlist(window);

	xmr_settings_set_radio(priv->settings, radio_name, (url == NULL ? "" : url));

	g_signal_emit(window, signals[RADIO_CHANGED], 0, radio_name, url);
}

static gboolean
on_login_dialog_button_login_clicked(GtkButton *button,
			GdkEvent  *event,
			XmrWindow *window)
{
	XmrWindowPrivate *priv = window->priv;

	static GtkWidget *entry_usr, *entry_pwd;

	const gchar *usr, *pwd;

	if (g_strcmp0(gtk_button_get_label(button),
					_("Logout")) == 0)
	{
		xmr_window_logout(window);
		priv->radio_type = Radio_Type_Custom;
		change_radio(window, DEFAULT_RADIO_NAME, DEFAULT_RADIO_URL);

		return FALSE;
	}

	if (entry_usr == NULL)
	{
		entry_usr = GTK_WIDGET(gtk_builder_get_object(priv->ui_login, "entry_usr"));
		entry_pwd = GTK_WIDGET(gtk_builder_get_object(priv->ui_login, "entry_pwd"));
	}

	usr = gtk_entry_get_text(GTK_ENTRY(entry_usr));
	pwd = gtk_entry_get_text(GTK_ENTRY(entry_pwd));

	if (is_text_empty(usr) || is_text_empty(pwd))
	{
		xmr_message(GTK_WIDGET(window),
					_("User name and password may not empty"),
					_("Login"));
		return TRUE;
	}

	g_free(priv->usr);
	g_free(priv->pwd);

	priv->usr = g_strdup(usr);
	priv->pwd = g_strdup(pwd);

	xmr_window_login(window);

	return FALSE;
}

static gboolean
is_text_empty(const gchar *text)
{
	const gchar *p;

	if (text == NULL)
		return TRUE;

	p = text;

	while(p && *p)
	{
		if (*p != ' '){
			return FALSE;
		}
		p++;
	}

	return TRUE;
}

static gchar *
radio_logo_url_to_uri(RadioInfo *radio)
{
	gchar *uri = NULL;
	gchar *name = NULL;

	g_return_val_if_fail(radio != NULL, NULL);

	name = g_path_get_basename(radio->logo);
	uri = g_strdup_printf("%s/%s", xmr_radio_icon_dir(), name);
	g_free(name);

	return uri;
}

static gboolean
on_delete_event(XmrWindow *window,
			GdkEvent *event,
			gpointer data)
{
	xmr_window_quit(window);

	return TRUE;
}

void
xmr_window_quit(XmrWindow *window)
{
	gint x, y;

	gtk_window_get_position(GTK_WINDOW(window), &x, &y);

	xmr_settings_set_window_pos(window->priv->settings, x, y);

	gtk_widget_destroy(GTK_WIDGET(window));
}

static void
on_extension_added(PeasExtensionSet *set,
		    PeasPluginInfo   *info,
		    PeasActivatable  *activatable,
		    gpointer data)
{
	peas_activatable_activate(activatable);
}

static void
on_extension_removed(PeasExtensionSet *set,
		      PeasPluginInfo   *info,
		      PeasActivatable  *activatable,
		      gpointer data)
{
	peas_activatable_deactivate(activatable);
}

static void
on_combo_box_changed(GtkComboBox *combo_box, XmrWindow *window)
{
	XmrWindowPrivate *priv = window->priv;
	gint idx = 0;
	GtkWidget *widget;
	GList *item_list;
	GtkWidget *item = NULL;

	item_list = gtk_container_get_children(GTK_CONTAINER(priv->skin_menu));

	idx= gtk_combo_box_get_active(combo_box);
	item  = g_list_nth_data(item_list, idx);

	if (idx == -1 || idx == 0 || idx - 1 >= g_list_length(priv->skin_list))
	{
		const gchar *label[] =
		{
			"label_version" , "label_author", "label_url", "label_email"
		};
		gint i;
	
		for(i=0; i<4; ++i)
		{
			widget = GTK_WIDGET(gtk_builder_get_object(priv->ui_pref, label[i]));
			if (widget){
				gtk_widget_hide(widget);
			}
		}

		widget = GTK_WIDGET(gtk_builder_get_object(priv->ui_pref, "cb_skin"));
		if (widget){
			gtk_widget_set_tooltip_text(widget, _("Gtk Theme"));
		}
		set_gtk_theme(window);
	}
	else
	{
		SkinInfo *info = g_list_nth_data(priv->skin_list, idx-1);

		if (info == NULL)
			return ;
		widget = GTK_WIDGET(gtk_builder_get_object(priv->ui_pref, "label_version"));
		if (widget)
		{
			gtk_widget_show(widget);
			gtk_label_set_text(GTK_LABEL(widget), info->version);
		}

		widget = GTK_WIDGET(gtk_builder_get_object(priv->ui_pref, "label_author"));
		if (widget)
		{
			gtk_widget_show(widget);
			gtk_label_set_text(GTK_LABEL(widget), info->author);
		}

		widget = GTK_WIDGET(gtk_builder_get_object(priv->ui_pref, "label_url"));
		if (widget)
		{
			gtk_widget_show(widget);
			gtk_button_set_label(GTK_BUTTON(widget), info->url);
			gtk_link_button_set_uri(GTK_LINK_BUTTON(widget), info->url);
		}

		widget = GTK_WIDGET(gtk_builder_get_object(priv->ui_pref, "label_email"));
		if (widget)
		{
			gtk_widget_show(widget);
			gtk_button_set_label(GTK_BUTTON(widget), info->email);
			gtk_link_button_set_uri(GTK_LINK_BUTTON(widget), info->email);
		}

		widget = GTK_WIDGET(gtk_builder_get_object(priv->ui_pref, "cb_skin"));
		if (widget){
			gtk_widget_set_tooltip_text(widget, info->file);
		}

		set_skin(window, info);
	}

	if (item){
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
	}

	g_list_free(item_list);
}

gboolean
xmr_window_playing(XmrWindow *window)
{
	g_return_val_if_fail(window != NULL, FALSE);

	return xmr_player_playing(window->priv->player);
}

static void
like_current_song(XmrWindow *window, gboolean like)
{
	XmrWindowPrivate *priv = window->priv;

	if (!xmr_service_is_logged_in(priv->service))
	{
		do_login(window);
		return ;
	}

#if GLIB_CHECK_VERSION(2, 32, 0)
	g_thread_new("like", (GThreadFunc)(like ? thread_like_song : thread_dislike_song), window);
#else
	g_thread_create((GThreadFunc)(like ? thread_like_song : thread_dislike_song), window, FALSE, NULL);
#endif

	if (!like){
	  xmr_window_play_next(window);
	}
}

void
xmr_window_love(XmrWindow *window)
{
	g_return_if_fail(window != NULL);
	xmr_button_toggle_state_on(XMR_BUTTON(window->priv->buttons[BUTTON_LIKE]), STATE_FOCUS);
	like_current_song(window, TRUE);
}

void
xmr_window_hate(XmrWindow *window)
{
	g_return_if_fail(window != NULL);

	xmr_button_toggle_state_off(XMR_BUTTON(window->priv->buttons[BUTTON_LIKE]));
	like_current_song(window, FALSE);
}

static void
change_radio_style(XmrWindow *window,
			gint new_style)
{
	XmrWindowPrivate *priv = window->priv;
	gint i;

	for(i=0; i<3; ++i){
		gtk_widget_hide(priv->chooser[i]);
	}

	if (new_style == 0 || new_style == 1) // SiRen radio
	{
		if (!xmr_service_is_logged_in(priv->service))
		{
			priv->switch_radio = TRUE;
			priv->radio_type = new_style;
			do_login(window);
		}
		else
		{
			// to avoid change radio from siren to siren
			if (priv->radio_type != new_style)
			{
				priv->radio_type = new_style;
				change_radio(window, NULL, NULL);
			}
		}
	}
	else
	{
		gtk_widget_show_all(priv->chooser[new_style - 2]);
	}
}

static void
on_volume_button_value_changed(GtkScaleButton *button,
			gdouble value,
			XmrWindow *window)
{
	if (!window->priv->syncing_volume) {
		xmr_player_set_volume(window->priv->player, value);
	}	
}

void
xmr_window_set_volume(XmrWindow *window,
			float value)
{
	g_return_if_fail(window != NULL);
	xmr_player_set_volume(window->priv->player, value);
}

static gboolean
test_conflict(GList *list, SongInfo *info)
{
	GList *p = list;
	
	while (p)
	{
		SongInfo *si = (SongInfo *)p->data;

		if (g_strcmp0(info->song_name, si->song_name) == 0 &&
			g_strcmp0(info->artist_name, si->artist_name) == 0 &&
			g_strcmp0(info->album_name, si->album_name) == 0)
		{
			return TRUE;
		}
		p = p->next;
	}
	
	return FALSE;
}

gboolean
xmr_window_add_song(XmrWindow *window,
					SongInfo *info)
{
	XmrWindowPrivate *priv;

	g_return_val_if_fail(window != NULL && info != NULL, FALSE);
	priv = window->priv;
	
	if (!test_conflict(priv->playlist, info))
	{
		priv->playlist = g_list_append(priv->playlist, song_info_copy(info));
		g_signal_emit(window, signals[PLAYLIST_CHANGED], 0);
		
		return TRUE;
	}
	
	return FALSE;
}

gboolean
xmr_window_add_song_and_play(XmrWindow *window,
							 SongInfo *info)
{
	XmrWindowPrivate *priv;

	g_return_val_if_fail(window != NULL && info != NULL, FALSE);
	priv = window->priv;
	
	if (!test_conflict(priv->playlist, info))
	{
		xmr_window_pause(window);
		
		// insert into second place
		priv->playlist = g_list_insert(priv->playlist, song_info_copy(info), 1);
		xmr_window_play_next(window);
		
		return TRUE;
	}
	
	return FALSE;
}

void
xmr_window_set_search_result(XmrWindow *window,
							 GList *list,
							 const gchar *from,
							 const gchar *link)
{
	XmrWindowPrivate *priv;

	g_return_if_fail(window != NULL && list != NULL);
	priv = window->priv;
	
	if (priv->xmr_searchlist == NULL)
		priv->xmr_searchlist = xmr_list_new(XMR_SEARCH_LIST, GTK_WINDOW(window));
	
	xmr_list_append(XMR_LIST(priv->xmr_searchlist), list, from, link);
	gtk_widget_show_all(priv->xmr_searchlist);
}

gboolean
xmr_window_is_current_song_marked(XmrWindow *window)
{
	g_return_val_if_fail (window != NULL, FALSE);
	
	return xmr_button_is_toggled(XMR_BUTTON(window->priv->buttons[BUTTON_LIKE]));
}

void
xmr_window_search_radio(XmrWindow *window)
{
	XmrWindowPrivate *priv;
	g_return_if_fail(window != NULL);
	priv = window->priv;
	
	if (priv->radio_search_box == NULL)
		priv->radio_search_box = xmr_search_box_new(GTK_WINDOW(window));
	
	xmr_search_box_show(XMR_SEARCH_BOX(priv->radio_search_box));
}

void
xmr_window_play_custom_radio(XmrWindow *window,
							 const gchar *name,
							 const gchar *url)
{
	g_return_if_fail(window != NULL);

	window->priv->radio_type = Radio_Type_Custom;
	change_radio(window, name, url);
}

void
xmr_window_increase_search_music_count(XmrWindow *window)
{
	XmrWindowPrivate *priv;
	g_return_if_fail(window != NULL);
	priv = window->priv;
	
	priv->search_music_count++;
	if (priv->search_music_count == 1)
	{
		priv->search_music_idle_id = g_timeout_add_full(
					G_PRIORITY_DEFAULT, 100,
					(GSourceFunc)search_music_progress_idle, priv->search_box,
					(GDestroyNotify)search_music_progress_done);
	}
}

void
xmr_window_decrease_search_music_count(XmrWindow *window)
{
	XmrWindowPrivate *priv;
	g_return_if_fail(window != NULL);
	priv = window->priv;
	
	priv->search_music_count--;
	if (priv->search_music_count == 0)
	{
		g_source_remove(priv->search_music_idle_id);
		priv->search_music_idle_id = 0;
	}
}

static gboolean
show_message_idle(XmrWindow *window)
{
	XmrWindowPrivate *priv = window->priv;

	xmr_message(GTK_WIDGET(window), priv->message.message, priv->message.title);

	g_free(priv->message.message);
	g_free(priv->message.title);

	priv->message.message = NULL;
	priv->message.title = NULL;

	return FALSE;
}

static void
login_finish(XmrWindow *window,
			 gboolean ok,
			 const gchar *message,
			 gpointer data)
{
	XmrWindowPrivate *priv = window->priv;
	if (!ok)
	{
		gchar *error_message;

		error_message = g_strdup_printf(_("Login failed:\n%s"), message);

		priv->message.message = g_strdup(error_message);
		priv->message.title = g_strdup(_("Login Status"));
		g_idle_add((GSourceFunc)show_message_idle, window);

		if (g_list_length(priv->playlist) == 0)
		{
			if (priv->radio_type != Radio_Type_Custom) {
				change_radio(window, DEFAULT_RADIO_NAME, DEFAULT_RADIO_URL);
			}
		}

		g_free(error_message);

		gtk_widget_hide(priv->menu_logout);
		gtk_widget_show(priv->menu_login);
	}
	else
	{
		xmr_settings_set_usr_info(priv->settings, priv->usr, priv->pwd);
		// switch to siren radio
		if (priv->switch_radio)
		{
			change_radio(window, NULL, NULL);
			priv->switch_radio = FALSE;
		}
		else
		{
			gchar *radio_name = NULL;
			gchar *radio_url = NULL;

			xmr_settings_get_radio(priv->settings, &radio_name, &radio_url);
			if (g_strcmp0(priv->playlist_url, radio_url) != 0)
			{
				if (radio_url && *radio_url != 0)
					priv->radio_type = Radio_Type_Custom;
				else if (g_strcmp0(radio_name, radio_names[0]) == 0)
					priv->radio_type = Radio_Type_MyRadio;
				else if (g_strcmp0(radio_name, radio_names[1]) == 0)
					priv->radio_type = Radio_Type_XMGuess;
				change_radio(window, radio_name, radio_url);
			}
			g_free(radio_name);
			g_free(radio_url);
		}

		gtk_widget_hide(priv->menu_login);
		gtk_widget_show(priv->menu_logout);
	}

	xmr_debug("login status: %s", message);
}

static void
on_logout(XmrWindow *window)
{
	gtk_widget_hide(window->priv->menu_logout);
	gtk_widget_show(window->priv->menu_login);

	if (window->priv->radio_type != Radio_Type_Custom)
	{
		window->priv->radio_type = Radio_Type_Custom;
		change_radio(window, DEFAULT_RADIO_NAME, DEFAULT_RADIO_URL);
	}
}

static void
fetch_playlist_finish(XmrWindow *window,
					  gint status,
					  GList *list,
					  gpointer data)
{
	XmrWindowPrivate *priv = window->priv;
	gboolean auto_play = g_list_length(priv->playlist) == 0;

	if (list != NULL)
	{
		priv->playlist = g_list_concat(priv->playlist, list);
		g_signal_emit(window, signals[PLAYLIST_CHANGED], 0);
	}

	if (status != 0)
	{
		xmr_debug("failed to get track list: %d (%s)", status, xmr_service_get_error_str(status));
	}
	else if (auto_play)
	{
		if (g_list_length(priv->playlist) > 0)
		{
			SongInfo *song;

			song = priv->playlist->data;
			song_info_free(priv->current_song);
			priv->current_song = song_info_copy(song);

			if (priv->use_gst_buffering)
			{
				xmr_debug("play song: %s", song->song_name);
				xmr_player_open(priv->player, song->location, NULL);
				xmr_player_play(priv->player);
				
				xmr_window_set_track_info(window);
				g_signal_emit(window, signals[TRACK_CHANGED], 0, priv->current_song);
			}
			else
			{
				gchar *file = make_track_file(song);
	
				xmr_downloader_add_task(priv->downloader, song->location, file);
				g_free(file);
	
				start_buffering_timer(window);
			}
		}
	}
}

static void
fetch_cover_finish(XmrWindow *window,
				   GdkPixbuf *pixbuf,
				   gpointer data)
{
	g_object_ref(pixbuf);

	window->priv->pb_cover_current = pixbuf;
	set_cover_image(window, pixbuf);

	g_object_unref(pixbuf);
}

static void
download_finish(XmrDownloader *downloader,
				const gchar *url,
				const gchar *file,
				XmrWindow *window)
{
	XmrWindowPrivate *priv = window->priv;

	xmr_debug("%s -> %s downloaded", url, file);

	// download next song
	{
		GList *p = priv->playlist;
		gboolean download_flag = FALSE;

		while(p)
		{
			SongInfo *song = p->data;
			gchar *track_file = make_track_file(song);

			if (strcmp(file, track_file) == 0 &&
				strcmp(song->location, url) == 0)
			{
				g_free(song->location);
				song->location = g_strdup(file);
				
				if (strcmp(priv->current_song->location, url) == 0 &&
					strcmp(priv->current_song->song_id, song->song_id) == 0)
				{
					g_free(priv->current_song->location);
					priv->current_song->location = g_strdup(file);
				}
			}

			if (!download_flag && !is_file_exists(track_file))
			{
				xmr_downloader_add_task(priv->downloader, song->location, track_file);
				download_flag = TRUE;
			}

			g_free(track_file);

			p = p->next;
		}
	}
}

static void
download_progress(XmrDownloader *downloader,
				  const gchar *url,
				  double progress,
				  XmrWindow *window)
{
	// xmr_debug("Downloading %s - %.2lf%%", url, progress);
}

static void
download_failed(XmrDownloader *downloader,
				const gchar *url,
				const gchar *message,
				XmrWindow *window)
{
	xmr_debug("Failed to download [%s]:\n%s", url, message);
	if (!xmr_window_playing(window))
		xmr_window_play_next(window);
}

static gboolean
str_start_with(const char *str, const char *prefix)
{
	size_t lenpre = strlen(prefix);
	size_t lenstr = strlen(str);
	return lenstr < lenpre ? FALSE : strncasecmp(prefix, str, lenpre) == 0;
}

static gboolean
is_track_downloaded(SongInfo *track)
{
	gboolean retv = FALSE;
	gchar *file = make_track_file(track);
	if (!file) {
		return FALSE;
	}
	
	retv = is_file_exists(file) && !str_start_with(track->location, "http://");
	
	g_free(file);
	
	return retv;
}

static gboolean
is_file_exists(const gchar *file)
{
	return g_file_test(file, G_FILE_TEST_EXISTS);
}

static gchar *
make_track_file(SongInfo *track)
{
	gchar *file = NULL;
	gchar *name = NULL;
	gchar *lname = NULL;

	if (track == NULL)
		return NULL;

	lname = g_path_get_basename(track->location);

	if (track->artist_name == NULL)
	{
		if (track->song_name == NULL)
			name = g_strdup_printf("%s", lname);
		else
			name = g_strdup_printf("%s.mp3", track->song_name);
	}
	else
	{
		if (track->song_name == NULL)
			name = g_strdup_printf("%s", lname);
		else
			name = g_strdup_printf("%s - %s.mp3", track->artist_name, track->song_name);
	}

	g_free(lname);

	// FIXME: should consider more condition
	// just replace '/' to ' '
	{
		gchar *p = name;
		while (p && *p)
		{
			if (*p == '/')
				*p = ' ';
			p++;
		}
	}

	file = g_strdup_printf("%s/%s", xmr_tmp_dir(), name);

	g_free(name);

	return file;
}

static void
xmr_event_send(XmrWindow *window, guint type, gpointer data)
{
	XmrEvent *e = g_new(XmrEvent, 1);
	e->type = type;
	e->data = data;

	g_async_queue_push(window->priv->queue_event, e);
	g_main_context_wakeup(g_main_context_default());
}

static gboolean
xmr_event_poll(XmrWindow *window)
{
	XmrWindowPrivate *priv = window->priv;

	XmrEvent *event = g_async_queue_try_pop(priv->queue_event);
	if (event == NULL)
		return TRUE;

	switch (event->type)
	{
	case XMR_EVENT_LOGIN_FINISH:
		{
			LoginFinish *l = (LoginFinish *)event->data;
			g_signal_emit(window, signals[LOGIN_FINISH], 0, l->ok, l->message);

			g_free(l->message);
			xmr_waiting_wnd_next_task(XMR_WAITING_WND(priv->waiting_wnd), INFO_LOGIN);
		}
		break;

	case XMR_EVENT_LOGOUT:
		g_signal_emit(window, signals[LOGOUT], 0);
		break;

	case XMR_EVENT_PLAYLIST:
		{
			FetchPlaylist *p = (FetchPlaylist *)event->data;
			g_signal_emit(window, signals[FETCH_PLAYLIST_FINISH], 0, p->result, p->list);
			xmr_waiting_wnd_next_task(XMR_WAITING_WND(priv->waiting_wnd), INFO_PLAYLIST);
		}
		break;

	case XMR_EVENT_COVER:
		{
			GdkPixbuf *pixbuf;
			GString *data;

			data = (GString *)event->data;

			pixbuf = gdk_pixbuf_from_memory(data->str, data->len);
			if (pixbuf == NULL)
			{
				xmr_debug("gdk_pixbuf_from_memory failed");
				break;
			}

			g_signal_emit(window, signals[FETCH_COVER_FINISH], 0, pixbuf);
	
			g_string_free(data, TRUE);
			g_object_unref(pixbuf);
		}
		break;

	case XMR_EVENT_APPEND_RADIO:
		{
			XmrRadio *xmr_radio;
			AppendRadio *radio = (AppendRadio *)event->data;

			if (radio == NULL)
				break;

			xmr_radio = xmr_radio_new_with_info(radio->uri, radio->info->name, radio->info->url);
			xmr_chooser_append(XMR_CHOOSER(priv->chooser[radio->idx]), GTK_WIDGET(xmr_radio));

			g_free(radio->uri);
			radio_info_free(radio->info);
		}
		break;

	case XMR_EVENT_PLAYER_EOS:
		xmr_window_play_next(window);
		break;

	case XMR_EVENT_PLAYER_STATE_CHANGED:
		{
			gint new_state = GPOINTER_TO_INT(event->data);
			if (new_state == GST_STATE_PLAYING)
			{
				gtk_widget_hide(priv->buttons[BUTTON_PLAY]);
				gtk_widget_show(priv->buttons[BUTTON_PAUSE]);
			}
			else if(new_state == GST_STATE_PAUSED)
			{
				gtk_widget_hide(priv->buttons[BUTTON_PAUSE]);
				gtk_widget_show(priv->buttons[BUTTON_PLAY]);
			}
			event->data = NULL; // Do not free!!!
		}
		break;

	case XMR_EVENT_PLAYER_TICK:
		{
			gchar *time = (gchar *)event->data;
			xmr_label_set_text(XMR_LABEL(priv->labels[LABEL_TIME]), time);
		}
		break;
		
	case XMR_EVENT_PLAYER_BUFFERING:
		{
			gint progress = GPOINTER_TO_INT(event->data);
			if (progress == 100)
				xmr_waiting_wnd_next_task(XMR_WAITING_WND(priv->waiting_wnd), INFO_BUFFERING);
			else
				xmr_waiting_wnd_add_task(XMR_WAITING_WND(priv->waiting_wnd), INFO_BUFFERING, WAITING_INFO_BUFFERING);
			xmr_debug("buffering (%d)...", progress);
			event->data = NULL;
		}
		break;
	}

	if (event->type != XMR_EVENT_COVER)
		g_free(event->data);
	g_free(event);

	return TRUE;
}

static gboolean
buffering_timeout(XmrWindow *window)
{
	XmrWindowPrivate *priv = window->priv;

	if (priv->use_gst_buffering)
		return FALSE;

	if (!xmr_window_playing(window))
	{
		if (priv->current_song != NULL && is_track_downloaded(priv->current_song))
		{
			gchar *file = make_track_file(priv->current_song);
			
			xmr_waiting_wnd_next_task(XMR_WAITING_WND(priv->waiting_wnd), INFO_BUFFERING);
			
			xmr_debug("buffering ok...");
			xmr_debug("play next song: %s", file);

			xmr_player_open(priv->player, file, NULL);
			xmr_player_play(priv->player);

			g_free(file);

			xmr_window_set_track_info(window);
			g_signal_emit(window, signals[TRACK_CHANGED], 0, priv->current_song);
			
			priv->buffering_timer = 0;
			return FALSE;
		}
		else
		{
			xmr_waiting_wnd_add_task(XMR_WAITING_WND(priv->waiting_wnd), INFO_BUFFERING, WAITING_INFO_BUFFERING);

			xmr_debug("buffering...");
		}
	}

	return TRUE;
}

static void
start_buffering_timer(XmrWindow *window)
{
	if (window->priv->buffering_timer == 0)
	{
		window->priv->buffering_timer = g_timeout_add(500, (GSourceFunc)buffering_timeout, window);
	}
}

static void
stop_buffering_timer(XmrWindow *window)
{
	if (window->priv->buffering_timer != 0)
	{
		g_source_remove(window->priv->buffering_timer);
		window->priv->buffering_timer = 0;
	}
}

static void
on_search_box_activate(GtkEntry  *entry,
					   XmrWindow *window)
{
	const gchar *text;

	if (gtk_entry_get_text_length(entry) == 0)
		return ;

	text = gtk_entry_get_text(entry);
	
	xmr_debug("search music: %s", text);
	
	g_signal_emit(window, signals[SEARCH_MUSIC], 0, text);
}

static gboolean
on_search_box_focus_in(GtkWidget *widget,
					   GdkEvent  *event,
					   gpointer   data)
{
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_xpm_data((const gchar **)icon_enter_xpm);
	
	gtk_entry_set_icon_from_pixbuf(GTK_ENTRY(widget), GTK_ENTRY_ICON_SECONDARY, pixbuf);
	
	g_object_unref(pixbuf);

	return FALSE;
}

static gboolean
on_search_box_focus_out(GtkWidget *widget,
					   GdkEvent  *event,
					   gpointer   data)
{
	gtk_entry_set_icon_from_stock(GTK_ENTRY(widget), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_FIND);

	return FALSE;
}

static gint
get_search_box_font_width(GtkWidget *search_box)
{
	GtkStyleContext *ctx = gtk_widget_get_style_context(search_box);
	PangoFontDescription *desc = NULL;
	gtk_style_context_get(ctx, GTK_STATE_FLAG_NORMAL, "font", &desc, NULL);
	gint size = pango_font_description_get_size(desc);
	if (PANGO_SCALE != 0)
	{
		if (!pango_font_description_get_size_is_absolute(desc))
			size /= PANGO_SCALE;
	}
	if (size == 0)
		size = 11;

	return size;
}

static void
update_playlist_menu_items(XmrWindow *window)
{
	XmrWindowPrivate *priv = window->priv;
	GList *p;
	gboolean is_first = TRUE;
	
	// remove all items
	{
		GList* items = gtk_container_get_children(GTK_CONTAINER(priv->menu_playlist));
		while(g_list_length(items) > 0)
		{
			GtkWidget *item = g_list_nth_data(items, 0);
			items = g_list_remove(items, item);
			
			// g_signal_handlers_disconnect_by_func(item, on_playlist_menu_item_activate, window);
			gtk_widget_destroy(item);
		}
	}

	p = priv->playlist;
	while (p)
	{
		GtkWidget *item;
		SongInfo *info = (SongInfo *)p->data;
		gchar *label = g_strdup_printf("%s - %s", info->artist_name, info->song_name);
		item = gtk_check_menu_item_new_with_label(label);
		g_free(label);
		
		if (is_first)
		{
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
			is_first = FALSE;
		}
		gtk_menu_shell_append(GTK_MENU_SHELL(priv->menu_playlist), item);

		// g_signal_connect(item, "activate", G_CALLBACK(on_playlist_menu_item_activate), window);
		// FIXME
		// Should user only see that playlist or can user select one to play?
		gtk_widget_set_sensitive(item, FALSE);
		gtk_widget_show(item);
		p = p->next;
	}
}

static void
on_playlist_changed(XmrWindow *window, gpointer data)
{
	update_playlist_menu_items(window);
}

static void
on_settings_changed(GSettings *settings,
					const char *key,
					XmrWindow *window)
{
	if (g_strcmp0(key, "gst-buffering") == 0)
	{
		window->priv->use_gst_buffering = g_settings_get_boolean(settings, key);
	}
	else if (g_strcmp0(key, "ontop") == 0)
	{
		gboolean active = g_settings_get_boolean(settings, key);
		gtk_window_set_keep_above(GTK_WINDOW(window), active);
	}
}

static gboolean
search_music_progress_idle(GtkEntry *entry)
{
	gtk_entry_progress_pulse(entry);

	return TRUE;
}

static void
search_music_progress_done(GtkEntry *entry)
{
	gtk_entry_set_progress_fraction(entry, 0.0);
}
