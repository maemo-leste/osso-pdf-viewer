/**
    @file configuration.h

    Configuration definitions.

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


#ifndef CONFIGURATION_H
#define CONFIGURATION_H


#include "glib.h"
#include "appdata.h"


/**
	Public interface
*/

gboolean get_configuration(gint argc, gchar * argv[], AppData * app_data);

gboolean save_configuration(AppData * app_data);


#endif /* CONFIGURATION_H */
