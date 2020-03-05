/**
    @file appdata.h

    Data structures for the whole application.

    Copyright (c) 2004-2006 Nokia Corporation.
	
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
    02111-1307 USA
*/


#ifndef APPDATA_H
#define APPDATA_H


#include <aconf.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gconf/gconf-client.h>
#include <gio/gio.h>
#include <hildon/hildon.h>
#include <hildon/hildon-file-system-model.h>
#include <libosso.h>
#include <osso-log.h>

#include <comapp_system.h>

typedef struct _AppData AppData;
typedef struct _AppUIData AppUIData;

typedef struct Arrow_Cords {
    gint x_pos_left;
    gint x_pos_right;
    gint y_pos;
} Arrow_Cords;

typedef enum {
    PDF_VIEWER_STATE_EMPTY,
    PDF_VIEWER_STATE_LOADING,
    PDF_VIEWER_STATE_LOADED,
    PDF_VIEWER_STATE_SAVING
} PDFViewerState;

typedef enum {
    STARTUP_MODE_DEFAULT,
    STARTUP_MODE_URI_REQUEST,
    STARTUP_MODE_STATE_DATA
} PDFViewerStartupMode;

/* Application data */
struct _AppData {
    AppUIData *app_ui_data;
    ComappSystemData *comapp_system;

    // XXX: TODO
    //GnomeVFSVolumeMonitor *volume_monitor;
    gchar *mmc_uri;

    PDFViewerState state;
    PDFViewerStartupMode mode;

    gboolean low_memory;
};


/* FLAGS */
#define PDF_FLAGS_SET(a,b)    ((a) |= (b))
#define PDF_FLAGS_UNSET(a,b)  ((a) &= ~(b))
#define PDF_FLAGS_TOGGLE(a,b) ((a) ^= (b))
#define PDF_FLAGS_IS_SET(a,b) ((a) & (b))

typedef enum {
    PDF_FLAGS_NONE = 0,
    PDF_FLAGS_RENDERING = 1,
    PDF_FLAGS_FULLSCREEN = 1 << 1,
    PDF_FLAGS_BUTTON_DOWN = 1 << 2,
    PDF_FLAGS_PANNING = 1 << 3,
    PDF_FLAGS_SHOW_IMAGES = 1 << 4,
    PDF_FLAGS_SELECT_KEY_ALLOWED = 1 << 5,
    PDF_FLAGS_BACKGROUNDED = 1 << 6,
    PDF_FLAGS_PAGE_ERROR = 1 << 7
} PDFFlags;

/* Application UI data */
struct _AppUIData {
    /* Pointer back to AppData (parent structure) */
    AppData *app_data;

    /* View items */
    HildonProgram *app;
    HildonWindow *app_view;

    /* UI Manager */
    GtkUIManager *ui_manager;

    /* Actions are available through ui manager, toolbar and menu through
     * appview */

    /* Toolbar widget */
    GtkToolbar *toolbar;

    /* Menu widget */
    HildonAppMenu *menu;
    GtkWidget *menu_open;
    GtkWidget *menu_save;
    GtkWidget *menu_details;
    GtkWidget *menu_single_page_continuous;

    /* Toolbar label widgets */
    GtkAction *current_page;
    //GtkWidget *total_pages;
    GtkWidget *current_page_item;
    GtkWidget *current_page_button;
    GtkWidget *current_zoom_item;
    GtkWidget *current_zoom_value;

    GtkWidget *img_images_on;
    GtkWidget *img_images_off;

    /* Application area widgets */
    GtkWidget *scrolled_window;
    GtkWidget *layout;
    GtkWidget *page_image;
    GdkPixbuf *left_image;
    GdkPixbuf *right_image;
    GdkPixbuf *left_image_orig;
    GdkPixbuf *right_image_orig;
 
    GdkPixbuf *ovr_image;
    GdkPixbuf *ovr_image_orig;
    gboolean fullscreen;
    gboolean ovrshown;
    Arrow_Cords last_ovr_cord;
 
    /* scrollbar widgets */
    GtkWidget *hscroll;
    GtkWidget *vscroll;
   
    /* fullscreen overlay and fullscreen toolbar widgets */
    GtkWidget *fullscreen_overlay_button;
    GtkWidget *fullscreen_toolbar_button;
     
    /* Details dialog, to close if it is needed */
    GtkWidget *details_dialog;

    /* Switch to page dialog */
    GtkWidget *switch_page_dialog;

    /* Open-save dialog */
    GtkWidget *opensave_dialog;

    /* Note-dialog for Corrupted, Unsupported format and Out of memory*/
    GtkWidget *note_dialog;

    /* password dialog */	
     GtkWidget *password_dialog;

     /* replace dialog */	
     GtkWidget *replace_dialog;	

    /* The last pointer position where it was updated */
    gint lastx;
    gint lasty;

    gint press_lastx;
    gint press_lasty;

    /* Current DPI */
    gdouble dpi;

    /* TRUE, if closing the app is needed */
    gboolean close_called;

    /* TRUE while rendering
    gboolean rendering; */

    /* TRUE while we copy the file from the gw */
    gboolean copy_from_gw;

    gpointer open_document_structure;

    GtkWidget *saving_banner;
    GtkWidget *show_images_banner;
    GtkWidget *hide_images_banner;
    //GtkWidget *zooming_banner;
    GtkWidget *opening_banner;

    guint arrow_put_id;
    guint arrow_hide_id;
    guint arrow_right_id;
    guint arrow_left_id;
    
    Arrow_Cords last_arrow_cord;

    int /* PDFFlags */ flags;
    
    /* This is used to workaround other bugs */
    guint key_pressed;

    gboolean save_dialog_opened;

    gchar *last_uri;

    /* Portrait mode */
    gboolean isPortrait;
};

#endif /* APPDATA_H */
