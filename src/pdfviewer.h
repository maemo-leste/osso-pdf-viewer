/**
    @file pdfviewer.h

    Pdfviewer definitions.

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


#ifndef PDFVIEWER_H
#define PDFVIEWER_H

#include <libgnomevfs/gnome-vfs-uri.h>
#include "appdata.h"
#include "state_save.h"

typedef enum {
    DOC_NAVI_FIRST = 0,
    DOC_NAVI_PREVIOUS,
    DOC_NAVI_NEXT,
    DOC_NAVI_LAST
} PDFNavigate;

typedef enum {
    DOC_ZOOM_INVALID = -5,
    DOC_ZOOM_WIDTH,
    DOC_ZOOM_PAGE,
    DOC_ZOOM_IN,
    DOC_ZOOM_OUT,
    DOC_ZOOM_50 = 0,
    DOC_ZOOM_100,
    DOC_ZOOM_150,
    DOC_ZOOM_200,
    DOC_ZOOM_250,
    DOC_ZOOM_300,
    DOC_ZOOM_400
} PDFZoom;

typedef enum {
    SCROLL_VER = 0,
    SCROLL_HOR
} PDFScroll;

/* errors/results */
typedef enum {
    RESULT_INVALID_INTERFACE = 0,
    RESULT_SAVE_OK,
    RESULT_SAVE_NOT_ALLOWED,
    RESULT_SAVE_FAILED,
    RESULT_SAVE_CANCELLED,
    RESULT_LOAD_OK,
    RESULT_UNSUPPORTED_FORMAT,
    RESULT_CORRUPTED_FILE,
    RESULT_INSUFFICIENT_MEMORY,
    RESULT_INVALID_URI,
    RESULT_NO_SPACE_ON_DEVICE,
    RESULT_INTERRUPTED_MMC_OPEN,
    RESULT_ENCRYPTED_FILE,
    RESULT_COPY_STARTED,
    RESULT_SAVING_NOT_COMPLETED
} PDFViewerResult;


#ifdef __cplusplus
extern "C" {
#endif

    /**
       Public interface
    */
    extern void pdf_viewer_init(AppUIData * app_ui_data);
    extern void pdf_viewer_deinit(void);

    extern PDFViewerResult pdf_viewer_open(const char *uri,
                                           const char *password);
    extern PDFViewerResult pdf_viewer_save(const char *dst);

    extern void pdf_viewer_navigate(PDFNavigate navigate_to);
    extern void pdf_viewer_navigate_page(int page);
    extern void pdf_viewer_zoom(PDFZoom zoom_level);
    extern void pdf_viewer_toggle_images(void);

    extern GnomeVFSURI *pdf_viewer_get_uri(void);
    extern int pdf_viewer_get_num_pages(void);
    extern int pdf_viewer_get_current_page(void);
    extern int pdf_viewer_get_current_zoom(void);

    extern void pdf_viewer_scroller_changed(PDFScroll scrl);
    extern void pdf_viewer_toggle_fullscreen(void);

    extern gchar *pdf_viewer_get_info(char *key);

    extern void pdf_viewer_get_state(AppState * app_state,
                                     gchar ** uri_str, gchar ** passwd);

    extern void pdf_viewer_unload(void);

    extern void pdf_viewer_empty_document(void);

    extern int pdf_viewer_get_zoom_percent(void);
    extern void pdf_viewer_move_after_fullscreen_togle(void);
    extern gboolean pdf_clicking_hyperlink(int x, int y);
    //extern void doAction(LinkAction *action);

    gboolean scrollbar_change_idle(gpointer app_ui_data);
    
    gboolean pdf_viewer_is_rendering();

    void pdf_viewer_cancel_if_render();

#ifdef __cplusplus
}
#endif
#endif                          /* PDFVIEWER_H */
