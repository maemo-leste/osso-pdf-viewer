/**
    @file settings.h

    Prototypes for Settings component.

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


#ifndef SETTINGS_H
#define SETTINGS_H


#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#include <libosso.h>
#include <osso-log.h>

#include "appdata.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 Get the factory default folder

 @return the default folder path as string
 */
    gchar *get_factory_default_folder(void);

/**
 Set key value. Value type not specified.

 @param key Key name as string.
 @param value Key value to be set as GConfValue.
 @returns TRUE on success, FALSE otherwise.
 */
    gboolean settings_set(const gchar * key, const GConfValue * value);

/**
 Set int type key value.

 @param key Key name as string.
 @param value Key value to be set as gint.
 @returns TRUE on success, FALSE otherwise.
 */
    gboolean settings_set_int(const gchar * key, const gint val);

/**
 Set string type key value.

 @param key Key name as string
 @param value Key value to be set as string.
 @returns TRUE on success, FALSE otherwise.
 */
    gboolean settings_set_string(const gchar * key, const gchar * val);

/**
 Set float type key value.

 @param key Key name as string.
 @param value Key value to be set as gdouble.
 @returns TRUE on success, FALSE otherwise.
 */
    gboolean settings_set_float(const gchar * key, const gdouble val);

/**
 Set boolean type key value.

 @param key Key name as string.
 @param value Key value to be set as gboolean.
 @returns TRUE on success, FALSE otherwise.
 */
    gboolean settings_set_bool(const gchar * key, const gboolean val);

/**
 Get key value.

 @param key Key name as string.
 @returns Value as GConfValue.
 */
    GConfValue *settings_get(const gchar * key);

/**
 Get int type key value.

 @param key Key name as string.
 @returns Value as gint.
 */
    gint settings_get_int(const gchar * key);

/**
 Get float type key value.

 @param key Key name as string.
 @returns Value as gdouble.
 */
    gdouble settings_get_float(const gchar * key);

/**
 Get string type key value.

 @param key Key name as string.
 @returns Value as string.
 */
    gchar *settings_get_string(const gchar * key);

/**
 Get boolean type key value.

 @param key Key name as string.
 @returns Value as gboolean.
 */
    gboolean settings_get_bool(const gchar * key);

/**
 Add a notify about changes on a key. Key may be folder as well.

 @param key Key name as string.
 @param func Function to handle the notify event.
 @returns Connection ID for removing the notify or zero on error.
 */
    guint settings_notify_add(const gchar * key, GConfClientNotifyFunc func);

/**
 Removes the notify about changes on the key.

 @param id Connection ID to be removed.
 */
    void settings_notify_remove(guint id);

/**
 Initialize GConf settings object.
 
 @param app_data Application data.
 @returns TRUE on success, FALSE otherwise.
 */
    gboolean init_settings(AppData * app_data);

/**
 Deinit GConf, free reserved memory.

 @param app_data Application data.
 @returns TRUE on success, FALSE otherwise.
 */
    gboolean deinit_settings(void);


#ifdef __cplusplus
}
#endif
#endif                          /* SETTINGS_H */
