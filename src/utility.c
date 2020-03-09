/**
    @file utility.c

    Miscellaneous utility functions.

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


#include <glib.h>
#include <hildon-mime.h>
#include <string.h>

#include "utility.h"
#include "constant.h"
#include "appdata.h"


/*******************************************************************************
 **** Private functions
 **/

/* Parses mimetypes from the desktop file */
static GList *
get_application_mime_types(void)
{
    static GList *l = NULL;

    if (!l)
    {
        l = hildon_mime_application_get_mime_types(DESKTOP_FILE_NAME);
    }

    return l;
}


/*******************************************************************************
 **** Public functions
 **/


/**
   Get current time as double
   
   @return Current system unix time as double
*/
double
get_time(void)
{
    GTimeVal tv;
    g_get_current_time(&tv);
    return (double) tv.tv_sec + ((double) tv.tv_usec / G_USEC_PER_SEC);
}

/**
   Get basename of uri for display

   @param  uri to the file
   @return newly allocated UTF-8 string - user shall free the string!
*/
gchar *
get_basename_for_display(const gchar * uri)
{
    GFile* file;
    GFileInfo* fileinfo;
    const gchar* basename;
    gchar * res = NULL;
    GError *error = NULL;

    file = g_file_new_for_uri(uri);
    fileinfo = g_file_query_info(file, G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME, G_FILE_QUERY_INFO_NONE, NULL, &error);
    if (error != NULL) {
        fprintf(stderr, "get_basename_for_display: error: g_file_query_info: %s\n", error->message);
        g_error_free(error);
    } else {
        basename = g_file_info_get_display_name(fileinfo);
        res = g_strdup(basename);

        g_object_unref(fileinfo);
    }

    g_object_unref(file);

    return res;
}

/**
   Checks if the given mime type is supported by the application
   
   @param  the mime type in gchar* format
   @return TRUE if supported, FALSE otherwise
*/
gboolean
mime_type_is_supported(const gchar * mime_type)
{
    GList *mime_types = NULL;
    GList *work = NULL;

    if (!mime_type)
    {
        return FALSE;
    }

    mime_types = get_application_mime_types();

    for (work = mime_types; work != NULL; work = work->next)
        if (g_str_has_prefix(mime_type, work->data))
            return TRUE;

    return FALSE;
}


/**
   Checks if the given file is supported by the application

   @param  uri in string format
   @return TRUE if yes, FALSE otherwise
*/
gboolean
file_is_supported(const gchar * uri_str)
{
    GFile* file;
    GFileInfo* fileinfo;
    GFileType filetype;
    GError *error = NULL;
    const gchar *contenttype;

    gboolean retval = FALSE;

    if (!uri_str)
    {
        return FALSE;
    }

    file = g_file_new_for_uri(uri_str);
    fileinfo = g_file_query_info(file, "standard::*", G_FILE_QUERY_INFO_NONE, NULL, &error);
    //fileinfo = g_file_query_info(gfile, G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE, G_FILE_QUERY_INFO_NONE, NULL, &error);
    if (error != NULL) {
        fprintf(stderr, "file_is_supported: error in g_file_query_info: %s\n", error->message);
        g_error_free(error);
        goto end;
    }

    filetype = g_file_info_get_file_type(fileinfo);
    contenttype = g_file_info_get_content_type(fileinfo);

    if ((filetype == G_FILE_TYPE_REGULAR) && mime_type_is_supported(contenttype) ) {
        retval = TRUE;
    }

    g_object_unref(fileinfo);

end:
    g_object_unref(file);

    return retval;
}


/**
   Returns a GtkFileFilter with supported
   mimetypes. Used by the FileChooserDialog.

   @return GtkFileFilter
*/
GtkFileFilter *
get_filter_for_supported_formats(void)
{
    GList *mime_types = NULL;
    GList *work = NULL;
    static GtkFileFilter *filter = NULL;

    if (!filter)
    {
        filter = gtk_file_filter_new();

        mime_types = get_application_mime_types();

        for (work = mime_types; work != NULL; work = work->next)
            if (work->data != NULL)
                gtk_file_filter_add_mime_type(filter, work->data);

        g_object_ref(filter);
    }

    return filter;
}


/**
   Only call if get_application_mime_types has been called
   previously.
*/
void
free_application_mime_types(void)
{
    GList *mime_types;

    mime_types = get_application_mime_types();

    if (mime_types != NULL)
    {
        hildon_mime_application_mime_types_list_free(mime_types);
    }
}


/**
   Only call if get_filter_for_supported_formats has been
   called previously.
*/
void
free_mime_filters(void)
{
    GtkFileFilter *filter = get_filter_for_supported_formats();

    if (filter != NULL)
    {
        g_object_unref(filter);
    }
}

gchar* uri_to_string(gchar* in_uri) {
    GFile* gfile = g_file_new_for_uri(in_uri);
    gchar* uri = g_file_get_path(gfile);
    g_object_unref(gfile);
    return uri;
}



/* EOF */
