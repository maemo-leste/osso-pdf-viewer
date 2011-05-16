/**
    @file state_save.h

    Data structures needed for state saving.

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


#ifndef STATE_SAVE_H
#define STATE_SAVE_H

#include "appdata.h"

/* Application State Save structure */
typedef struct _AppState AppState;

struct _AppState {
    gint is_valid;
    gint scroll_hadj;
    gint scroll_vadj;
    gint fullscreen;
    gint show_images;
    gint current_page;
    double dpi;
    gint zoom_level;
};


typedef enum {
    ST_SUCCESS = 0,
    ST_ERR_NO_SAVED_STATE,
    ST_ERR_OPEN,
} StateSaveResultCode;

/*******************************************************************************
 **** Public Functions
 ***/

/**
   Save application state.

   @param app_data Application data object
   @return result code, ST_SUCCESS on success
 */
StateSaveResultCode save_app_state(AppData * app_data);


/**
   Destory application state data.

   @param  app_data Application data object
   @result gboolean, TRUE if destroyed
 */
gboolean destroy_state_data(AppData * app_data);


#ifdef __cplusplus
extern "C" {
#endif

/**
   Read application state.
   
   @param app_data  Application data object
   @param app_state application state object to be loaded
   @param pointer on the file's uri which should be loaded
   @param pointer on password string for encrypted PDF
   @result result code, ST_SUCCESS on success
 */

    StateSaveResultCode read_app_state(AppData * app_data,
                                       AppState * app_state,
                                       gchar ** uri_str, gchar ** passwd);

#ifdef __cplusplus
}
#endif
#endif                          /* state_save.h */
