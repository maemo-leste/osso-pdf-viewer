/**
    @file interface.h

    Definitions for general user interface functionality.

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


#ifndef INTERFACE_H
#define INTERFACE_H


#include <gtk/gtk.h>
#include <hildon/hildon-program.h>

#include "constant.h"
#include "appdata.h"
#include "state_save.h"
#include "pdfviewer.h"


gboolean take_screen_shot(gpointer widget);
void game_get_screenshot(GtkWidget *widget, GdkEventKey *event, gpointer data);


/**
	Public interface
*/

void ui_create_main_window(AppData * app_data);

void ui_run_file_chooser(AppUIData * app_ui_data,
                         GtkFileChooserAction action);

void ui_run_switch_page_dialog(AppUIData * app_ui_data);

void ui_run_details_dialog(AppUIData * app_ui_data);

void ui_toggle_fullscreen(AppUIData * app_ui_data, gboolean fullscreen);

void ui_set_fullscreen(AppUIData * app_ui_data, gboolean fullscreen);

void ui_update_current_page(AppUIData * app_ui_data);

void ui_show_ovr_image(AppUIData * app_ui_data); 

void ui_hide_overlay_image (AppUIData * app_ui_data, GdkPixbuf * image);

void  ui_calculate_overlay_cords(AppUIData * app_ui_data, Arrow_Cords * cords);

/* The 'extern c' calling convention is required as this is called from the
 * 'pdfviewer' component as well. */
#ifdef __cplusplus
extern "C" {
#endif
    gboolean ui_put_right_arrow_on_idle(gpointer data);
    gboolean ui_put_left_arrow_on_idle(gpointer data);
    void ui_put_arrow(AppUIData * app_ui_data, GdkPixbuf * arrow_image,
                      gint dest_x, gint dest_y, GdkPixbuf ** temporary_copy);
    void ui_calculate_arrows_cords(AppUIData * app_ui_data,
                                   Arrow_Cords * cords);
    void ui_hide_arrows_if_exists(AppUIData * app_ui_data,
                                  gboolean destroy_put_source);

    void ui_arrow_hide(gpointer data);

    /* dimmed menu items callbacks */
    void ui_close_all_banners(AppData * app_data);
    void idle_delete(gpointer data);

    void ui_open_document(AppUIData * app_ui_data,
                          const gchar * filename, const gchar * password);
/**
	Dim helper function (was: dim_functions).
	A function for enabling and disabling buttons on the toolbar
	Moved to here from 'pdfviewer'
	
	@param app_ui_data AppUIData object
	@param dim control to set
	@param TRUE to enable, FALSE to disable
*/
    void ui_enable_page_controls(AppUIData * app_ui_data, PDFDim dim,
                                 gboolean enable);

/**
	Dim helper function 2 (was: dim_functions).
	A function for enabling and disabling document controls.
	
	@param app_ui_data AppUIData object
	@param dim control to set
	@param TRUE to enable, FALSE to disable
*/
    void ui_enable_document_controls(AppUIData * app_ui_data,
                                     gboolean enable);

    void ui_enable_document_open(AppUIData * app_ui_data, gboolean enable);

/**
	Enable or disable scrollbars
	
	@param app_ui_data AppUIData object
	@param henable show or hide horizontal scrollbar
	@param venable show or hide vertical scrollbar
*/
    void ui_enable_scrollbars(AppUIData * app_ui_data,
                              gboolean henable, gboolean venable);


/**
   Sets application UI to the correct state according to the state data.
   Loads PDF document if necessary.

   @param app_ui_data AppUIData object (not NULL)
   @param app_state application state object (not NULL)
 */
    void ui_load_state(AppUIData * app_ui_data, AppState * app_state);

/**
   Checks if the given toolbar item is sensitive.
  
   @param app_ui_data AppUIData object (not NULL)
   @param PDFDim - toolbar item
   @return TRUE if sensitve else FALSE 
*/
    gboolean ui_get_toolitem_is_sensitive(AppUIData * app_ui_data,
                                          PDFDim dim);

/**
   Show  Maximum & Minimum zooming banners in application window

   @param app_ui_data AppUIData object (not NULL)
*/
    void ui_show_zoom_banner(AppUIData * app_ui_data);

/**
   Show an information banner in application window

   @param GtkWindow application window
   @param information string
*/
    void ui_show_banner(GtkWidget * widget, const gchar * msg);

/**
   Show a progress banner in application window

   @param GtkWindow application window
   @param information string
*/
    GtkWidget *ui_show_progress_banner(GtkWindow * app_window,
                                       const gchar * msg);

/**
   Show an information note - with OK button - in application window
   
   @param application data
   @param information string
*/
    void ui_show_note(AppUIData *app_ui_data, const gchar * msg);


/**
   Show the PDFViewerResult in the proper way
   
   @param AppUIData
   @param PDFViewerResult
*/
    void ui_show_result(AppUIData * app_ui_data, PDFViewerResult result);

/**
   Sets current zoom

   @param app_ui_data AppUIData structure
   @param value zoom value
   @return void
*/
    void ui_set_current_zoom(AppUIData * app_ui_data, int value);

/**
   Update every menu/toolbar item on the UI
   according to the application's state

   @param AppUIData
*/
    void ui_update(AppUIData * app_ui_data);
    
    
/**
   Loads application state
   
   @param app_ui_data - strucuture
   @param app_state - structure
*/	
	void ui_scrollbars_sensitive( AppUIData * app_ui_data, gboolean sensitive );

#ifdef __cplusplus
}
#endif
#endif                          /* INTERFACE_H */
