/**
    @file callbacks.h

    Function definitions for user interface callbacks.

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


#ifndef CALLBACKS_H
#define CALLBACKS_H


#include "i18n.h"
#include "appdata.h"
#include <gtk/gtk.h>
#include <hildon/hildon-number-editor.h>
#include <libgnomevfs/gnome-vfs-monitor.h>

/**
	Public interface
*/

int get_display_single_page_continuous_mode ();

/* ACTION callbacks */
gboolean menu_opened(GtkWidget * widget,
                     GdkEventExpose * event, gpointer user_data);
void on_undefined_action(GtkAction * action, gpointer user_data);

void on_document_open(GtkAction * action, gpointer user_data);
void on_document_save(GtkAction * action, gpointer user_data);
void on_document_details(GtkAction * action, gpointer user_data);

void on_page_previous(GtkAction * action, gpointer user_data);
void on_page_next(GtkAction * action, gpointer user_data);
void on_page_first(GtkAction * action, gpointer user_data);
void on_page_last(GtkAction * action, gpointer user_data);
void on_page_switch_to(GtkAction * action, gpointer user_data);

void on_screen_full_screen(GtkAction * action, gpointer user_data);
void on_screen_zoom_in(GtkAction * action, gpointer user_data);
void on_screen_zoom_out(GtkAction * action, gpointer user_data);
void on_screen_zoom_page(GtkAction * action, gpointer user_data);
void on_screen_zoom_width(GtkAction * action, gpointer user_data);
void on_screen_show_images(GtkAction * action, gpointer user_data);

/* end of action callbacks */

/* other callbacks */

void display_single_page_continuous(HildonCheckButton *button);

void on_switch_to_page_help(GtkAction * action, gpointer user_data);

gboolean key_press(GtkWidget * widget, GdkEventKey * event, gpointer data);

gboolean key_release(GtkWidget * widget, GdkEventKey * event, gpointer data);

/* screen callbacks */
int on_screen_scroll(GtkAdjustment * adjustment, gpointer user_data);
void on_screen_tap_and_hold_event(GtkWidget * widget, gpointer data);
gboolean on_screen_event(GtkWidget * widget,
                         GdkEvent * event, gpointer user_data);
gboolean on_screen_press(GtkWidget * widget,
                         GdkEventButton * event, gpointer user_data);
gboolean on_screen_release(GtkWidget * widget,
                           GdkEventButton * event, gpointer user_data);
gboolean on_screen_motion(GtkWidget * widget,
                          GdkEventMotion * event, gpointer user_data);
//sandy starts
gboolean on_scroll_release(GtkWidget * widget,
		           GdkEventButton * event, gpointer user_data);
gboolean on_scroll_press(GtkWidget * widget,
		         GdkEventButton * event, gpointer user_data);
//sandy ends


/* application top/untop callbacks */
void top_changed(GObject * self, GParamSpec * property_param, gpointer data);


gboolean on_delete_event(GtkWidget * widget,
                         GdkEvent * event, gpointer user_data);

gboolean window_state_changed(GtkWidget *widget, GdkEventWindowState *event, gpointer user_data); 

gboolean show_number_editor_error(gpointer app_ui_data);

gboolean on_number_editor_error(HildonNumberEditor * hildonnumbereditor,
                                gint arg1, gpointer user_data);


void on_file_changed(GnomeVFSMonitorHandle * handle,
                     const gchar * monitor_uri,
                     const gchar * info_uri,
                     GnomeVFSMonitorEventType event_type, gpointer user_data);

gboolean configure_event_cb (GtkWidget *widget, GdkEvent *event, gpointer user_data);


#endif
 /* CALLBACKS_H */
