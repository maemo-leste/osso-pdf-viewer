/**
    @file callbacks.c

    Callbacks for the user interface.

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


#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <gtk/gtk.h>
#include <libosso.h>
#include <hildon/hildon-program.h>
#include <hildon/hildon-banner.h>
#include <gdk/gdkkeysyms.h>
#include <conbtdialogs-dbus.h>
#include <dbus/dbus-glib.h>

#include "callbacks.h"
#include "interface.h"
#include "utility.h"
#include "pdfviewer.h"
#include "constant.h"
#include "state_save.h"
#include "debug.h"
#include "main.h"
#include "ui.h"


#define NO_RESET_STATE_FILE

#define ACTIVE_AREA_WIDTH 72
#define SCREEN_WIDTH 800
#define MOVE_THRESHOLD 25

#define ACTIVE_AREA_HEIGHT 72
#define SCREEN_HEIGHT 480

int display_mode = 1;

/*
 * Action callbacks for document menu
 */

void
on_document_open(GtkAction * action, gpointer user_data)
{
    AppUIData *app_ui_data;
    display_mode = 1;

    g_return_if_fail(user_data != NULL);

    app_ui_data = (AppUIData *) user_data;

    /* Disabling the open document option */
    ui_enable_document_open(app_ui_data, FALSE);
    
    /* Don't show dialog if application is rendering or saving */
    if( !pdf_viewer_is_rendering() &&
    	app_ui_data->app_data->state != PDF_VIEWER_STATE_SAVING ) {
    	
        ui_run_file_chooser(app_ui_data, GTK_FILE_CHOOSER_ACTION_OPEN);
    } else {
    	//ui_show_banner( NULL, _("pdfv_ib_menu_not_available") );
    }
  
    /* Enabling the open document option */
    ui_enable_document_open(app_ui_data, TRUE);
}

void
on_document_save(GtkAction * action, gpointer user_data)
{
    AppUIData *app_ui_data;

    g_return_if_fail(user_data != NULL);

    app_ui_data = (AppUIData *) user_data;
    
    if( app_ui_data->app_data->state != PDF_VIEWER_STATE_LOADED ) {
    	//ui_show_banner( NULL, _("pdfv_ib_menu_not_available") );
    } else {
        app_ui_data->save_dialog_opened = TRUE;
	    ui_run_file_chooser(app_ui_data, GTK_FILE_CHOOSER_ACTION_SAVE);
        app_ui_data->save_dialog_opened = FALSE;
	}
}

void
on_document_details(GtkAction * action, gpointer user_data)
{
    AppUIData *app_ui_data;

    g_return_if_fail(user_data != NULL);

    app_ui_data = (AppUIData *) user_data;

    ui_run_details_dialog(app_ui_data);
}

/*
 * Action callbacks for page menu
 */

void
on_page_previous(GtkAction * action, gpointer user_data)
{
    AppUIData *app_ui_data;

    g_return_if_fail(user_data != NULL);

    app_ui_data = (AppUIData *) user_data;
    
    /* Hiding the overlay button when traversing between pages*/
    ui_hide_overlay_image(app_ui_data, app_ui_data->ovr_image_orig);
    
    if( !pdf_viewer_is_rendering() ) {
        pdf_viewer_navigate(DOC_NAVI_PREVIOUS);
        ui_update_current_page(app_ui_data);
	
    }
}

void
on_page_next(GtkAction * action, gpointer user_data)
{
    AppUIData *app_ui_data;

    g_return_if_fail(user_data != NULL);

    app_ui_data = (AppUIData *) user_data;

    /* Hiding the overlay button when traversing between pages*/
    ui_hide_overlay_image(app_ui_data, app_ui_data->ovr_image_orig);
 
    if( !pdf_viewer_is_rendering() ) {
    	
        pdf_viewer_navigate(DOC_NAVI_NEXT);
        ui_update_current_page(app_ui_data);
	
     }
}

void
on_page_first(GtkAction * action, gpointer user_data)
{
    AppUIData *app_ui_data;

    g_return_if_fail(user_data != NULL);

    app_ui_data = (AppUIData *) user_data;


    if ( !pdf_viewer_is_rendering() ) {
        pdf_viewer_navigate(DOC_NAVI_FIRST);
        ui_update_current_page(app_ui_data);
    }
}


void
on_page_last(GtkAction * action, gpointer user_data)
{
    AppUIData *app_ui_data;

    g_return_if_fail(user_data != NULL);

    app_ui_data = (AppUIData *) user_data;


    if ( !pdf_viewer_is_rendering() ) {
        pdf_viewer_navigate(DOC_NAVI_LAST);
        ui_update_current_page(app_ui_data);
    }
}


void
on_page_switch_to(GtkAction * action, gpointer user_data)
{
    AppUIData *app_ui_data;

    g_return_if_fail(user_data != NULL);

    app_ui_data = (AppUIData *) user_data;

    if( !pdf_viewer_is_rendering() ) {
        ui_run_switch_page_dialog(app_ui_data);
    }
}


/*
 * Action callbacks for screen menu
 */

void
on_screen_full_screen(GtkAction * action, gpointer user_data)
{
    gboolean active = gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action));
    ui_set_fullscreen((AppUIData *) user_data, active);
}


void
on_screen_zoom_in(GtkAction * action, gpointer user_data)
{
    AppUIData *app_ui_data = (AppUIData *) user_data;

    g_return_if_fail(app_ui_data != NULL);

    if( !pdf_viewer_is_rendering() ) {
        pdf_viewer_zoom(DOC_ZOOM_IN);
    }
}


