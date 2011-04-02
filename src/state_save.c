/**
    @file state_save.c

    Application state saving.

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


#include <libosso.h>
#include <string.h>

#include "state_save.h"
#include "pdfviewer.h"
#include "constant.h"
#include "settings.h"

#include "utility.h"

/*******************************************************************************
 **** Private functions
 **/

/**
   Constructs the AppState struct based on the current 
   state of the application.
*/
static gboolean
construct_app_state(AppUIData * app_ui_data, AppState * app_state)
{
    /* check if app_ui_data is not null */
    g_return_val_if_fail(app_ui_data != NULL, FALSE);

    /* 
     * Fill the fields of app_state using the values from the UI data
     * structures and PDFViewerPrivate object. */

    app_state->fullscreen =
        PDF_FLAGS_IS_SET(app_ui_data->flags, PDF_FLAGS_FULLSCREEN);
    app_state->show_images =
        PDF_FLAGS_IS_SET(app_ui_data->flags, PDF_FLAGS_SHOW_IMAGES);
    app_state->current_page = pdf_viewer_get_current_page();
    app_state->dpi = app_ui_data->dpi;

    /* Adjustments */
    if (!app_ui_data->hscroll || !app_ui_data->vscroll)
    {
        return FALSE;
    }

    /* save the positions of the scrollbars */
    app_state->scroll_hadj =
        gtk_range_get_value(GTK_RANGE(app_ui_data->hscroll));
    app_state->scroll_vadj =
        gtk_range_get_value(GTK_RANGE(app_ui_data->vscroll));

    /* the application state struct is valid */
    app_state->is_valid = TRUE;

    return TRUE;
}

/*******************************************************************************
 **** Public Functions
 ***/

StateSaveResultCode
save_app_state(AppData * app_data)
{
    osso_state_t osso_state;
    AppState app_state;
    StateSaveResultCode ret = ST_ERR_NO_SAVED_STATE;
    gchar *uri_str = NULL;
    gchar *passwd = NULL;

    g_return_val_if_fail(app_data != NULL, ret);

    /* there's no document open, nothing to save */
    if (!pdf_viewer_get_uri())
        return ret;

    memset(&app_state, 0, sizeof(AppState));

    if (!construct_app_state(app_data->app_ui_data, &app_state))
        return ret;

    /* Strings cannot be stored in appdata, except as constant-length tables, 
     * so we use gconf instead. */

    pdf_viewer_get_state(&app_state, &uri_str, &passwd);

    /* saving uri string */
    if (!settings_set_string(GCONF_KEY_LAST_FILE, uri_str))
    {
        g_free(uri_str);
        return ret;
    }
    g_free(uri_str);


    /* saving password (should be stored in md5! */
    if (passwd)
    {
        if (!settings_set_string(GCONF_KEY_PASSWORD, passwd))
        {
            g_free(passwd);
            return ret;
        }
        g_free(passwd);
    }

    /* saving the AppState with libosso's state saving function */
    osso_state.state_data = &app_state;
    osso_state.state_size = sizeof(AppState);

    if (osso_state_write(app_data->comapp_system->osso, &osso_state) !=
        OSSO_OK)
        return ret;

    return ST_SUCCESS;
}


gboolean
destroy_state_data(AppData * app_data)
{
    osso_state_t osso_state;
    //AppData app_state;
    AppState app_state; //CID 3943
    

    g_return_val_if_fail(app_data != NULL, FALSE);
    g_return_val_if_fail(app_data->comapp_system->osso != NULL, FALSE);

    memset(&app_state, 0, sizeof(AppState));

    osso_state.state_data = &app_state;
    osso_state.state_size = sizeof(AppState);

    return (osso_state_write(app_data->comapp_system->osso, &osso_state) ==
            OSSO_OK);
}

StateSaveResultCode
read_app_state(AppData * app_data,
               AppState * app_state, gchar ** uri_str, gchar ** passwd)
{
    StateSaveResultCode ret = ST_ERR_OPEN;
    osso_return_t osso_ret;
    osso_state_t osso_state;

    g_return_val_if_fail(app_data != NULL, ret);
    g_return_val_if_fail(app_state != NULL, ret);

    /* read the osso state information */
    osso_state.state_data = app_state;
    osso_state.state_size = sizeof(AppState);

    osso_ret = osso_state_read(app_data->comapp_system->osso, &osso_state);

    /* there was no state information saved / not valid */
    if (osso_ret != OSSO_OK || !app_state->is_valid)
    {
        ret = ST_ERR_NO_SAVED_STATE;
        return ret;
    }

    /* Get string state informations */
    *uri_str = settings_get_string(GCONF_KEY_LAST_FILE);
    *passwd = settings_get_string(GCONF_KEY_PASSWORD);

    return ST_SUCCESS;
}

/* EOF */
