/**
    @file gtk-switch.h

    Solve namespace clash when including xpdf_goo and gtk.

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


/**
 * Since there is a namespace clash when including xpdf_goo and gtk 
 * to solve this:
 * e.g:
 * #include "GString.h"
 * #include "gtk-switch.h"
 * 	#include <gtk/gtk.h>
 * #include "gtk-switch.h"
 */

#ifdef XPDF_GOO
#undef GString
#undef GList
#undef GDir
// #undef GMutex
#undef XPDF_GOO
#else
#define GString	G_String
#define GList		G_List
#define GDir		G_Dir
// #define GMutex G_Mutex
#define XPDF_GOO
#endif