void
on_screen_zoom_out(GtkAction * action, gpointer user_data)
{
    AppUIData *app_ui_data = (AppUIData *) user_data;

    g_return_if_fail(app_ui_data != NULL);

    if( !pdf_viewer_is_rendering() ) {
        pdf_viewer_zoom(DOC_ZOOM_OUT);
    }
}


void
on_screen_zoom_page(GtkAction * action, gpointer user_data)
{
    AppUIData *app_ui_data = (AppUIData *) user_data;

    g_return_if_fail(app_ui_data != NULL);

    if( !pdf_viewer_is_rendering() ) {
        pdf_viewer_zoom(DOC_ZOOM_PAGE);
    }
}


void
on_screen_zoom_width(GtkAction * action, gpointer user_data)
{
    AppUIData *app_ui_data = (AppUIData *) user_data;

    g_return_if_fail(app_ui_data != NULL);

    if( !pdf_viewer_is_rendering() ) {
        pdf_viewer_zoom(DOC_ZOOM_WIDTH);
    }
}


void
on_screen_show_images(GtkAction * action, gpointer user_data)
{
    AppUIData *app_ui_data = (AppUIData *) user_data;
    gboolean active = gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action));

    g_return_if_fail(app_ui_data != NULL);

    if (active != PDF_FLAGS_IS_SET(app_ui_data->flags, PDF_FLAGS_SHOW_IMAGES))
    {

        gboolean images;

        PDF_FLAGS_TOGGLE(app_ui_data->flags, PDF_FLAGS_SHOW_IMAGES);

        images = PDF_FLAGS_IS_SET(app_ui_data->flags, PDF_FLAGS_SHOW_IMAGES);

    }

    if (pdf_viewer_get_num_pages() > 0)
        pdf_viewer_toggle_images();
}

/*
 * Other action callbacks
 */

/** Action callback for undefined actions.
  * This function is invoked by all undefined menu and toolbar interactions.
  *	
  * @param action Corresponding action
  * @param user_data AppUIData
  * @return void
  */
void
on_undefined_action(GtkAction * action, gpointer user_data)
{
    g_debug("Unhandled action \"%s\"", gtk_action_get_name(action));
}


static gboolean
key_press_disabled(AppUIData * app_ui_data, GdkEventKey * event)
{
    /* 
     * rendering a document or no document is opened disabling HW keys,
     * except if it's SELECT HW button. */
    if( pdf_viewer_is_rendering() ) {
        return TRUE;             // rendering
    }

	/*
    if (pdf_viewer_get_uri() != NULL)
        return (0);             // all keys enabled

    if (event->keyval == GDK_Return)
        return (0);
    if (event->keyval == GDK_Escape)
        return (0);
    */

    return FALSE;                 // uri == null
}

