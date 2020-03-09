/**
    @file configuration.c

    Application configuration access through Settings component.

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
#include <glib.h>

#include "appdata.h"
#include "configuration.h"
#include "constant.h"
#include "settings.h"
#include "utility.h"
#include "pdfviewer.h"
#include "ui/callbacks.h"
#include "debug.h"

/**
	Load application configuration from GConf and command line options

	@param argc number of arguments passed from main
	@param *argv[] value structure for arguments
	@param app_data Application data
	@returns gboolean TRUE if successful; FALSE otherwise
*/
gboolean get_configuration(gint argc, gchar * argv[], AppData * app_data)
{
	gboolean show_images = TRUE;
	gboolean first_run = TRUE;

	TDB("Get conf\n");
	/* default images on */
	first_run = settings_get_bool(GCONF_KEY_FIRST_RUN);
	if (!first_run) {
		settings_set_bool(GCONF_KEY_IMAGES, TRUE);
		settings_set_bool(GCONF_KEY_FIRST_RUN, TRUE);
	}

	/* get show images option */
	show_images = settings_get_bool(GCONF_KEY_IMAGES);

	if (show_images == TRUE) {
		PDF_FLAGS_SET(app_data->app_ui_data->flags,
			      PDF_FLAGS_SHOW_IMAGES);
	} else {
		PDF_FLAGS_UNSET(app_data->app_ui_data->flags,
				PDF_FLAGS_SHOW_IMAGES);
	}
	return TRUE;
}

/**
	Save application configuration to GConf
	
	@param app_data Application data
	@returns gboolean TRUE if successful; FALSE otherwise
*/
gboolean save_configuration(AppData * app_data)
{
	g_return_val_if_fail(app_data, FALSE);
	/* store boolean value into gconf */
	settings_set_bool(GCONF_KEY_IMAGES,
			  (gboolean) (PDF_FLAGS_IS_SET
				      (app_data->app_ui_data->flags,
				       PDF_FLAGS_SHOW_IMAGES)));

	return TRUE;
}

/* EOF */
