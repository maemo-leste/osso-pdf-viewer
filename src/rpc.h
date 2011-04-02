/**
    @file rpc.h

    DBUS functionality, definitions.

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


#ifndef RPC_H
#define RPC_H

#include "../aconf.h"

#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>

#include <gtk/gtk.h>
#include <libosso.h>
#include <glib.h>
#include <osso-log.h>

#include "appdata.h"


/**
	Public interface
*/

gboolean init_comapp_system(AppData * app_data);
void deinit_comapp_system(AppData * app_data);

osso_return_t send_dbus_message(const gchar * method,
                                osso_rpc_t * retval, AppData * app_data);


#endif /* DBUS_H */