/* Callback for keypresses */
gboolean
key_press(GtkWidget * widget, GdkEventKey * event, gpointer data)
{
    AppUIData *app_ui_data;
    GtkAdjustment *adj = NULL;
    GtkWidget *w = NULL;

    app_ui_data = (AppUIData *) data;
    app_ui_data->key_pressed = event->keyval;
    
    switch (event->keyval)
    {
        /* F6 (or SELECT when file open) = toggle full screen mode */
        case GDK_KP_Enter:
        case GDK_Return:      
			if( !PDF_FLAGS_IS_SET(app_ui_data->flags,
				PDF_FLAGS_SELECT_KEY_ALLOWED ) ||
				pdf_viewer_get_num_pages() == 0 ) {
							
				break;
			}
			PDF_FLAGS_UNSET(app_ui_data->flags, PDF_FLAGS_SELECT_KEY_ALLOWED);			
        case GDK_F6:
            ui_toggle_fullscreen(app_ui_data,
	            !PDF_FLAGS_IS_SET(app_ui_data->flags, PDF_FLAGS_FULLSCREEN));
            return TRUE;
        case GDK_F4:           // we only return for menu key
        	return FALSE;
		case GDK_Escape:
            return FALSE;
    }

    if( key_press_disabled(app_ui_data, event) ) {
        return TRUE;
    }

    switch (event->keyval)
    {

		/* scrolling UP or LEFT is almost the same, just different
         * GtkAdjustments */
        /* UP = scroll up 20% */
        case GDK_KP_Up:
        case GDK_Up:
	    ui_hide_overlay_image(app_ui_data, app_ui_data->ovr_image_orig);//prasanna
            adj = gtk_layout_get_vadjustment(GTK_LAYOUT(app_ui_data->layout));

            /* LEFT = scroll left 20% */
        case GDK_KP_Left:
        case GDK_Left:
            
	    ui_hide_overlay_image(app_ui_data, app_ui_data->ovr_image_orig);//prasanna
            if (!adj) {

				/* Ignore when no document loaded */
				if( app_ui_data->app_data->state == PDF_VIEWER_STATE_EMPTY ) {
					/*ui_show_banner(GTK_WIDGET(app_ui_data->app_view),
						_("pdfv_ib_menu_not_available") );*/
            		return TRUE;
            	}

                adj = gtk_layout_get_hadjustment(
                	GTK_LAYOUT(app_ui_data->layout));
            }

            /* If scroll is far left/top or page fits to screen then try to
               move to previous page */
            if ( adj->upper <= adj->page_size || adj->value == 0.0 ) {
            	w = gtk_ui_manager_get_widget(app_ui_data->ui_manager,
					"/ToolBar/pdfv_me_menu_page_previous");

				if ((w != NULL) && (gtk_widget_is_sensitive(w))) {
					on_page_previous(NULL, data);
					adj = gtk_layout_get_vadjustment(GTK_LAYOUT          (app_ui_data->layout));	
					gtk_adjustment_set_value(adj,adj->value + adj->upper - adj->page_size -0.0001);
				} else {
					if (pdf_viewer_get_current_page() == 1) {
                            ui_show_banner(GTK_WIDGET(app_ui_data->app_view),
                                           _("pdfv_ib_first_page_reached"));
                    }
                }

                return TRUE;
            }            
            
            /* scroll the page by 20% */            
            if (adj->lower <= adj->value - 0.2 * adj->page_size)
                gtk_adjustment_set_value(adj,
                                         adj->value - 0.2 * adj->page_size);
            else
                gtk_adjustment_set_value(adj, adj->lower);

            return TRUE;

            /* scrolling DOWN or RIGHT is almost the same, just different
             * GtkAdjustments */
            /* DOWN = scroll down 20% */
        case GDK_KP_Down:
	case GDK_Down:
	    ui_hide_overlay_image(app_ui_data, app_ui_data->ovr_image_orig);//prasanna
            adj = gtk_layout_get_vadjustment(GTK_LAYOUT(app_ui_data->layout));

            /* RIGHT = scroll to the right 20% */
        case GDK_KP_Right:
        case GDK_Right:
	    ui_hide_overlay_image(app_ui_data, app_ui_data->ovr_image_orig);//prasanna
			if( !adj ) {
	            adj = gtk_layout_get_hadjustment(GTK_LAYOUT(app_ui_data->layout));

				/* Ignore when no document loaded */
				if( app_ui_data->app_data->state == PDF_VIEWER_STATE_EMPTY ) {
					/*ui_show_banner(GTK_WIDGET(app_ui_data->app_view),
						_("pdfv_ib_menu_not_available") );*/
	           		return TRUE;
	           	}
	           	
	        }
            
	        if( adj->page_size < adj->upper &&
	           	adj->value < ( adj->upper - adj->page_size ) ) {
		           
				/* scroll the page by 20% */
	    	    if (adj->value + adj->page_size * 0.2 <=
	    	    	adj->upper - adj->page_size)
						gtk_adjustment_set_value(adj,
							adj->value + adj->page_size * 0.2);
	    	    else
					gtk_adjustment_set_value(adj, adj->upper - adj->page_size);
	    	    
			/* Move more space to move, to next page */             
			} else {
	           	on_page_next(NULL, data);
			}

            return TRUE;

	case GDK_KP_Space: 
        case GDK_space: 
                ui_hide_overlay_image(app_ui_data, app_ui_data->ovr_image_orig);
                adj = gtk_layout_get_vadjustment(GTK_LAYOUT(app_ui_data->layout));

		if (event->state & GDK_SHIFT_MASK) {
	            /* scroll the page up by one screen-height, when Shift+Spacebar is pressed*/		
                    if (adj->lower <= adj->value - adj->page_size)
                        gtk_adjustment_set_value(adj, adj->value - adj->page_size);
	            else
	                 gtk_adjustment_set_value(adj, adj->lower);
                 }
		else
		 {
	            /* scroll the page down by one screen-height, when Spacebar is pressed*/		
		    if( adj->page_size < adj->upper && 
				    adj->value < ( adj->upper - adj->page_size ) ) {
                	if (adj->value + adj->page_size <= adj->upper - adj->page_size)
                            gtk_adjustment_set_value(adj, adj->value + adj->page_size);
			else
		            gtk_adjustment_set_value(adj, adj->upper - adj->page_size);
	            }
	         }
	
		return TRUE;
				      
            /* display infoprint when maximum or minimum zoom level has been
             * reached. See Bugzilla 18393 */

        /* F8 = zoom out */
        case GDK_F8:
	        w = gtk_ui_manager_get_widget(app_ui_data->ui_manager,
				"/ToolBar/pdfv_me_menu_screen_zoom_out");
			if ((w != NULL) && (gtk_widget_is_sensitive(w))) {
				pdf_viewer_zoom(DOC_ZOOM_OUT);
			} else if (pdf_viewer_get_zoom_percent() <= 50) {
				ui_show_banner(GTK_WIDGET(app_ui_data->app_view),
					_("pdfv_ib_minimum_zoom"));
			}
			return TRUE;

        /* F7 = zoom in */
        case GDK_F7:
	        w = gtk_ui_manager_get_widget(app_ui_data->ui_manager,
 	           "/ToolBar/pdfv_me_menu_screen_zoom_in");
            if ((w != NULL) && (gtk_widget_is_sensitive(w))) {
				pdf_viewer_zoom(DOC_ZOOM_IN);
			} else if (pdf_viewer_get_zoom_percent() >= 400) {
	            ui_show_banner(GTK_WIDGET(app_ui_data->app_view),
	                _("pdfv_ib_maximum_zoom"));
			}
			return TRUE;

            /* 
             * RETURN/SELECT = if no document present Open file dialog else
             * full screen mode 
        case GDK_KP_Enter:
        case GDK_Return:
	        if( !PDF_FLAGS_IS_SET(app_ui_data->flags,
	        	PDF_FLAGS_SELECT_KEY_ALLOWED ) ) {
	        	break;
	        }
            PDF_FLAGS_UNSET(app_ui_data->flags, PDF_FLAGS_SELECT_KEY_ALLOWED);
            
            if( (pdf_viewer_get_num_pages() != 0) ||
                !(gtk_widget_is_sensitive( gtk_ui_manager_get_widget
                  (app_ui_data->ui_manager,
                  	"/MenuBar/pdfv_me_main_menu_document/"
                  	"pdfv_me_menu_document_open")))) {

                ui_toggle_fullscreen(app_ui_data,
                	!PDF_FLAGS_IS_SET( app_ui_data->flags,
						PDF_FLAGS_FULLSCREEN));
                  	
            }
            
            return TRUE;
			*/

        default:
            break;
    }

    return FALSE;
}


