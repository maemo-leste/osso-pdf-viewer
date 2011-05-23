/**
    @file ui.h

    Application user interface definitions.

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


#ifndef UI_H
#define UI_H


#include <gtk/gtk.h>

#include "callbacks.h"


/* Toolbar position for current page widget and total pages widget */
#define TOOLBAR_POS_CURRENT_PAGE_WIDGET 2
#define TOOLBAR_POS_CURRENT_ZOOM_WIDGET 6
#define TOOLBAR_POS_FULLSCREEN          8

/* Custom action names */
#define ACTION_PAGE_SWITCH_TO "pdfv_me_menu_page_switch_to"

/* Custom action paths */
#define ACTION_PATH_FULL_SCREEN "/ToolBar/pdfv_me_menu_screen_full_screen"

/* Use empty accelerator "" for stock action entries to disable stock
 * accelerator */
static GtkActionEntry action_entries[] = {

    /* Document menu */
    {"pdfv_me_menu_document_details", GTK_STOCK_PROPERTIES,
     "pdfv_me_menu_document_details", "", NULL,
     G_CALLBACK(on_document_details)},

    /* Page menu */
    /* No stock icon available for previous, next? */

    {"pdfv_me_menu_page_previous", "general_back",
     "pdfv_me_menu_page_previous", NULL, NULL,
     G_CALLBACK(on_page_previous)},

    {"pdfv_me_menu_page_next", "general_forward",
     "pdfv_me_menu_page_next", NULL, NULL,
     G_CALLBACK(on_page_next)},

    {"pdfv_me_menu_page_first", "pdf_viewer_first_page",
     "pdfv_me_menu_page_first", "", NULL,
     G_CALLBACK(on_page_first)},

    {"pdfv_me_menu_page_last", "pdf_viewer_last_page",
     "pdfv_me_menu_page_last", "", NULL,
     G_CALLBACK(on_page_last)},
    /* Screen menu, Zoom submenu */

    {"pdfv_me_menu_screen_zoom_in", "pdf_zoomin",
     "pdfv_me_menu_screen_zoom_in", "", NULL,
     G_CALLBACK(on_screen_zoom_in)},

    {"pdfv_me_menu_screen_zoom_out", "pdf_zoomout",
     "pdfv_me_menu_screen_zoom_out", "", NULL,
     G_CALLBACK(on_screen_zoom_out)},

    {"pdfv_me_menu_screen_zoom_page", GTK_STOCK_ZOOM_FIT,
     "pdfv_me_menu_screen_zoom_page", "", NULL,
     G_CALLBACK(on_screen_zoom_page)},

    {"pdfv_me_menu_screen_zoom_width", GTK_STOCK_ZOOM_100,
     "pdfv_me_menu_screen_zoom_width", "", NULL,
     G_CALLBACK(on_screen_zoom_width)},
};
static guint n_action_entries = G_N_ELEMENTS(action_entries);


static GtkToggleActionEntry toggle_action_entries[] = {
    {"pdfv_me_menu_single_page_continuous", "pdf_viewer_single_page_continuous",
     "pdfv_me_menu_single_page_continuous", "", NULL,
     G_CALLBACK(display_single_page_continuous),
     TRUE},

    {"pdfv_me_menu_screen_full_screen", "general_fullsize", 
     "pdfv_me_menu_screen_full_screen", NULL, NULL,
     G_CALLBACK(on_screen_full_screen),
     FALSE},

    {"pdfv_me_menu_screen_show_images","general_image",
     "pdfv_me_menu_screen_show_images", NULL, NULL,
     G_CALLBACK(on_screen_show_images),
     TRUE},
};
static guint n_toggle_action_entries = G_N_ELEMENTS(toggle_action_entries);


/* added separators between all toolbar buttons Expand is enabled
 * programmatically since expand='true' does not work in current Gtk version. 
 * See build_toolbar() in interface.c. See also P12669-T-496 / Bugzilla
 * 18292. */
static const gchar *ui_info = 
    "<ui>" 
    "  <toolbar name='ToolBar'>" 
    "    <toolitem action='pdfv_me_menu_page_first'/>" 
    "    <toolitem action='pdfv_me_menu_page_previous'/>" 
    "    <toolitem action='pdfv_me_menu_page_next'/>" 
    "    <toolitem action='pdfv_me_menu_page_last'/>" 
    "    <toolitem action='pdfv_me_menu_single_page_continuous'/>" 
    "    <toolitem action='pdfv_me_menu_screen_zoom_out'/>" 
    "    <toolitem action='pdfv_me_menu_screen_zoom_in'/>" 
    "    <toolitem action='pdfv_me_menu_screen_full_screen'/>" 
    "  </toolbar>"
    "  <popup name='Popup'>"
    "    <menuitem action='pdfv_me_menu_page_previous'/>"
    "    <menuitem action='pdfv_me_menu_page_next'/>"
    "    <separator name='Sep1'/>"
    "    <menuitem action='pdfv_me_menu_screen_zoom_in'/>"
    "    <menuitem action='pdfv_me_menu_screen_zoom_out'/>"
    "    <menuitem action='pdfv_me_menu_document_details'/>" " </popup>" "</ui>";

#endif /* UI_H */
