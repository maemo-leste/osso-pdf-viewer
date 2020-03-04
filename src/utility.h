/**
    @file utility.h

    Utility definitions.

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


#ifndef UTILITY_H
#define UTILITY_H

#include <libosso.h>
#include <gtk/gtk.h>
#include <libgnomevfs/gnome-vfs.h>

#define OSSO_LOG_CRIT(...) g_critical(__VA_ARGS__)
#define OSSO_LOG_ERR(...) g_error(__VA_ARGS__)
#define OSSO_LOG_WARNING(...) g_warning(__VA_ARGS__)
#define OSSO_LOG_INFO(...) g_message(__VA_ARGS__)
#define OSSO_LOG_DEBUG(...) g_debug(__VA_ARGS__)

#define ACTION_NOT_IMPLEMENTED(...) (hildon_banner_show_information(NULL, NULL,"Feature not implemented (%s)", gtk_action_get_name(action)))

#define NOT_IMPLEMENTED(...) (hildon_banner_show_information(NULL, NULL,"Feature not implemented (%s)", __VA_ARGS__))

double get_time(void);

gboolean mime_type_is_supported(const gchar * mime_type);

GtkFileFilter *get_filter_for_supported_formats(void);

void free_application_mime_types(void);
void free_mime_filters(void);

#ifdef __cplusplus
extern "C" {
#endif

    gboolean file_is_supported(const gchar * uri);

    gchar *get_basename_for_display(const gchar * uri);

    gchar* uri_to_string(gchar *);

#ifdef __cplusplus
}
#endif
#endif                          /* UTILITY_H */