/* 
 * Navigation keys Arrow keys GDK_Left, GDK_Right, GDK_Up, GDK_Down Cancel
 * (Escape) Esc GDK_Escape Menu key F4 GDK_F4 Home F5 GDK_F5 Fullscreen F6
 * GDK_F6 Plus (Zoom in) F7 GDK_F7 Minus (Zoom out) F8 GDK_F8 */


/* non-repeating key handlers */
gboolean
key_release(GtkWidget * widget, GdkEventKey * event, gpointer data)
{
    AppUIData *app_ui_data = (AppUIData *) data;
    guint key_pressed_was;

    g_return_val_if_fail(app_ui_data != NULL, FALSE);
    
    key_pressed_was = app_ui_data->key_pressed;
    app_ui_data->key_pressed = 0;
       
    switch (event->keyval)
    {
        case GDK_KP_Enter:
        case GDK_Return:
                    
            if( ( key_pressed_was == GDK_KP_Enter ||
            	key_pressed_was == GDK_Return ) &&
            	(pdf_viewer_get_num_pages() == 0))/* &&
                (gtk_widget_is_sensitive
                 (gtk_ui_manager_get_widget
                  (app_ui_data->ui_manager,
                   "/MenuBar/pdfv_me_main_menu_document/"
                   "pdfv_me_menu_document_open"))))*/
            {
	        /* Disable the open dialog if the user presses the enter key continuously,
		 * it should respond only for first key press */	
	        ui_enable_document_open(app_ui_data, FALSE);    
                
		ui_run_file_chooser(app_ui_data,
                                    GTK_FILE_CHOOSER_ACTION_OPEN);
		
		/* Enable the open dialog after the user responded to the open dialog*/
		ui_enable_document_open(app_ui_data, TRUE);    
            }
	    
        case GDK_F6:                    
            
            PDF_FLAGS_SET(app_ui_data->flags, PDF_FLAGS_SELECT_KEY_ALLOWED);            
            return TRUE;
            
        case GDK_Escape:
            if( key_pressed_was == GDK_Escape &&
            	PDF_FLAGS_IS_SET(app_ui_data->flags, PDF_FLAGS_FULLSCREEN )) {
                ui_toggle_fullscreen( app_ui_data, FALSE );
                return FALSE;
            }
	 case GDK_KP_Up:
	 case GDK_Up:	 
	 case GDK_KP_Down:
	 case GDK_Down:
	 case GDK_KP_Right:
	 case GDK_Right:    
	 case GDK_KP_Left:
	 case GDK_Left:    
              if (!pdf_viewer_is_rendering()) 
                  ui_show_ovr_image(app_ui_data);

            break;            
    }

    if (key_press_disabled(app_ui_data, event))
        return TRUE;

	return FALSE;

}


int
on_screen_scroll(GtkAdjustment * adjustment, gpointer user_data)
{
    GtkAdjustment *adj = NULL;
    GtkWidget *w = NULL;
    AppData *app_data = get_app_data();
    ui_hide_arrows_if_exists(app_data->app_ui_data, FALSE);
    pdf_viewer_scroller_changed((PDFScroll) GPOINTER_TO_UINT(user_data));
   if ( display_mode ==1){
   	adj = gtk_layout_get_vadjustment(GTK_LAYOUT(app_data->app_ui_data->layout));
	if ( !adj) {
	/* Ignore when no document loaded */
	     if ( app_data->state == PDF_VIEWER_STATE_EMPTY ) {
		  ui_show_banner(GTK_WIDGET(app_data->app_ui_data->app_view),
						_("pdfv_ib_menu_not_available") );
            	  return TRUE;
             }
	     adj = gtk_layout_get_hadjustment(GTK_LAYOUT(app_data->app_ui_data->layout));
         }
/* If scroll is far left/top or page fits to screen then try to move to previous page */
	 //g_print("adj->value %f",adj->value);
         if ( adj->upper <= adj->page_size || adj->value < 0.0001 ) {
              w = gtk_ui_manager_get_widget(app_data->app_ui_data->ui_manager,
					"/ToolBar/pdfv_me_menu_page_previous");
	      if ( (w != NULL) && (gtk_widget_is_sensitive(w))) {	
		   pdf_viewer_navigate_page(pdf_viewer_get_current_page()-1);
                   ui_update_current_page(app_data->app_ui_data);
		   adj = gtk_layout_get_vadjustment(GTK_LAYOUT(app_data->app_ui_data->layout));	
		   gtk_adjustment_set_value(adj,adj->value + adj->upper - adj->page_size -0.0001);
	      } else {
			if ( pdf_viewer_get_current_page() == 1) {
                             ui_show_banner(GTK_WIDGET(app_data->app_ui_data->app_view),
                                           _("pdfv_ib_first_page_reached"));
                    	}
                 }
          return TRUE;
           }                   	
	 if ( adj->page_size < adj->upper &&adj->value < ( adj->upper - adj->page_size ) ) {
	      return TRUE;	       
	 } else {
		if ( pdf_viewer_get_num_pages() != pdf_viewer_get_current_page() ) {
	           pdf_viewer_navigate_page(pdf_viewer_get_current_page()+1);
                   ui_update_current_page(app_data->app_ui_data);
		   adj = gtk_layout_get_vadjustment(GTK_LAYOUT(app_data->app_ui_data->layout));	
		   gtk_adjustment_set_value(adj,adj->value + 0.0001);
		}
	   }	
         return TRUE;     
   }        
}


