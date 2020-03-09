/**
    @file main.c

    Application main.

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


#include <libintl.h>
#include <locale.h>
#include <gtk/gtk.h>
#include <libosso.h>
#include <signal.h>
#include <stdlib.h>
#include <dbus/dbus-glib.h>
#include <hildon/hildon.h>

#include <comapp_system.h>

#include "rpc.h"
#include "ui/interface.h"
#include "utility.h"
#include "pdfviewer.h"
#include "configuration.h"
#include "settings.h"
#include "state_save.h"
#include "debug.h"
#include "main.h"

/*******************************************************************************
 **** Private data
 **/
static AppData *app_data = NULL;

AppData *
get_app_data(void)
{
    return app_data;
}

/*******************************************************************************
 **** Private functions
 **/

GThread *mainThread = NULL;
gboolean just_exit = FALSE;

/*******************************************************************************
 **** Public function
 **/

/**
	Application main.
	Initializes internationalization, libosso, app and appview.
	Calls user interface creation functions and gtk_main.

	Follows the component initialization order of
	osso-filemanager main.
		
	@param argc Number of command line arguments
	@param argv Command line arguments
	@return 0 if successful; otherwise error code
*/
int
main(int argc, char *argv[])
{
    gboolean result;

    //g_thread_init(NULL);
    dbus_g_thread_init();
    gdk_threads_init();

    mainThread = g_thread_self();


    /* Allocate application data structures */
    app_data = g_new0(AppData, 1);
    if (app_data == NULL)
    {
        OSSO_LOG_CRIT("Failed memory allocation: AppData");
        exit(1);
    }

    /* there was no low memory ind from HW yet. */
    app_data->low_memory = FALSE;

    app_data->app_ui_data = g_new0(AppUIData, 1);
    if (app_data->app_ui_data == NULL)
    {
        OSSO_LOG_CRIT("Failed memory allocation: AppUIData");
        // free_app_data();
        exit(1);
    }

    /* Add reference back to parent structure (AppData) */
    app_data->app_ui_data->app_data = app_data;

    /* init comapp_system */

    init_comapp_system(app_data);

    /* Initialize GConf and read application configuration */
    if (!init_settings(app_data))
    {
        OSSO_LOG_CRIT("Failed initializing GConf");
        return OSSO_ERROR;
    }
    
    if (!get_configuration(argc, argv, app_data))
    {
        OSSO_LOG_CRIT("Failed reading configuration");
        return OSSO_ERROR;
    }
#ifdef ENABLE_NLS
    /* Initialize localization */
    /* Gettext does not seem to work properly without the following function
     * call */
    setlocale(LC_ALL, "");

    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);
#endif /* ENABLE_NLS */

    /* Initialize GTK */
    //gtk_init(&argc, &argv);
    
    /* Initialize GTK+ and Hildon */
    hildon_gtk_init(&argc, &argv);

    /* Create application UI */
    ui_create_main_window(app_data);

    /* Initialize engine */
    pdf_viewer_init(app_data->app_ui_data);

    app_data->app_ui_data->password_dialog = NULL;

    app_data->app_ui_data->replace_dialog = NULL;

    //hildon_gtk_window_take_screenshot(GTK_WINDOW(app_data->app_ui_data->app_view), TRUE);
    
    g_signal_connect(G_OBJECT(app_data->app_ui_data->app_view), "map-event",
		    	G_CALLBACK(game_get_screenshot), NULL);    //prasanna
    
    GDK_THR_ENTER;
    gtk_main();
    GDK_THR_LEAVE;
    
    if( !just_exit ) {
		g_debug( "Save configuration..." );
        save_configuration(app_data);

		g_debug( "Deinit engine..." );
        pdf_viewer_deinit();

		g_debug( "Deinit compapp systems..." );
        deinit_comapp_system(app_data);

		g_debug( "Deinit settings..." );
        deinit_settings();

		g_debug( "Deinit mime..." );
        free_mime_filters();
        free_application_mime_types();
    }
    
    /* Exit successfully, regardless of any errors */
	g_debug( "Exit success" );    
    exit(EXIT_SUCCESS);
}


/* EOF */
