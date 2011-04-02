/**
    @file rpc.c

    DBUS functionality.

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


#include <gtk/gtk.h>
#include <libosso.h>
#include <comapp_system.h>


#include "rpc.h"
#include "appdata.h"
#include "constant.h"
#include "utility.h"
#include "ui/interface.h"
#include "debug.h"

#include "pdfviewer.h"
#include "thread_debug.h"


/* flag from main.c */
extern gboolean just_exit;

/**
	Sends a DBUS message
	
	@todo Currently cannot specify target app or pass arguments.
	
	@param method Method to be called.
	@param retval The return value of the method.
	@param app_data Application specific data
*/
osso_return_t
send_dbus_message(const gchar * method,
                  osso_rpc_t * retval, AppData * app_data)
{
    return osso_rpc_run(app_data->comapp_system->osso,
                        "com.nokia.app_launcher",
                        "/com/nokia/app_launcher",
                        "app_launcher", method, retval, DBUS_TYPE_INVALID);
}


/**
	Private functions
*/

/** 
	Receive D-BUS messages and handles them
	
	@param interface The interface of the called method.
	@param method The method that was called.
	@param arguments A GArray of osso_rpc_t_structures.
	@param data An application specific pointer.
	@param retval The return value of the method.
	@returns gint value
*/
static gint
pdf_dbus_req_handler(const gchar * interface,
                     const gchar * method,
                     GArray * arguments, gpointer data, osso_rpc_t * retval)
{
    ComappSystemData *csd = (ComappSystemData *) data;
    AppData *app_data = (AppData *) csd->user_data;
    
    if (g_ascii_strcasecmp(method, "top_application") == 0)
    {
       /* If the application is the top application then move it to current desktop
	* and give it the keyboard focus */ 
        gtk_window_present(GTK_WINDOW(app_data->app_ui_data->app_view));

	retval->type = DBUS_TYPE_BOOLEAN;
        retval->value.b = TRUE;
        return OSSO_OK;
    }
    return OSSO_ERROR;
}


/**
 * comapp_system mime_open callback
 */
static void
pdf_mime_open_cb(ComappSystemData * cas)
{
    AppUIData *app_ui_data = ((AppData *) cas->user_data)->app_ui_data;

    /* bringing front the application on MIME_OPEN */
    gtk_window_present(GTK_WINDOW(app_ui_data->app_view));

    if (app_ui_data->save_dialog_opened)
    {
        /* if save dialog was opened before mime open was called
         * don't load the file just top the application */
        return;
    }

    /* set startup mode */
    app_ui_data->app_data->mode = STARTUP_MODE_URI_REQUEST;

    if (app_ui_data->details_dialog)
    {
        gtk_dialog_response(GTK_DIALOG(app_ui_data->details_dialog),
                            GTK_RESPONSE_OK);
    }

    if (app_ui_data->switch_page_dialog)
    {
        gtk_dialog_response(GTK_DIALOG(app_ui_data->switch_page_dialog),
                            GTK_RESPONSE_CANCEL);
    }

    if (app_ui_data->opensave_dialog)
    {
        gtk_dialog_response(GTK_DIALOG(app_ui_data->opensave_dialog),
                            GTK_RESPONSE_CANCEL);
    }

    if (app_ui_data->note_dialog)
    {
        gtk_dialog_response(GTK_DIALOG(app_ui_data->note_dialog),
                            GTK_RESPONSE_OK);
    }
    
    /* attempt to open document */
    ui_open_document(app_ui_data, cas->mime_open_uri, NULL);
}


/**
 * comapp_system hw event handler callback
 */
static void
pdf_hw_event_handler(ComappSystemData * cas)
{
    AppData *app_data = (AppData *) cas->user_data;

    if (cas->memory_low)
    {
        app_data->low_memory = TRUE;
        g_debug("%s: MEMORY LOW", __func__);
    } 
}


static gboolean
at_reset_cb(ComappSystemData * cas)
{
    g_debug("at_reset_cb\n");
    just_exit = TRUE;
    destroy_state_data((AppData *) cas->user_data);
    return FALSE;
}

static gboolean
at_bgkill_cb(ComappSystemData * cas)
{
    g_debug("at_bgkill_cb\n");

    AppUIData *app_ui_data = ((AppData *) cas->user_data)->app_ui_data;

     if(app_ui_data->replace_dialog)
    {
    	gtk_widget_destroy(GTK_WIDGET(app_ui_data->replace_dialog));
	app_ui_data->replace_dialog = NULL;	
    }
	 
    if(app_ui_data->password_dialog)
    {
    	gtk_widget_destroy(GTK_WIDGET(app_ui_data->password_dialog));
	app_ui_data->password_dialog = NULL;	
    }
    if (app_ui_data->details_dialog)
    {
        gtk_dialog_response(GTK_DIALOG(app_ui_data->details_dialog),
                            GTK_RESPONSE_OK);
    }

    if (app_ui_data->switch_page_dialog)
    {
        gtk_dialog_response(GTK_DIALOG(app_ui_data->switch_page_dialog),
                            GTK_RESPONSE_CANCEL);
    }

    if (app_ui_data->opensave_dialog)
    {
        gtk_dialog_response(GTK_DIALOG(app_ui_data->opensave_dialog),
                            GTK_RESPONSE_CANCEL);
    }

    if (app_ui_data->note_dialog)
    {
        gtk_dialog_response(GTK_DIALOG(app_ui_data->note_dialog),
                            GTK_RESPONSE_OK);
    }
   	
    just_exit = FALSE;
    save_app_state((AppData *) cas->user_data);
    return FALSE;
}

/**
 * Initialize comapp_system stuff.
 *
 * @param app_data Application data structure
 * @return TRUE if it is successful
 */
gboolean
init_comapp_system(AppData * app_data)
{
    /* init comapp_system data structure. let it init osso_context and gconf
     * too */

    app_data->comapp_system = g_new0(ComappSystemData, 1);

    app_data->comapp_system->save_for_asking = at_reset_cb;
    app_data->comapp_system->save_for_loading = at_bgkill_cb;
    app_data->comapp_system->dbus_req_handler = pdf_dbus_req_handler;
    app_data->comapp_system->hw_state_changed = pdf_hw_event_handler;
    app_data->comapp_system->mime_open_cb = pdf_mime_open_cb;
    app_data->comapp_system->pkg_name = OSSO_PDFVIEWER_PACKAGE;
    app_data->comapp_system->pkg_version = PACKAGE_VERSION;
    app_data->comapp_system->dbus_service = OSSO_PDFVIEWER_SERVICE;
    app_data->comapp_system->dbus_path = OSSO_PDFVIEWER_OBJECT_PATH;
    app_data->comapp_system->dbus_interface = OSSO_PDFVIEWER_INTERFACE;
    app_data->comapp_system->user_data = app_data;

    /* initialize */

    app_data->comapp_system = comapp_system_init(app_data->comapp_system);

    /* init logging */

    ULOG_OPEN(PACKAGE_NAME " " PACKAGE_VERSION);

    return TRUE;
}

/**
 * Deinitializes comapp_system.
 *
 * @param app_data application data structure
 */
void
deinit_comapp_system(AppData * app_data) {
    comapp_system_deinit( app_data->comapp_system );
}

/* EOF */