void
on_screen_tap_and_hold_event(GtkWidget * widget, gpointer data)
{
    gtk_grab_remove(widget);

    gtk_menu_popup(GTK_MENU(data), NULL, NULL, NULL, NULL, 0,
                   gtk_get_current_event_time());
}


gboolean
on_screen_event(GtkWidget * widget, GdkEvent * event, gpointer user_data)
{
    AppUIData *app_ui_data = (AppUIData *) user_data;
    g_return_val_if_fail(app_ui_data != NULL, TRUE);

    if (app_ui_data->app_data->state <= PDF_VIEWER_STATE_LOADING &&
        event->type == GDK_BUTTON_PRESS)
        return TRUE;

    return FALSE;
}

gboolean
on_scroll_press(GtkWidget * widget, GdkEventButton * event,
		                gpointer user_data)
{
    AppUIData *app_ui_data = (AppUIData *)user_data;
    ui_hide_overlay_image(app_ui_data, app_ui_data->ovr_image_orig);
    
    return FALSE;
}
	 
gboolean
on_scroll_release(GtkWidget * widget, GdkEventButton * event,
			                 gpointer user_data)
{
    AppUIData *app_ui_data = (AppUIData *)user_data;
    ui_show_ovr_image(app_ui_data);

    return FALSE;
}

gboolean
on_screen_press(GtkWidget * widget, GdkEventButton * event,
                gpointer user_data)
{
    AppUIData *app_ui_data = (AppUIData *) user_data;
    g_return_val_if_fail(app_ui_data != NULL, TRUE);
    
    /* Hiding the overlay button whenever screen is pressed*/
    ui_hide_overlay_image(app_ui_data, app_ui_data->ovr_image_orig);
     
    /* Initialize values for potential panning */
    app_ui_data->press_lastx = app_ui_data->lastx = (gint) event->x;
    app_ui_data->press_lasty = app_ui_data->lasty = (gint) event->y;

    /*check if the point that user is clicking is a hyperlink or not */
    if(pdf_clicking_hyperlink(app_ui_data->lastx, app_ui_data->lasty))
    {
	    /* Change the page number showed in UI */
	    ui_update_current_page(app_ui_data);
    }
    /* We are not panning yet */
    PDF_FLAGS_UNSET(app_ui_data->flags, PDF_FLAGS_PANNING);
    PDF_FLAGS_SET(app_ui_data->flags, PDF_FLAGS_BUTTON_DOWN);

    gtk_grab_add(widget);

    return FALSE;
}


gboolean
on_screen_release(GtkWidget * widget, GdkEventButton * event,
                  gpointer user_data)
{
    AppUIData *app_ui_data = (AppUIData *) user_data;
    guint press_pos = 0, scrollbar_pos = 0;
	gint move = 0; /* -1 prev, 1 next */
    guint press_pos_fs = 0, v_scrollbar_pos = 0; 

    g_return_val_if_fail(app_ui_data != NULL, TRUE);
           
    scrollbar_pos =
        (guint) gtk_range_get_value(GTK_RANGE(app_ui_data->hscroll));
 
    v_scrollbar_pos = 
	 (guint) gtk_range_get_value(GTK_RANGE(app_ui_data->vscroll));
 
    press_pos = (guint) event->x;

    press_pos_fs = (guint) event->y; 

    /* Stop panning */
    PDF_FLAGS_UNSET(app_ui_data->flags,
                    PDF_FLAGS_PANNING | PDF_FLAGS_BUTTON_DOWN);

    gtk_grab_remove(widget);
    
    /* Show the overlay button only when rendering is not taking place*/
    if (!pdf_viewer_is_rendering()) 
    ui_show_ovr_image(app_ui_data);
      
    /* Check for FS and tap, then identify the action */
    if( PDF_FLAGS_IS_SET(app_ui_data->flags, PDF_FLAGS_FULLSCREEN ) ) {

	/* Initialize values for potential panning when no document is opened */
       	if(app_ui_data->app_data->state == PDF_VIEWER_STATE_EMPTY) {
    		app_ui_data->press_lastx = app_ui_data->lastx = (gint) event->x;
	    	app_ui_data->press_lasty = app_ui_data->lasty = (gint) event->y;
	}
	
    	/* If loading always give banner */
    	if(app_ui_data->app_data->state == PDF_VIEWER_STATE_LOADING ) {
	    	//ui_show_banner( NULL, _("pdfv_ib_menu_not_available") );
	    	return FALSE;    	
    	
    	} else if(
    		( abs(app_ui_data->press_lastx - event->x) < MOVE_THRESHOLD ) &&
			( abs(app_ui_data->press_lasty - event->y) < MOVE_THRESHOLD ) ) {
		
		//if (press_pos >= (scrollbar_pos + SCREEN_WIDTH - ACTIVE_AREA_WIDTH)) {
		if(!app_ui_data->isPortrait) {
			if (press_pos >= (scrollbar_pos + SCREEN_WIDTH - ACTIVE_AREA_WIDTH) &&
				(press_pos_fs <= (v_scrollbar_pos + SCREEN_HEIGHT - ACTIVE_AREA_HEIGHT))) {
	        		move = 1;
        		} else if(press_pos <= (scrollbar_pos + ACTIVE_AREA_WIDTH)) {
        			move = -1;
        		}
		}
		else {
			if (press_pos >= (scrollbar_pos + SCREEN_HEIGHT - ACTIVE_AREA_WIDTH) &&
				(press_pos_fs <= (v_scrollbar_pos + SCREEN_WIDTH - ACTIVE_AREA_HEIGHT))) {
	        		move = 1;
        		} else if(press_pos <= (scrollbar_pos + ACTIVE_AREA_WIDTH)) {
        			move = -1;
        		}
		}

		/* Calculating the activation area of  fullscreen overlay*/
		if(!app_ui_data->isPortrait) {
			/* Landscape mode */
			if ((press_pos_fs >= v_scrollbar_pos + SCREEN_HEIGHT - ACTIVE_AREA_HEIGHT)
			     && ((press_pos >= (scrollbar_pos + SCREEN_WIDTH - ACTIVE_AREA_WIDTH)))){ 
			      move = 2;
			}
		}
		else {
			/* Portrait mode */
			if ((press_pos_fs >= v_scrollbar_pos + SCREEN_WIDTH - ACTIVE_AREA_HEIGHT)
			     && ((press_pos >= (scrollbar_pos + SCREEN_HEIGHT - ACTIVE_AREA_WIDTH)))){ 
			      move = 2;
			}
		}
		
														  
        }
    }
    
    /* We can quit if we don't have an action */
    if( move == 0 ) {
    	return FALSE;
    }
    
    /* Check if it's ok to run action */
    if( pdf_viewer_is_rendering() || 
    	!PDF_FLAGS_IS_SET(app_ui_data->flags, PDF_FLAGS_SELECT_KEY_ALLOWED) ) {
    	//ui_show_banner( NULL, _("pdfv_ib_menu_not_available") );
    	
	/* Do the actions */
    } else if( move == 1 ) {
	    on_page_next(NULL, user_data);
	} else if( move == -1 ) {
		on_page_previous(NULL, user_data);
	}
    
        else if(move == 2){
                 ui_toggle_fullscreen(app_ui_data, FALSE);
        }
		    
    return FALSE;
}


gboolean
on_screen_motion(GtkWidget * widget, GdkEventMotion * event,
                 gpointer user_data)
{
    AppUIData *app_ui_data = (AppUIData *) user_data;
    GtkAdjustment *adj;
    gdouble value;
    gint x, y;
    gint dx, dy;
    g_return_val_if_fail(app_ui_data != NULL, TRUE);

    /* ignore event if button is not pressed, or page is rendering */
    /* TODO could add test to ensure that document is loaded */
    if (!PDF_FLAGS_IS_SET(app_ui_data->flags, PDF_FLAGS_BUTTON_DOWN)
        || pdf_viewer_is_rendering() ) {
        return TRUE;
    }

    if (event->is_hint)
    {
        /* hint event, retrieve pointer location */
        gdk_window_get_pointer(event->window, &x, &y, NULL);
    }
    else
    {
        /* use motion event coordinates */
        x = (gint) event->x;
        y = (gint) event->y;
    }

    /* calculate delta values */
    dx = x - app_ui_data->lastx;
    dy = y - app_ui_data->lasty;

    if (!PDF_FLAGS_IS_SET(app_ui_data->flags, PDF_FLAGS_PANNING))
    {
        /* not panning yet, check if threshold exceeded */
        if (ABS(dx) > PANNING_THRESHOLD || ABS(dy) > PANNING_THRESHOLD)
        {
            /* start panning */
            PDF_FLAGS_SET(app_ui_data->flags, PDF_FLAGS_PANNING);

            /* don't move just yet */
            app_ui_data->lastx = x;
            app_ui_data->lasty = y;
        }
    }
    else
    {
        /* panning */

        /* retrieve and set vertical adjustment, ensure that range is not
         * exceeded */
        adj = gtk_layout_get_vadjustment(GTK_LAYOUT(app_ui_data->layout));
        value = adj->value - (gdouble) dy;
        if (value < adj->lower)
            value = adj->lower;
        else if (value > (adj->upper - adj->page_size))
            value = adj->upper - adj->page_size;
        gtk_adjustment_set_value(adj, value);

        /* retrieve and set horizontal adjustment */
        adj = gtk_layout_get_hadjustment(GTK_LAYOUT(app_ui_data->layout));
        value = adj->value - (gdouble) dx;
        if (value < adj->lower)
            value = adj->lower;
        else if (value > (adj->upper - adj->page_size))
            value = adj->upper - adj->page_size;
        gtk_adjustment_set_value(adj, value);

        /* NOTE on_screen_scroll handler is invoked automatically when
         * gtk_adjustment_set_value is called */
    }

    return FALSE;
}


/**
 
	Handles top/untop event.

	@param widget Pointer to window widget
	@param data Pointer to AppUIData
*/

void
top_changed(GObject * self, GParamSpec * property_param, gpointer data)
{
    HildonProgram *program = HILDON_PROGRAM(self);
    if (hildon_program_get_is_topmost(program))
    {
        AppUIData *app_ui_data = (AppUIData *) data;

        /* topmost status acquired */
        g_return_if_fail(app_ui_data != NULL);
        g_return_if_fail(app_ui_data->app_data != NULL);
        // g_return_if_fail(app_ui_data->app_data->osso_context != NULL);

        /* destory the state file */
        // destroy_state_data(app_ui_data->app_data);

        PDF_FLAGS_UNSET(app_ui_data->flags, PDF_FLAGS_BACKGROUNDED);

        hildon_program_set_can_hibernate(app_ui_data->app, FALSE);

    }
    else
    {
        /* topmost status lost, save state */

        AppUIData *app_ui_data = (AppUIData *) data;

        if (app_ui_data->app_data == NULL ||
            app_ui_data->app_data->comapp_system->osso == NULL)
        {
            return;
        }

        /* Save application's state */
        save_app_state(app_ui_data->app_data);

        /* Key repeat flags need to be reset on focus out */
        PDF_FLAGS_SET(app_ui_data->flags,
                      PDF_FLAGS_SELECT_KEY_ALLOWED | PDF_FLAGS_BACKGROUNDED);

        hildon_program_set_can_hibernate(app_ui_data->app, TRUE);

    }
}



gboolean
on_delete_event(GtkWidget * widget, GdkEvent * event, gpointer user_data)
{
    AppUIData *app_ui_data = (AppUIData *) user_data;
    gboolean retval = FALSE;
    g_return_val_if_fail(app_ui_data != NULL, FALSE);
    g_return_val_if_fail(app_ui_data ->app_data != NULL, FALSE);

    TDB("%s\n", __FUNCTION__);

    if (app_ui_data->app_data->state == PDF_VIEWER_STATE_LOADING ) {
        retval = TRUE;
    } else {
        app_ui_data->close_called = TRUE;
        g_return_val_if_fail(app_ui_data != NULL, FALSE);

        /* wait for finishing rendering thread */
        TDB("call pdf_viewer_cancel_if_render\n");
        pdf_viewer_cancel_if_render();

        destroy_state_data(app_ui_data->app_data);
        retval = TRUE;
        gtk_main_quit();
    }
    return retval;
}

gboolean
window_state_changed(GtkWidget *widget, GdkEventWindowState *event, gpointer user_data)
{
    AppUIData *app_ui_data = (AppUIData *)user_data;
    
    gtk_window_present(GTK_WINDOW(app_ui_data->app_view));

    return FALSE;
}	

typedef struct {
    HildonNumberEditor *hildonnumbereditor;
    AppUIData *app_ui_data;
} SNumberError;


gboolean
show_number_editor_error(gpointer data)
{
    SNumberError *number = (SNumberError *) data;
    guint min = PDF_PAGE_INIT + 1,
        max = pdf_viewer_get_num_pages();
    const gchar *PDFstr = SUPPRESS_FORMAT_WARNING(_("pdfv_error_pagenumber"));

    gchar *info = NULL;
    GDK_THR_ENTER;
    if (!G_IS_OBJECT(number->hildonnumbereditor))
    {
        GDK_THR_LEAVE;
        return FALSE;
    }

    if ((info =
         g_strdup_printf(PDFstr/*(SUPPRESS_FORMAT_WARNING(_("pdfv_error_pagenumber"))*/,
                         min, max)) == NULL)
    {
        GDK_THR_LEAVE;
        return FALSE;
    }

    hildon_banner_show_information(GTK_WIDGET(number->app_ui_data->app_view),
                                   NULL, info);
    g_free(info);
    g_object_set_data(G_OBJECT(number->hildonnumbereditor), "idle-id",
                      (gpointer) 0);
    GDK_THR_LEAVE;
    return FALSE;
}

gboolean
on_number_editor_error(HildonNumberEditor * hildonnumbereditor,
                       gint arg1, gpointer user_data)
{
    AppUIData *app_ui_data = (AppUIData *) user_data;
    SNumberError *error;

    g_return_val_if_fail(app_ui_data != NULL, FALSE);

    if (g_object_get_data(G_OBJECT(hildonnumbereditor), "idle-id"))
        return TRUE;

    error = g_new(SNumberError, 1);
    error->app_ui_data = app_ui_data;
    error->hildonnumbereditor = hildonnumbereditor;
    g_object_set_data(G_OBJECT(hildonnumbereditor), "idle-id",
                      (gpointer) g_idle_add_full(G_PRIORITY_DEFAULT_IDLE,
                                                 show_number_editor_error,
                                                 error,
                                                 (GDestroyNotify) g_free));

    return TRUE;
}

#if 0
/**
	Handles on_file_changed event

	@param handle Gnome vfs monitor handle
	@param monitor_uri uri to monitor
	@param info_uri, not used
	@param event_type the type of the file event
	@param user_data pointer to AppData as gpointer
*/
void
on_file_changed(GnomeVFSMonitorHandle * handle,
                const gchar * monitor_uri,
                const gchar * info_uri,
                GnomeVFSMonitorEventType event_type, gpointer user_data)
{
    AppData *app_data = (AppData *) user_data;
    gchar *loaded_uri = NULL;
    GtkWidget *widget = NULL;
    g_return_if_fail(app_data != NULL);

    switch (event_type)
    {
        case GNOME_VFS_MONITOR_EVENT_DELETED:

            /* check if this document is currently loaded */
            if (app_data->state == PDF_VIEWER_STATE_LOADED)
            {
                loaded_uri = gnome_vfs_uri_to_string(pdf_viewer_get_uri(),
                                                     GNOME_VFS_URI_HIDE_NONE);

                if (strcmp(monitor_uri, loaded_uri) == 0)
                {
                    /* Dim 'save as' item */
                    widget =
                        gtk_ui_manager_get_widget(app_data->app_ui_data->
                                                  ui_manager,
                                                  "/MenuBar/pdfv_me_main_menu_document/"
                                                  "pdfv_me_menu_document_save");
                    gtk_widget_set_sensitive(widget, FALSE);
                    /* as per spec, a deleted file which is open in an
                     * application should be closed */
                    pdf_viewer_unload();
                }

                g_free(loaded_uri);
            }
	
            OSSO_LOG_DEBUG("file has been deleted! info: %s, monitor: %s\n",
                           info_uri, monitor_uri);

            break;
        case GNOME_VFS_MONITOR_EVENT_CREATED:
        case GNOME_VFS_MONITOR_EVENT_CHANGED:
            OSSO_LOG_DEBUG("file has been changed! info: %s, monitor: %s\n",
                           info_uri, monitor_uri);
            break;
        default:
            OSSO_LOG_DEBUG("file has been something! info: %s, monitor: %s\n",
                           info_uri, monitor_uri);
            break;
    }
}
#endif

gboolean take_screen_shot(gpointer widget)
{ 
   GtkWidget *window = (GtkWidget *)widget;
   gchar *filename = NULL;
   gchar *service = g_strconcat(OSSO_PDFVIEWER_SERVICE, ".pvr", NULL);   

   //filename = g_build_filename((const ghar*)("/home/user/.cache/launch/"), service, NULL);
   filename = g_build_filename("/home/user/.cache/launch/", service, NULL);

   if(FALSE == g_file_test(filename, G_FILE_TEST_EXISTS))
   {
   	hildon_gtk_window_take_screenshot(GTK_WINDOW(window), TRUE);
   }

   return FALSE;
}

gboolean
configure_event_cb (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GtkWidget *toolbar_widget;

	g_return_val_if_fail(widget != NULL && HILDON_IS_WINDOW(widget), FALSE);
	g_return_val_if_fail(event->type == GDK_CONFIGURE, FALSE);
	g_return_val_if_fail(user_data != NULL, FALSE);

	AppUIData *app_ui_data = (AppUIData *) user_data;
	GdkEventConfigure *configure_event = (GdkEventConfigure*)event;

	if (configure_event->height > configure_event->width) 
	{
		/* Portrait mode */
		app_ui_data->isPortrait = TRUE;
		gtk_tool_item_set_expand(app_ui_data->current_page_item, FALSE);
		
		if(app_ui_data->current_zoom_item != NULL)
			gtk_container_remove(app_ui_data->toolbar, app_ui_data->current_zoom_item);

		toolbar_widget = gtk_ui_manager_get_widget(app_ui_data->ui_manager,
        					               "/ToolBar/"
							       "pdfv_me_menu_screen_zoom_out");
		if (toolbar_widget != NULL)
	        	gtk_container_remove(app_ui_data->toolbar, toolbar_widget);

		toolbar_widget = gtk_ui_manager_get_widget(app_ui_data->ui_manager,
        					               "/ToolBar/"
							       "pdfv_me_menu_screen_zoom_in");
		if (toolbar_widget != NULL)
	        	gtk_container_remove(app_ui_data->toolbar, toolbar_widget);
	}
	else
	{
		/* Landscape mode */
		app_ui_data->isPortrait = FALSE;
		gtk_tool_item_set_expand(app_ui_data->current_page_item, TRUE);

		if(gtk_toolbar_get_item_index(app_ui_data->toolbar, app_ui_data->current_zoom_item) != TOOLBAR_POS_CURRENT_ZOOM_WIDGET) 
		{
			toolbar_widget = gtk_ui_manager_get_widget(app_ui_data->ui_manager,
        						               "/ToolBar/"
								       "pdfv_me_menu_screen_zoom_out");
			if (toolbar_widget != NULL)
				gtk_toolbar_insert(GTK_TOOLBAR(app_ui_data->toolbar),
        		               toolbar_widget, (TOOLBAR_POS_CURRENT_ZOOM_WIDGET-1));

			if(app_ui_data->current_zoom_item != NULL)
				gtk_toolbar_insert(GTK_TOOLBAR(app_ui_data->toolbar),
        	        	       app_ui_data->current_zoom_item, TOOLBAR_POS_CURRENT_ZOOM_WIDGET);

			toolbar_widget = gtk_ui_manager_get_widget(app_ui_data->ui_manager,
        						               "/ToolBar/"
								       "pdfv_me_menu_screen_zoom_in");
			if (toolbar_widget != NULL)
				gtk_toolbar_insert(GTK_TOOLBAR(app_ui_data->toolbar),
        		               toolbar_widget, (TOOLBAR_POS_CURRENT_ZOOM_WIDGET+1));
		}

	}
	return FALSE;
}

void
game_get_screenshot(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
  g_idle_add((GSourceFunc)(take_screen_shot), (gpointer)(widget));
}  

void
display_single_page_continuous (HildonCheckButton *button)
{
  gboolean active;

  active = hildon_check_button_get_active (button);
  
  if(active)
    display_mode = 1;
  else
    display_mode = 0;
}

int
get_display_single_page_continuous_mode ()
{
  return display_mode;
}
