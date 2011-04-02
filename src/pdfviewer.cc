/**
    @file pdfviewer.cc

    Interface to Xpdf engine.

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


#include <aconf.h>
#include <math.h>
#include <sys/vfs.h>
#include <sys/types.h>
#include <fcntl.h>

#include "debug.h"
#include "gtk-switch.h"
#include <glib.h>
#include <glib/gthread.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <hildon/hildon-note.h>
#include "gtk-switch.h"
#include "SplashPattern.h"
#include "SplashTypes.h"
#include "ErrorCodes.h"
#include "GlobalParams.h"
#include "PDFDoc.h"
#include "Object.h"
#include "Dict.h"
#include "UnicodeMap.h"
#include "OssoOutputDev.h"
#include "OssoStream.h"

#include "pdfviewer.h"
#include "constant.h"
#include "ui/interface.h"
#include "i18n.h"
#include "utility.h"
#include "settings.h"

#include "thread_debug.h"

#define IS_RENDERING(p) (p->render_thread != NULL)

static unsigned int page_to_load;

struct _PDFViewerPrivate {
    AppUIData *app_ui_data;
    OssoOutputDev *output_dev;
    PDFDoc *pdf_doc;
    GThread *thread;

    GThread *render_thread;
    GThread *render_join_thread;    
    GThread *cancel_thread;

    unsigned int num_pages;
    unsigned int current_page;

    double dpi;
    PDFZoom zoom_level;

    /* Coordinates of the current view */
    gdouble x;
    gdouble y;
    guint scroll_x;
    guint scroll_y;

    /* required for state saving */
    GString *password;

    gboolean cancelled;
    gboolean is_mmc;
    gboolean is_gateway;
    gboolean need_show_info;
    gboolean cancel_render;
    gboolean abort_cancel;
    gchar *save_dst;

    GnomeVFSHandle *vfs_handle;
    GnomeVFSURI *vfs_URI;
    GnomeVFSURI *vfs_URI_gateway;
    GnomeVFSHandle *read_handle, *write_handle;
    gchar *uri_from_gateway;
    gchar *password_from_gateway;
};


/************************
 **** Private data
 **/
static _PDFViewerPrivate *priv = NULL;

int dpi_array[] = { 36,         /* 50% */
    72,                         /* 100% */
    108,                        /* 150% */
    144,                        /* 200% */
    180,                        /* 250% */
    216,                        /* 300% */
    288
};                              /* 400% */

static int zoom_numbers[] = { 50,
    100,
    150,
    200,
    250,
    300,
    400
};

/* the value from where we use partial rendering (in dpi) */
#define FULL_RENDER_DPI  dpi_array[DOC_ZOOM_400]

gboolean _pdf_abort_rendering = FALSE;

/************************
 **** Prototypes for private functions
 **/

static void display_page(void);
static void resize_layout(void);
static double get_custom_zoom_level(gboolean fit_width);
static void on_outputdev_redraw(void *user_data);
static GBool on_abort_check(void *user_data);
static gint on_progress_info(GnomeVFSXferProgressInfo * info, gpointer data);

static size_t custom_floor(const void *array,
                           const void *key,
                           size_t nmemb,
                           size_t width,
                           int (*compar) (const void *, const void *));


static size_t custom_ceil(const void *array,
                          const void *key,
                          size_t nmemb,
                          size_t width,
                          int (*compar) (const void *, const void *));

static void volume_unmounted_cb(GnomeVFSVolumeMonitor * vfsvolumemonitor,
                                GnomeVFSVolume * arg1, gpointer user_data);


static void empty_application_area(void);

static gint64 get_free_space(void);


/************************
 **** Private functions
 **/


/**
	Renders document page via PDFDoc displayPage
*/
static void
display_page()
{
    g_assert(priv->pdf_doc);
    
    TDB( "%s start",  __FUNCTION__ );

    PDF_FLAGS_SET(priv->app_ui_data->flags, PDF_FLAGS_RENDERING);

    /* partial rendering */
    if (priv->dpi > FULL_RENDER_DPI)
    {
        TDB("render\n");
#ifdef LOWMEM
        int buf_w, buf_h;

        if (!PDF_FLAGS_IS_SET(priv->app_ui_data->flags, PDF_FLAGS_FULLSCREEN))
        {
            buf_w = VIEWPORT_BUFFER_WIDTH;
            buf_h = VIEWPORT_BUFFER_HEIGHT;
        }
        else
        {
            buf_w = FULLSCREEN_BUFFER_WIDTH;
            buf_h = FULLSCREEN_BUFFER_HEIGHT;
        }
#endif

        priv->pdf_doc->displayPageSlice(priv->output_dev,
                                        priv->current_page, priv->dpi,
                                        priv->dpi, 0, gFalse,
                                        gFalse, gFalse,
                                        (int) priv->x, (int) priv->y,
#ifndef LOWMEM
                                        BUFFER_WIDTH, BUFFER_HEIGHT,
#else
                                        buf_w, buf_h,
#endif
                                        &on_abort_check, NULL);

        if (!priv->cancel_render)
        {
	    DTRY(gdk);
            GDK_THR_ENTER;
	    DLOCKED(gdk);
            gtk_layout_move(GTK_LAYOUT(priv->app_ui_data->layout),
                            priv->app_ui_data->page_image,
                            (int) priv->x, (int) priv->y);
            GDK_THR_LEAVE;
	    DUNLOCKED(gdk);
        }
    }
    else
    {
        TDB("render full: %p\n", priv->thread);
        try {
        	priv->pdf_doc->displayPage(priv->output_dev, priv->current_page,
            	priv->dpi, priv->dpi, 0, gFalse,
                gFalse, gFalse, &on_abort_check, NULL);
        } catch( int e ) {
		    PDF_FLAGS_UNSET(priv->app_ui_data->flags, PDF_FLAGS_RENDERING);
		    throw 0;
        }
        TDB("render full end\n");
    }
    if (globalParams->getBigImage() == gTrue)
    {
        globalParams->ackBigImage();
	DTRY(gdk);
        GDK_THR_ENTER;
	DLOCKED(gdk);
        ui_show_banner(GTK_WIDGET(priv->app_ui_data->app_view),
                       _("pdfv_ni_not_enough_memory_page"));
        GDK_THR_LEAVE;
	DUNLOCKED(gdk);
    }
    PDF_FLAGS_UNSET(priv->app_ui_data->flags, PDF_FLAGS_RENDERING);
    
    TDB( "%s end",  __FUNCTION__ );    
}

/**
 Helper function for page fit zoom and width fit zoom,
 to avoid unnecesseary variables allocations.
 
 @param TRUE if page width fit zoom, FALSE if page fit zoom
 @return ratio of the layout size and page size
*/
static double
get_custom_zoom_level(gboolean fit_width)
{
    gboolean ratio = FALSE;
    gint rotate = 0;
    double page_hsize = 0, page_vsize = 0;

    double screen_width =
        (double) priv->app_ui_data->scrolled_window->allocation.width;
    double screen_height =
        (double) priv->app_ui_data->scrolled_window->allocation.height;

    /* the page orientation is landscape or portrait? */
    rotate =
        priv->pdf_doc->getCatalog()->getPage(priv->current_page)->getRotate();

    /* get the dimensions of the document's current page */
    if (rotate == 90 || rotate == 270)
    {
        page_hsize = priv->pdf_doc->getPageCropHeight(priv->current_page);
        page_vsize = priv->pdf_doc->getPageCropWidth(priv->current_page);
    }
    else
    {
        page_hsize = priv->pdf_doc->getPageCropWidth(priv->current_page);
        page_vsize = priv->pdf_doc->getPageCropHeight(priv->current_page);
    }

    /* if the ratio of the current page is smaller than the ratio of screen
     * we have to consider the vertical scrollbar's size request. */
    if ((ratio =
         ((page_hsize / page_vsize) < (screen_width / screen_height))))
    {
        screen_width -= (double) SCROLLBAR_SIZE;
    }

    /* 
     * in case of fit to width request or if the the current page's ratio is
     * bigger than the request is like a 'fit to page' so return zoom_w! */
    if (fit_width || !ratio)
    {
    	double zoom_w = screen_width / page_hsize;
    	
    	/* Check for the scroller bars (now 40px is it good?) */
    	if( fit_width && screen_height < zoom_w * page_hsize ) {
    		zoom_w = ( screen_width - 40 ) / page_hsize;
    	}
        return zoom_w;
    }

    /* the request was a fit to page and ratio was smaller. */
    return screen_height / page_vsize;

}

static void
disable_all_ui()
{

    g_return_if_fail(priv->app_ui_data != NULL);

    ui_close_all_banners(priv->app_ui_data->app_data);

    ui_enable_document_open(priv->app_ui_data, FALSE);
    ui_enable_document_controls(priv->app_ui_data, FALSE);
    ui_enable_page_controls(priv->app_ui_data, DIM_ALL, FALSE);
	ui_scrollbars_sensitive( priv->app_ui_data, FALSE );
	
	g_debug( "%s done", __FUNCTION__ );
        
}

static void
enable_all_ui()
{

    /* adjust page buttons */
    ui_enable_page_controls(priv->app_ui_data, DIM_PREV,
                            (priv->current_page == 1 ? FALSE : TRUE));
    ui_enable_page_controls(priv->app_ui_data, DIM_NEXT,
                            (priv->num_pages ==
                             priv->current_page ? FALSE : TRUE));
    ui_enable_page_controls(priv->app_ui_data, DIM_SWITCH_TO,
                            (priv->num_pages == 1 ? FALSE : TRUE));
	ui_scrollbars_sensitive( priv->app_ui_data, TRUE );

    /* Enable/disable zoom controls */
    if (priv->dpi <= dpi_array[DOC_ZOOM_50])
    {
        if (ui_get_toolitem_is_sensitive(priv->app_ui_data, DIM_ZOOM_OUT))
            ui_enable_page_controls(priv->app_ui_data, DIM_ZOOM_OUT, FALSE);

        if (!ui_get_toolitem_is_sensitive(priv->app_ui_data, DIM_ZOOM_IN))
            ui_enable_page_controls(priv->app_ui_data, DIM_ZOOM_IN, TRUE);
    }
    else if (priv->dpi < dpi_array[DOC_ZOOM_400])
    {
        if (!ui_get_toolitem_is_sensitive(priv->app_ui_data, DIM_ZOOM_OUT))
            ui_enable_page_controls(priv->app_ui_data, DIM_ZOOM_OUT, TRUE);

        if (!ui_get_toolitem_is_sensitive(priv->app_ui_data, DIM_ZOOM_IN))
            ui_enable_page_controls(priv->app_ui_data, DIM_ZOOM_IN, TRUE);
    }
    else
    {
        if (!ui_get_toolitem_is_sensitive(priv->app_ui_data, DIM_ZOOM_OUT))
            ui_enable_page_controls(priv->app_ui_data, DIM_ZOOM_OUT, TRUE);

        if (ui_get_toolitem_is_sensitive(priv->app_ui_data, DIM_ZOOM_IN))
            ui_enable_page_controls(priv->app_ui_data, DIM_ZOOM_IN, FALSE);
    }
    if (PDF_FLAGS_IS_SET(priv->app_ui_data->flags, PDF_FLAGS_PAGE_ERROR))
    {
	ui_enable_page_controls(priv->app_ui_data, DIM_ZOOM_IN, FALSE);
    }
    ui_enable_document_open(priv->app_ui_data, TRUE);
    ui_enable_document_controls(priv->app_ui_data, TRUE);

    ui_close_all_banners(priv->app_ui_data->app_data);

    if (priv->need_show_info)
    {
        priv->need_show_info = FALSE;

        if (priv->current_page == 1)
            ui_show_banner(GTK_WIDGET(priv->app_ui_data->app_view),
                           _("pdfv_ib_first_page_reached"));
        else if (priv->current_page == priv->num_pages)
            ui_show_banner(GTK_WIDGET(priv->app_ui_data->app_view),
                           _("pdfv_ib_last_page_reached"));
    }

    priv->app_ui_data->app_data->state = PDF_VIEWER_STATE_LOADED;
    
	g_debug( "%s done", __FUNCTION__  );    
    
}

G_LOCK_DEFINE_STATIC(cancel_mutex);
G_LOCK_DEFINE_STATIC(redraw_mutex);

static gpointer
render_page_func(gpointer data)
{

    TDB("render_page_func begin\n");
    priv->cancel_render = FALSE;

    // If displaying page fails show empty page
    try {
        if (get_free_space() == 0) {
            // rendering with full storage causes crash, avoid it
            g_warning( "Not enough memory on flash." );
            throw (0);
        }
    	display_page();
    } catch( int e ) {
    	fprintf( stderr, "%s: Can't display page\n", __FUNCTION__ );
	    empty_application_area();
	    
	    // This should only happen when there isn't enough memory to show
	    // page. Show let's so banner for that. Fix this if exception is later
	    // used for other signals
        GDK_THR_ENTER;	    
    	ui_show_banner(GTK_WIDGET(priv->app_ui_data->app_view),
			_("pdfv_ni_not_enough_memory_page"));		
        enable_all_ui();

	 /* bug: 80343 STARTS */
	GtkWidget* widget = gtk_ui_manager_get_widget(priv->app_ui_data->ui_manager,
                                       "/MenuBar/pdfv_me_main_menu_view"
                                       "/pdfv_me_menu_screen_show_images");
        if (widget != NULL)
        	gtk_widget_set_sensitive(widget, FALSE);

    	widget = gtk_ui_manager_get_widget(priv->app_ui_data->ui_manager,
                                       "/ToolBar/"
                                       "pdfv_me_menu_screen_show_images");
   	 if (widget != NULL)
        	gtk_widget_set_sensitive(widget, FALSE);
	 /* bug: 80343 ENDS */
	 
        // Disable zoom in because it will fail next time too
        ui_enable_page_controls( priv->app_ui_data, DIM_ZOOM_IN, FALSE );
        // Disable zoom out because it will fail next time too
        ui_enable_page_controls( priv->app_ui_data, DIM_ZOOM_OUT, FALSE );
        GDK_THR_LEAVE;
        priv->render_thread = NULL;      
        return NULL;
    }
 	
    if (!priv->cancel_render)
    {
		DTRY(gdk);
        GDK_THR_ENTER;
		DLOCKED(gdk);
        resize_layout();
        enable_all_ui();
        GDK_THR_LEAVE;
		DUNLOCKED(gdk);
    }
    
    priv->render_thread = NULL;
    TDB("render_page_func end\n");

    return NULL;
}

static void
create_rendering() {
    if( priv->render_join_thread ) {
        g_thread_join( priv->render_join_thread );
    }
    priv->render_join_thread = priv->render_thread =
    	g_thread_create(render_page_func, NULL, TRUE, NULL);
    if( priv->render_thread == NULL ) {
        g_critical( "Can't create render thread" );
    }	
}

static gpointer
cancel_rendering_func(gpointer data)
{
    TDB("Cancel rendering\n");
    DTRY(cancel_mutex#1);
    G_LOCK(cancel_mutex);
    DLOCKED(cancel_mutex#1);
    priv->cancel_render = TRUE;
    priv->abort_cancel = FALSE;
    G_UNLOCK(cancel_mutex);
    DUNLOCKED(cancel_mutex#1);
    if (priv->render_thread)
    {
        g_thread_join(priv->render_join_thread);
        priv->render_join_thread = NULL;
    }
    DTRY(cancel_mutex#2);
    G_LOCK(cancel_mutex);
    DLOCKED(cancel_mutex#2);
    if (!priv->abort_cancel)
        create_rendering();
    priv->cancel_thread = NULL;
    G_UNLOCK(cancel_mutex);
    DUNLOCKED(cancel_mutex#2);
    TDB("Cancel rendering done\n");

    return NULL;
}

static void
render_page()
{
    DTRY(cancel_mutex);
    G_LOCK(cancel_mutex);
    DLOCKED(cancel_mutex);
    if( priv->render_thread != NULL ) {
        if (!priv->cancel_thread) {
            priv->cancel_thread =
            	g_thread_create(cancel_rendering_func, NULL, FALSE, NULL);
        }
    } else {
        create_rendering();
    }
    G_UNLOCK(cancel_mutex);
    DUNLOCKED(cancel_mutex);
}

static void
cancel_if_render()
{
    TDB("Cancel if render\n");
    DTRY(cancel_mutex);
    G_LOCK(cancel_mutex);
    DLOCKED(cancel_mutex);
    TDB("Cancel if render1\n");
    priv->abort_cancel = TRUE;
    priv->cancel_render = TRUE;
    TDB("Cancel if render2: %d\n", IS_RENDERING(priv));
    G_UNLOCK(cancel_mutex);
    DUNLOCKED(cancel_mutex);

    while( priv->render_thread != NULL )
        usleep(50000);
    TDB("Cancel if render done\n");
}

/**
	OutputDev redraw callback.
	Called when page has been internally rendered using Splash.
	Calls OssoOutputDev redraw to copy internal buffer to display.

	@return void
*/
static void
on_outputdev_redraw(void *user_data)
{
    AppUIData *app_ui_data;
    app_ui_data = (AppUIData *) user_data;
    g_assert(app_ui_data);
    if (!_pdf_abort_rendering && !priv->cancel_render)
    {
        TDB("on_outputdev_redraw1\n");
        DTRY(redraw_mutex);
        G_LOCK(redraw_mutex);
        DLOCKED(redraw_mutex);
        DTRY(gdk);
        gdk_threads_enter();
        DLOCKED(gdk);
        if (priv->app_ui_data->arrow_left_id)
        {
            g_source_remove(priv->app_ui_data->arrow_left_id);
            priv->app_ui_data->arrow_left_id = 0;
        }
        if (priv->app_ui_data->arrow_right_id)
        {
            g_source_remove(priv->app_ui_data->arrow_right_id);
            priv->app_ui_data->arrow_right_id = 0;
        }
        ui_arrow_hide(priv->app_ui_data);
        gdk_threads_leave();
        DUNLOCKED(gdk);
        priv->output_dev->redraw(app_ui_data);
        G_UNLOCK(redraw_mutex);
        DUNLOCKED(redraw_mutex);
        TDB("on_outputdev_redraw2\n");
    }
}

static void
calc_size_dpi(guint dpi, guint * width, guint * height,
              guint * screen_w, guint * screen_h)
{
    guint _width, _height, rotate;
    gint current_width, current_height;

    g_return_if_fail(priv->pdf_doc != NULL);

    /* calculate the needed size of the layout */
    _width = (guint) (priv->pdf_doc->getPageCropWidth(priv->current_page) *
                      (dpi / (double) SCREEN_DPI));
    _height = (guint) (priv->pdf_doc->getPageCropHeight(priv->current_page) *
                       (dpi / (double) SCREEN_DPI));

    /* the page orientation is landscape or portrait? */
    rotate =
        priv->pdf_doc->getCatalog()->getPage(priv->current_page)->getRotate();

    /* in case of landscape exchange width with height */
    if (rotate == 90 || rotate == 270)
    {
        _width ^= _height;
        _height ^= _width;
        _width ^= _height;
    }

    if (width)
        *width = _width;
    if (height)
        *height = _height;

    /* Get the current widht and height of the application actual window*/
    gtk_window_get_size(GTK_WINDOW(priv->app_ui_data->app_view), &current_width, &current_height);
#if 0    
    /* 
     * get the screen's dimensions. the scrollbars' size DOES count in the
     * actual area! */
    if (PDF_FLAGS_IS_SET(priv->app_ui_data->flags, PDF_FLAGS_FULLSCREEN))
    {
        if (screen_w)
            *screen_w =
                current_width -
                ((_height < current_height) ? 0 : SCROLLBAR_SIZE);
        if (screen_h)
            *screen_h =
                current_height -
                ((_width < current_width) ? 0 : SCROLLBAR_SIZE);
    }
    else
    {
        if (screen_w)
            *screen_w =
                current_width -
                ((_height < current_height) ? 0 : SCROLLBAR_SIZE);
        if (screen_h)
            *screen_h =
                current_height -
                ((_width < current_width) ? 0 : SCROLLBAR_SIZE);
    }
#endif
    /* screen width/height is fetched from current window size 
     * it will take care of both normal and fullscreen mode */
        if (screen_w)
            *screen_w = current_width;

        if (screen_h)
            *screen_h = current_height;
}

static void
adjust_focus_point(guint old_dpi)
{
    guint old_width, old_height, width, height;
    guint old_sw, old_sh, sw, sh;
    guint old_x, old_y, x, y;
    guint scx, scy;

    calc_size_dpi(old_dpi, &old_width, &old_height, &old_sw, &old_sh);
    calc_size_dpi((guint) priv->dpi, &width, &height, &sw, &sh);

    TDB("Adjust focus: old (%d, %d)-(%d, %d)\n", old_width, old_height,
        old_sw, old_sh);
    TDB("Adjust focus: new (%d, %d)-(%d, %d)\n", width, height, sw, sh);

    scx = (guint) gtk_range_get_value(GTK_RANGE(priv->app_ui_data->hscroll));
    scy = (guint) gtk_range_get_value(GTK_RANGE(priv->app_ui_data->vscroll));
    TDB("Adjustment: (%d, %d)\n", scx, scy);
    old_x = old_width < old_sw ? old_width / 2 : (scx + old_sw / 2);
    old_y = old_height < old_sh ? old_height / 2 : (scy + old_sh / 2);

    TDB("Old center: (%d, %d)\n", old_x, old_y);
    x = (guint) ((gdouble) old_x * (gdouble) priv->dpi / (gdouble) old_dpi);
    y = (guint) ((gdouble) old_y * (gdouble) priv->dpi / (gdouble) old_dpi);
    TDB("New center: (%d, %d)\n", x, y);

    if (priv->dpi > FULL_RENDER_DPI)
    {
        priv->x = x < BUFFER_WIDTH / 2 ? 0 : x - BUFFER_WIDTH / 2;
        priv->y = y < BUFFER_HEIGHT / 2 ? 0 : y - BUFFER_HEIGHT / 2;
        TDB("Priv left: (%d, %d)\n", (gint) priv->x, (gint) priv->y);
    }

    priv->scroll_x = x < sw / 2 ? 0 : x - sw / 2;
    priv->scroll_y = y < sh / 2 ? 0 : y - sh / 2;

    TDB("Top left: (%d, %d)\n", priv->scroll_x, priv->scroll_y);
}

/**
	Resizes the GtkLayout in case of partial rendering.
	We need to resize the gtklayout size, since in case of
	partial rendering we still need a proper scroller
	
	@return void
*/
static void
resize_layout()
{
    guint width, height, layout_w, layout_h;
    guint screen_w, screen_h;

    g_return_if_fail(priv->pdf_doc != NULL);

    if (PDF_FLAGS_IS_SET(priv->app_ui_data->flags, PDF_FLAGS_PAGE_ERROR)
        /* || priv->app_ui_data->app_data->low_memory */ )
    {
        return;
    }

    TDB("Resizing layout...\n");

    /* get the current size of the layout */
    gtk_layout_get_size(GTK_LAYOUT(priv->app_ui_data->layout),
                        &layout_w, &layout_h);

    calc_size_dpi((guint) priv->dpi, &width, &height, &screen_w, &screen_h);

    /* enable or disable scrollbars as necessary */
    ui_enable_scrollbars(priv->app_ui_data,
                         (width > screen_w), (height > screen_h));

    /* center the document */
    if (priv->dpi <= FULL_RENDER_DPI)
    {
 
    	if (PDF_FLAGS_IS_SET(priv->app_ui_data->flags, PDF_FLAGS_FULLSCREEN))
    	{
     	    gtk_layout_move(GTK_LAYOUT(priv->app_ui_data->layout),
                        priv->app_ui_data->page_image,
                        (screen_w < (screen_w - width) / 2)
                        ? 0 : (screen_w - width) / 2,
                        (screen_h < (screen_h - height) / 2)
                        ? 0 : (screen_h - height) / 2);
    	}
    	else
    	{
     	    gtk_layout_move(GTK_LAYOUT(priv->app_ui_data->layout),
                        priv->app_ui_data->page_image,
                        (screen_w < (screen_w - width) / 2)
                        ? 0 : (screen_w - width) / 2, 0);
   	}
    }
    else
    {
        gtk_layout_move(GTK_LAYOUT(priv->app_ui_data->layout),
                        priv->app_ui_data->page_image,
                        (int) priv->x, (int) priv->y);
    }

    /* resize the layout if needed */
    if (layout_w != width || layout_h != height)
    {
        gtk_layout_set_size(GTK_LAYOUT(priv->app_ui_data->layout),
                            width, height);
    }

    TDB("Resizing layout done...\n");
}

void
pdf_viewer_move_after_fullscreen_togle(void)
{
    if (PDF_FLAGS_IS_SET(priv->app_ui_data->flags, PDF_FLAGS_FULLSCREEN))
    {     
    
    	/* Togled to fullscreen mode */
    	gint x, y;

        x = (gint)gtk_range_get_value(GTK_RANGE(priv->app_ui_data->hscroll))
            - (FULLSCREEN_WIDTH - VIEWPORT_WIDTH) / 2;
        if (x < 0)
            priv->scroll_x = 0;
		else
        	priv->scroll_x = x;

        y = (gint)gtk_range_get_value(GTK_RANGE(priv->app_ui_data->vscroll))
            - (FULLSCREEN_HEIGHT - VIEWPORT_HEIGHT) / 2;
        if (y < 0)
            priv->scroll_y = 0;
		else
	        priv->scroll_y = y;
    }
    else
    {                           /* out from fullscreen mode */
        priv->scroll_x =
            (guint) gtk_range_get_value(GTK_RANGE(priv->app_ui_data->hscroll))
            + (FULLSCREEN_WIDTH - VIEWPORT_WIDTH) / 2;
        priv->scroll_y =
            (guint) gtk_range_get_value(GTK_RANGE(priv->app_ui_data->vscroll))
            + (FULLSCREEN_HEIGHT - VIEWPORT_HEIGHT) / 2;
    }
}

/**
   Abort checker callback function 
   Used for passing to the PDFDoc->display() function, so in case
   of errors while rendering a page we can stop the actual rendering.

   @param 
   @return GBool - if TRUE the page rendering stops,
                   if FALSE the page rendering continues.
*/
static GBool
on_abort_check(void *user_data)
{
    GBool return_val = gFalse;

    g_return_val_if_fail(priv->app_ui_data != NULL, gTrue);
    g_return_val_if_fail(priv->app_ui_data->app_data != NULL, gTrue);

    TDB("Abort check....\n");
    /* there was while rendering page */
    /* and when loading? */
    if (priv->app_ui_data->app_data->low_memory)
//        && (priv->app_ui_data->app_data->state != PDF_VIEWER_STATE_LOADING))
    {

        PDF_FLAGS_SET(priv->app_ui_data->flags, PDF_FLAGS_PAGE_ERROR);

        priv->app_ui_data->app_data->low_memory = FALSE;

        // show information banner 
	DTRY(gdk);
	gdk_threads_enter();
	DLOCKED(gdk);
	ui_show_banner(GTK_WIDGET(priv->app_ui_data->app_view),
                       _("pdfv_ni_not_enough_memory_page"));
	gdk_threads_leave();
	DUNLOCKED(gdk);

        // empty application area 
        empty_application_area();

        // disable zoom controls 
//	gdk_threads_enter();
//        ui_enable_page_controls(priv->app_ui_data, DIM_ERROR, FALSE);
//	gdk_threads_leave();

        return_val = gTrue;
    }
    return return_val || priv->cancel_render;
}


/** 
    Callback function for saving method.
    Checks if saving lasts more than SAVE_TIMEOUT, if yes
    a "Saving..." banner appears.

    @param GnomeVFSXferProgressInfo
    @param data - GTimer from pdf_viewer_save
    @return gint - 0 to abort the saving.
*/

static GtkWidget *pb_banner = NULL;

static gint
on_progress_info(GnomeVFSXferProgressInfo * info, gpointer data)
{
    GTimer *timer = *(GTimer **) data;

    // g_return_val_if_fail(timer != NULL, FALSE);

    if (timer && SAVE_TIMEOUT <= g_timer_elapsed(timer, NULL))
    {
        /* print out 'Saving' banner */
        pb_banner =
            ui_show_progress_banner(GTK_WINDOW(priv->app_ui_data->app_view),
                                    _("pdfv_ib_saving"));
        priv->app_ui_data->saving_banner = pb_banner;
        g_timer_destroy(timer);
        *(GTimer **) data = NULL;
    }

    while (gtk_events_pending())
        gtk_main_iteration();

    return 1;
}

/**
   Returns the nearest (from down) value to the given key
   in the given array.

   @param array of elements
   @param key element
   @param number of elements in the array
   @param size of one element
   @param pointer on the compare function
   @return index of the nearest element

*/

static size_t
custom_floor(const void *array,
             const void *key,
             size_t nmemb,
             size_t width, int (*compar) (const void *, const void *))
{
    void *current;
    size_t lower = 0;
    size_t upper = nmemb;
    size_t index;
    int result;

    /* check input */


    /* obvious case */
    index = nmemb;
    current = (void *) (((char *) array) + ((index - 1) * width));  // last
                                                                    // element
    if (0 < compar(key, current))
        return --index;

    /* binary search */
    while (lower < upper)
    {
        index = (lower + upper) / 2;

        current = (void *) (((char *) array) + (index * width));
        result = compar(key, current);

        if (result < 0)
            upper = index;
        else if (result > 0)
            lower = index + 1;
        else
            return (0 < index) ? --index : index;
    }
    return (0 < lower) ? --lower : lower;
}

/**
   Returns the nearest (from up) value to the given key
   in the given array.

   @param array of elements
   @param key element
   @param number of elements in the array
   @param size of one element
   @param pointer on the compare function
   @return index of the nearest element

*/
static size_t
custom_ceil(const void *array,
            const void *key,
            size_t nmemb,
            size_t width, int (*compar) (const void *, const void *))
{
    void *current;
    size_t lower = 0;
    size_t upper = nmemb;
    size_t index;
    int result;

    /* check input */


    /* obvious case */
    current = (void *) ((char *) array);    // first element
    if (compar(key, current) < 0)
        return 0;

    /* binary search */
    while (lower < upper)
    {
        index = (lower + upper) / 2;

        current = (void *) (((char *) array) + (index * width));
        result = compar(key, current);

        if (result < 0)
            upper = index;
        else if (result > 0)
            lower = index + 1;
        else
            return (index < nmemb - 1) ? ++index : index;
    }
    return (upper < nmemb) ? upper : index;
}


static void
volume_unmounted_cb(GnomeVFSVolumeMonitor * vfsvolumemonitor,
                    GnomeVFSVolume * volume, gpointer user_data)
{
    AppData *appdata = (AppData *) user_data;
    gchar *uri = NULL;
    gchar *volume_uri = NULL;

    g_return_if_fail(priv != NULL);
    g_return_if_fail(appdata != NULL);
    /* maybe not */
    g_return_if_fail(priv->pdf_doc != NULL);

    // /system/osso/af/usb-cable-attached
    TDB("Volume unmounted2\n");

    /* get the URI regarding to state */
    switch (appdata->state)
    {
        case PDF_VIEWER_STATE_LOADED:
        case PDF_VIEWER_STATE_LOADING:
        case PDF_VIEWER_STATE_SAVING:
            uri =
                gnome_vfs_uri_to_string(pdf_viewer_get_uri(),
                                        GNOME_VFS_URI_HIDE_NONE);
            break;
        default:
            uri = NULL;
            break;
    }

    if (uri != NULL)
    {
        /* the mount point of the changed volume */
        volume_uri = gnome_vfs_volume_get_activation_uri(volume);
    }

    /* check if unmounting this volume has anything to do with us */
    if (volume_uri)
    {
        if (g_str_has_prefix(uri, volume_uri))
        {
            // if (appdata->state != PDF_VIEWER_STATE_LOADING)
            if (settings_get_bool("/system/osso/af/usb-cable-attached"))
            {
                if (appdata->state == PDF_VIEWER_STATE_LOADING
                    || appdata->state == PDF_VIEWER_STATE_SAVING)
                    ui_show_banner(GTK_WIDGET(priv->app_ui_data->app_view),
                                   _("mmc_ib_please_wait"));
            }
            else
                pdf_viewer_unload();
            /* else { cancel_if_render(); priv->is_mmc = (appdata->mmc_uri &&
             * g_str_has_prefix(uri, appdata->mmc_uri)); priv->cancelled =
             * TRUE; } */
        }

        if (priv->save_dst && g_str_has_prefix(priv->save_dst, volume_uri))
        {
            if (settings_get_bool("/system/osso/af/usb-cable-attached"))
                ui_show_banner(GTK_WIDGET(priv->app_ui_data->app_view),
                               _("mmc_ib_please_wait"));
            else
            {
                priv->is_mmc = (appdata->mmc_uri &&
                                g_str_has_prefix(priv->save_dst,
                                                 appdata->mmc_uri));
                priv->cancelled = TRUE;
            }
        }
    }

    if (uri)
        g_free(uri);
    if (volume_uri)
        g_free(volume_uri);
}

/**
   Helper function.
   Clears application area
*/
static void
empty_application_area(void)
{
    g_return_if_fail(priv != NULL);

    /* document is already loaded */
    if (priv->pdf_doc
        || (priv->app_ui_data->app_data->state == PDF_VIEWER_STATE_LOADED))
    {

        /* reinitialize output device */
        priv->output_dev->startPage(0, NULL);
        /* redraw the current page (blank page) */
        DTRY(redraw_mutex);
        G_LOCK(redraw_mutex);
        DLOCKED(redraw_mutex);
        priv->output_dev->redraw(priv->app_ui_data);
        G_UNLOCK(redraw_mutex);
        DUNLOCKED(redraw_mutex);
    }
}


int
pdf_viewer_get_zoom_percent(void)
{
    if (priv->zoom_level >= 0 && dpi_array[priv->zoom_level] == priv->dpi)
    {
        return zoom_numbers[priv->zoom_level];
    }
    else
    {
        /* get the custom value for custom dpi */
        return (int) floor((priv->dpi / (double) SCREEN_DPI) * 100);
    }
}

int
compare(const void *a, const void *b)
{
    return (*(int *) a - *(int *) b);
}

/**
   Initialize xpdf eninge.
   Initializes the xpdf engine in a joinable thread.
   (@see pdf_viewer_init():g_thread_create)
*/
static gpointer
init_thread_func(gpointer data)
{
    AppUIData *app_ui_data;
    SplashColor paperColor;
    const gchar *mmc_env = NULL;

    /* check input */
    app_ui_data = (AppUIData *) data;
    g_return_val_if_fail(app_ui_data != NULL, NULL);
    g_return_val_if_fail(app_ui_data->app_data != NULL, NULL);

    g_return_val_if_fail(priv != NULL, NULL);

    /* setting xpdf engine parameters */
    if (!globalParams)
    {
        globalParams = new GlobalParams((char *)"");
        globalParams->setEnableFreeType((char *)"yes");
        globalParams->setAntialias((char *)"yes");
    }

    /* paper color = white */
    paperColor[0] = paperColor[1] = paperColor[2] = 0xff;

    /* creating output device for xpdf */
    priv->output_dev = new OssoOutputDev(gFalse, paperColor, gFalse,
                                         65536, gFalse,
                                         &on_outputdev_redraw, app_ui_data);

    /* set where the MMC is mounted */
    mmc_env = g_getenv(MMC_MOUNTPOINT_ENV);
    if (mmc_env)
    {
        app_ui_data->app_data->mmc_uri =
            g_strconcat(URI_FILE_PREFIX, mmc_env, NULL);
    }

    /* Volume monitor (singleton object, no refcounting) */
    app_ui_data->app_data->volume_monitor = gnome_vfs_get_volume_monitor();
    g_signal_connect(G_OBJECT(app_ui_data->app_data->volume_monitor),
                     "volume_pre_unmount",
                     G_CALLBACK(volume_unmounted_cb), app_ui_data->app_data);
    return NULL;
}

/**
  
*/
static gint64
get_free_space()
{
    struct statfs info;
    int status = -1;
    gint64 free = -1;

    status = statfs(TEMP_DIR_PATH, &info);

    if (status == 0) {
        free = (((gint64) info.f_bsize) * ((gint64) info.f_bavail)) / KB_SIZE;
    } else {
        g_warning( "Getting free space for '%s' failed.", TEMP_DIR_PATH );
    }

    if (free > 0) {
        if (free < RESERVED_SPACE)
            free = 0;
        else
            free -= RESERVED_SPACE;
    } else {
    	free = 0;
    }

	g_debug( "%s: %d", __FUNCTION__, (gint)free );
    return free;
}

/************************
 **** Public functions
 **/

/**
	Initializes module

	@param app_ui_data AppUIData structure
	@return void
*/
void
pdf_viewer_init(AppUIData * app_ui_data)
{
    AppData *appdata;
    AppState app_state;
    gchar *state_uri = NULL;
    gchar *passwd = NULL;
    StateSaveResultCode state_res;

    g_return_if_fail(app_ui_data != NULL);
    g_return_if_fail(app_ui_data->app_data != NULL);
    appdata = app_ui_data->app_data;

    /* Initialize private structure */
    priv = g_new0(_PDFViewerPrivate, 1);
    if (priv == NULL)
    {
        OSSO_LOG_CRIT
            ("Memory allocation for 'PDFViewerPrivate' structure failed");
        return;
    }

    // xInitMutex(&priv->cancel_mutex);
    priv->app_ui_data = app_ui_data;
    priv->thread = g_thread_create(init_thread_func, app_ui_data, TRUE, NULL);

    /* state loading is not in thread because D-BUS dies with it! */

    /* check state startup mode DEFAULT/STATE SAVE */
    if (appdata->mode != STARTUP_MODE_URI_REQUEST)
    {
        memset(&app_state, 0, sizeof(AppState));

        /* read saved application state */
        state_res = read_app_state(appdata, &app_state, &state_uri, &passwd);

        /* there was a valid app_state saved */
        if ((state_res == ST_SUCCESS) && file_is_supported(state_uri))
        {
            appdata->mode = STARTUP_MODE_STATE_DATA;
            page_to_load = app_state.current_page;
            TDB("Page tom load: %u\n", page_to_load);
        }
        else
        {
            appdata->mode = STARTUP_MODE_DEFAULT;
            page_to_load = 0;
            TDB("Page tom load: %u\n", page_to_load);
        }
    }

    /* Initialize private structure */
    if (appdata->mode != STARTUP_MODE_STATE_DATA)
    {
        priv->dpi = dpi_array[DOC_ZOOM_100];
        priv->zoom_level = DOC_ZOOM_100;

        priv->num_pages = PDF_PAGE_INIT;
        priv->current_page = PDF_PAGE_INIT;
        priv->x = priv->y = 0;

        priv->password = NULL;

        TDB(("state09\n"));
        /* wait up for the thread! */
        if (priv->thread != NULL)
        {
            g_thread_join(priv->thread);
            priv->thread = NULL;
        }

        TDB(("state11\n"));
        globalParams->
            setShowImages(PDF_FLAGS_IS_SET
                          (priv->app_ui_data->flags, PDF_FLAGS_SHOW_IMAGES));

        appdata->state = PDF_VIEWER_STATE_EMPTY;
    }
    else
    {

        TDB(("state12\n"));
        /* set saved state */
        priv->current_page = app_state.current_page;
        TDB("state12: %ud\n", priv->current_page);
        priv->dpi = (double) app_state.dpi;
        priv->zoom_level = (PDFZoom) app_state.zoom_level;
        priv->x = app_state.scroll_hadj;
        priv->y = app_state.scroll_vadj;

        /* syncronize UI */
        ui_load_state(app_ui_data, &app_state);

        TDB(("state13.1\n"));
        /* wait up for the thread! */
        if (priv->thread != NULL)
        {
            TDB(("state13.2\n"));
            g_thread_join(priv->thread);
            priv->thread = NULL;
        }

        TDB(("state14\n"));
        /* moved from above */
        globalParams->setShowImages(app_state.show_images);


        TDB(("state15\n"));
        /* load file */
        ui_open_document(app_ui_data, state_uri, passwd);

        /* reset startup mode to default */
        appdata->mode = STARTUP_MODE_DEFAULT;

        g_free(state_uri);
        if (passwd)
            g_free(passwd);
    }

    /* set current zoom widget value */
    ui_set_current_zoom(app_ui_data, pdf_viewer_get_zoom_percent());

}


/**
	Deinitializes module, freeing all allocated memory

	@return void
*/
void
pdf_viewer_deinit()
{
    gint gatewaypdf_handle = 0;

    if (priv->pdf_doc != NULL)
    {
        delete priv->pdf_doc;
        priv->pdf_doc = NULL;
    }

    if (priv->output_dev != NULL)
    {
        priv->output_dev->clear();
        delete priv->output_dev;
        priv->output_dev = NULL;
    }

    if (globalParams != NULL)
    {
        delete globalParams;
        globalParams = NULL;
    }

    if (priv->password != NULL)
    {
        delete priv->password;
        priv->password = NULL;
    }

    if (priv->vfs_handle)
    {
        gnome_vfs_close(priv->vfs_handle);
        priv->vfs_handle = NULL;
    }

    if (priv->vfs_URI)
    {
        gnome_vfs_uri_unref(priv->vfs_URI);
        priv->vfs_URI = NULL;
    }

    if (priv->vfs_URI_gateway)
    {
        gnome_vfs_uri_unref(priv->vfs_URI_gateway);
        priv->vfs_URI_gateway = NULL;
    }

    // xDestroyMutex(&priv->cancel_mutex);
    if (priv != NULL)
    {
        g_free(priv);
        priv = NULL;
    }

    /* Remove temporary gateway PDF file, if exists */
    gatewaypdf_handle = open(GATEWAY_TMP_FILE, O_RDONLY);
    if (gatewaypdf_handle != -1)
    {
        close(gatewaypdf_handle);
        remove(GATEWAY_TMP_FILE);
    }

    OSSO_LOG_DEBUG("Engine deinitialization finished successfully");
}

void
pdf_viewer_empty_document() 
{
	g_debug( "%s start", __FUNCTION__ );

    cancel_if_render();

	/* g_debug( "%s 1", __FUNCTION__ ); */
    empty_application_area();

	/* g_debug( "%s 2", __FUNCTION__ ); */
    if (priv->pdf_doc) {
        delete priv->pdf_doc;
        priv->pdf_doc = 0;
    }
    
	/* g_debug( "%s 3", __FUNCTION__ ); */
    if (priv->vfs_URI) {
        gnome_vfs_uri_unref(priv->vfs_URI);
        priv->vfs_URI = 0;
    }
    if (priv->vfs_URI_gateway) {
        gnome_vfs_uri_unref(priv->vfs_URI_gateway);
        priv->vfs_URI_gateway = 0;
    }

	/* g_debug( "%s 4", __FUNCTION__ ); */
    priv->app_ui_data->app_data->state = PDF_VIEWER_STATE_EMPTY;
    priv->num_pages = PDF_PAGE_INIT;
    priv->current_page = PDF_PAGE_INIT;
    priv->x = priv->y = 0;

    if (priv->write_handle)
    {
        gnome_vfs_close(priv->write_handle);
        priv->write_handle= NULL;
    }

    if (priv->read_handle)
    {
        gnome_vfs_close(priv->read_handle);
        priv->read_handle = NULL;
    }

    if (priv->vfs_handle)
    {
        gnome_vfs_close(priv->vfs_handle);
        priv->vfs_handle = NULL;
    }

	/* g_debug( "%s 5", __FUNCTION__ ); */
    ui_enable_document_open(priv->app_ui_data, TRUE);

	/* g_debug( "%s 6", __FUNCTION__ ); */
    ui_update(priv->app_ui_data);

    g_debug( "%s done\n", __FUNCTION__ );
}

static gboolean
pdf_viewer_copy_from_gw(gpointer data)
{
    static GnomeVFSFileSize bytes_read, bytes_written;
    static GnomeVFSResult vfs_result;
    gchar buffer[128 * 1024];
    TDB("%s called\n", __FUNCTION__);
    if (((!priv->app_ui_data->close_called))
        &&
        (gnome_vfs_read
         (priv->read_handle, buffer, sizeof(buffer),
          &bytes_read) == GNOME_VFS_OK))
    {
        vfs_result =
            gnome_vfs_write(priv->write_handle, buffer, bytes_read,
                            &bytes_written);
        if (vfs_result != GNOME_VFS_OK)
        {
            TDB(("write not ok\n"));
            if (priv->read_handle)
            {
                gnome_vfs_close(priv->read_handle);
                priv->read_handle = NULL;
            }
            if (priv->write_handle)
            {
                gnome_vfs_close(priv->write_handle);
                priv->write_handle = NULL;
            }
            gint gatewaypdf_handle = open(GATEWAY_TMP_FILE, O_RDONLY);
            if (gatewaypdf_handle != -1)
            {
                close(gatewaypdf_handle);
                remove(GATEWAY_TMP_FILE);
            }

	    DTRY(gdk);
            GDK_THR_ENTER;
	    DLOCKED(gdk);
            ui_show_result(priv->app_ui_data, RESULT_INSUFFICIENT_MEMORY);
            GDK_THR_LEAVE;
	    DUNLOCKED(gdk);
            return FALSE;
        }
        else
        {                       // write ok... putting this to idle again
            return TRUE;
        }
    }
    else
    {
        if (priv->app_ui_data->close_called)
        {
            if (priv->read_handle)
            {
                gnome_vfs_close(priv->read_handle);
                priv->read_handle = NULL;
            }
            if (priv->write_handle)
            {
                gnome_vfs_close(priv->write_handle);
                priv->write_handle = NULL;
            }
            gint gatewaypdf_handle = open(GATEWAY_TMP_FILE, O_RDONLY);
            if (gatewaypdf_handle != -1)
            {
                close(gatewaypdf_handle);
                remove(GATEWAY_TMP_FILE);
            }
            idle_delete(priv->app_ui_data->open_document_structure);
            return FALSE;
        }
        if (priv->read_handle)
        {
            gnome_vfs_close(priv->read_handle);
            priv->read_handle = NULL;
        }
        if (priv->write_handle)
        {
            gnome_vfs_close(priv->write_handle);
            priv->write_handle = NULL;
        }
        /* everything was ok, now calling the opener function again, to
         * continue where we stopped */
        TDB("Calling ui_open_document again\n");
	g_debug("new URI = %s\n", priv->uri_from_gateway);
        ui_open_document(priv->app_ui_data, priv->uri_from_gateway,
                         priv->password_from_gateway);
        return FALSE;
    }
    // any problem???
    return FALSE;
}


/**
	Opens a PDF document

	@param fname gnomevfs URI
	@param password Password; NULL if not provided
	@return errNone if successful; PDFDoc error code otherwise
*/
PDFViewerResult
pdf_viewer_open(const char *uri, const char *password)
{
    PDFDoc *new_doc;
    BaseStream *pdf_stream;

    GnomeVFSHandle *vfs_handle = NULL;
    GnomeVFSResult vfs_result;
    gchar *uri_scheme = NULL;
    int err;
    Object obj;
    AppData *app_data = NULL;
    PDFViewerResult result = RESULT_LOAD_OK;
    const gchar *uri_gateway = NULL;
    gboolean freepriv = FALSE;

    _pdf_abort_rendering = FALSE;
    
    g_debug( __FUNCTION__ );

    cancel_if_render();
    disable_all_ui();

    /* Make the ovr_image_orig NULL for a new document */
    if(priv->app_ui_data->ovr_image_orig != NULL)
    {
    	g_object_unref(priv->app_ui_data->ovr_image_orig);
	priv->app_ui_data->ovr_image_orig = NULL;
    }

    if (!priv->app_ui_data->copy_from_gw) {
    
    	g_debug( "%s open non-copy '%s'", __FUNCTION__, uri );
    
        priv->app_ui_data->opening_banner =
            ui_show_progress_banner(GTK_WINDOW(priv->app_ui_data->app_view),
                                    _("pdfv_ib_opening"));

        priv->need_show_info = FALSE;

        /* Remove temporary gateway PDF file, if exists */
        gint gatewaypdf_handle = open(GATEWAY_TMP_FILE, O_RDONLY);
        if (gatewaypdf_handle != -1) {
            close(gatewaypdf_handle);
            remove(GATEWAY_TMP_FILE);
        }
        priv->is_gateway = FALSE;

        /* If opening for gateway/shared, copy to tmp first */
        if (g_str_has_prefix(uri, "obex://") ||
            g_str_has_prefix(uri, "upnpav://") ||
            g_str_has_prefix(uri, "smb://")) {
            
            g_debug( "%s, Make local copy out of remote file", __FUNCTION__ );
            priv->is_gateway = TRUE;

            GnomeVFSResult vfs_result;
            vfs_result =
                gnome_vfs_open(&priv->read_handle, uri, GNOME_VFS_OPEN_READ);
            if (vfs_result != GNOME_VFS_OK)
            {
                g_warning("open not ok");
                if (priv->read_handle)
                {
                    gnome_vfs_close(priv->read_handle);
                    priv->read_handle = NULL;
                }
                pdf_viewer_empty_document();
                return RESULT_INVALID_URI;
            }
            vfs_result =
                gnome_vfs_create(&priv->write_handle, GATEWAY_TMP_FILE,
                                 GNOME_VFS_OPEN_WRITE, FALSE, 0777);
            if (vfs_result != GNOME_VFS_OK)
            {
                if (priv->read_handle)
                {
                    gnome_vfs_close(priv->read_handle);
                    priv->read_handle = NULL;
                }
                if (priv->write_handle)
                {
                    gnome_vfs_close(priv->write_handle);
                    priv->write_handle = NULL;
                }
                g_warning("create not ok");
                return RESULT_INSUFFICIENT_MEMORY;
            }
            priv->app_ui_data->copy_from_gw = TRUE;

            priv->uri_from_gateway = g_strdup(uri);
            priv->password_from_gateway = g_strdup(password);
            g_idle_add(pdf_viewer_copy_from_gw, NULL);
            return RESULT_COPY_STARTED;
        }   

    } else {
        priv->app_ui_data->copy_from_gw = FALSE;
		freepriv = TRUE;
        uri_gateway = priv->uri_from_gateway;
        password = priv->password_from_gateway;
        uri = GATEWAY_TMP_FILE;
        g_debug( "There was a copy '%s' %s' '%s'",
        	uri_gateway, password, uri );
    }
    
    g_debug( "%s, start opening", __FUNCTION__ );         

    app_data = priv->app_ui_data->app_data;

    g_return_val_if_fail(app_data != NULL, RESULT_INVALID_INTERFACE);

    /* check flash memory space */
    if( get_free_space() == 0 ) {
    	g_warning( "Not enough memory on flash." );
        pdf_viewer_empty_document();
		if (freepriv) {
	    	g_free(priv->uri_from_gateway);
	    	g_free(priv->password_from_gateway);
		}
        return RESULT_INSUFFICIENT_MEMORY;
    }

    /* setting default flags */
    priv->cancelled = FALSE;
    priv->is_mmc = FALSE;
    PDF_FLAGS_UNSET(priv->app_ui_data->flags, PDF_FLAGS_PAGE_ERROR);
    /* wait for engine init! */
    if (priv->thread != NULL) {
        g_thread_join(priv->thread);
        priv->thread = NULL;
        OSSO_LOG_DEBUG("trying to join the init thread");
    }

    /* setting application state */
    app_data->state = PDF_VIEWER_STATE_LOADING;

    /* gnomeVFS support */
    uri_scheme = gnome_vfs_get_uri_scheme(uri);
    if (!uri_scheme) {
        uri = gnome_vfs_get_uri_from_local_path(uri);
    } else {
        g_free(uri_scheme);
    }

    /* init the vfs_handle */
    vfs_result = gnome_vfs_open(&vfs_handle, uri, GNOME_VFS_OPEN_READ);

    if (vfs_result != GNOME_VFS_OK)
    {
        result = RESULT_INVALID_URI;

	/* if non-pdf file is opened directly from the File Manager*/
        if(!file_is_supported(uri))
          	result =  RESULT_UNSUPPORTED_FORMAT;
    
        /* couldn't load the document */
        pdf_viewer_empty_document();
		if (freepriv) {
	    g_free(priv->uri_from_gateway);
	    g_free(priv->password_from_gateway);
	}

        return result;
    }

    /* create stream of it */
    obj.initNull();
    pdf_stream = new OssoStream(vfs_handle, 0, gFalse, 0, &obj);

    /* the document loading has been cancelled */
    if (priv->cancelled)
    {
        if (vfs_handle)
        {
            gnome_vfs_close(vfs_handle);
            vfs_handle = NULL;
        }

        /* check the reason */
        if (priv->is_mmc)
        {
            result = RESULT_INTERRUPTED_MMC_OPEN;
        }
        else
        {
            result = RESULT_INVALID_URI;
        }

        /* couldn't load the document */
        pdf_viewer_empty_document();

	if (freepriv) {
	    g_free(priv->uri_from_gateway);
	    g_free(priv->password_from_gateway);
	}
        return result;
    }

    /* store password for state saving */
    if (priv->password)
    {
        delete priv->password;
        priv->password = NULL;
    }

    /* If the coming file is not a pdf file then
       return unsupported format */    
    if(!file_is_supported(uri)){
	  if(pdf_stream != NULL)
	      g_free(pdf_stream); //CID 6549

          return RESULT_UNSUPPORTED_FORMAT;
    }

    /* Same password used for both owner and user fields. This way, either
     * one will open an encrypted document. */
    priv->password = (password != NULL) ? new GString(password) : NULL;
    new_doc = new PDFDoc(pdf_stream, priv->password, priv->password);

    /* there was a problem with opening the document */
    if (!new_doc->isOk())
    {
        err = new_doc->getErrorCode();
        TDB("pdf_viewer_open: Open not ok");
        delete new_doc;

        /* close the file */
        if (vfs_handle)
        {
            gnome_vfs_close(vfs_handle);
            vfs_handle = NULL;
        }
        	

        switch (err)
        {
            case errOpenFile:
                TDB("pdf_viewer_open: Invalid URI\n");
                result = RESULT_INVALID_URI;
                break;
            case errDamaged:
                TDB("pdf_viewer_open: Corrupted file\n");
                result = RESULT_CORRUPTED_FILE;
                break;
            case errEncrypted:
                TDB("pdf_viewer_open: Encrypted file\n");
                result = RESULT_ENCRYPTED_FILE;
		break;
           
            case errBadCatalog:
                TDB("pdf_viewer_open: file saving not over\n");
                result = RESULT_SAVING_NOT_COMPLETED;
                
                break;
            default:
                if (priv->cancelled && priv->is_mmc)
                {
                    TDB("pdf_viewer_open: Interrupped MMC open\n");
                    result = RESULT_INTERRUPTED_MMC_OPEN;
                }
                else
                {
                    TDB("pdf_viewer_open: Unsupported format\n");
                    result = RESULT_UNSUPPORTED_FORMAT;
                }
        }	
               
        // At least in case of corrupted file we should clear uris
        if( result == RESULT_CORRUPTED_FILE ) {
	    	if (priv->vfs_URI) {
    	    	gnome_vfs_uri_unref(priv->vfs_URI);
    	    	priv->vfs_URI = NULL;
    	    }
		    if (priv->vfs_URI_gateway) {
    	    	gnome_vfs_uri_unref(priv->vfs_URI_gateway);
    	    	priv->vfs_URI_gateway = NULL;
    		}
    	}
    	
        pdf_viewer_empty_document();    	

		if (freepriv) {
	    	g_free(priv->uri_from_gateway);
	    	g_free(priv->password_from_gateway);
		}
		
        /* reset zoom level to 100% */
        priv->dpi = dpi_array[DOC_ZOOM_100];
        ui_set_current_zoom(priv->app_ui_data, pdf_viewer_get_zoom_percent());
        
	return result;
    }

    cancel_if_render();

    /* replace the old document */
    if (priv->pdf_doc)
    {
        delete priv->pdf_doc;
    }
    if (priv->vfs_handle)
    {
        gnome_vfs_close(priv->vfs_handle);
        priv->vfs_handle = NULL;
    }
    if (priv->vfs_URI)
    {
        gnome_vfs_uri_unref(priv->vfs_URI);
    }
    if (priv->vfs_URI_gateway)
    {
        gnome_vfs_uri_unref(priv->vfs_URI_gateway);
    }

    priv->vfs_handle = vfs_handle;
    priv->vfs_URI = gnome_vfs_uri_new(uri);
    if (uri_gateway != NULL)
    {
        priv->vfs_URI_gateway = gnome_vfs_uri_new(uri_gateway);
    }
    else
    {
        priv->vfs_URI_gateway = NULL;
    }

    priv->pdf_doc = new_doc;
    priv->num_pages = priv->pdf_doc->getNumPages();

    if (app_data->mode != STARTUP_MODE_STATE_DATA)
    {
        /* start from the beginning */
        TDB("At here: %u\n", page_to_load);
        if (page_to_load != 0)
        {
            priv->current_page = page_to_load;
        }
        else
        {
            priv->current_page = 1;
        }

        /* reset coordinates */
        priv->x = priv->y = 0;

        /* reset zoom level to 100% */
        priv->dpi = dpi_array[DOC_ZOOM_100];
        ui_set_current_zoom(priv->app_ui_data, pdf_viewer_get_zoom_percent());

    }
    else if (priv->num_pages < priv->current_page)
    {
        /* malicious file! */
        delete priv->pdf_doc;
        priv->pdf_doc = NULL;

        /* close the file descriptor and clean uri */
        if (priv->vfs_handle)
        {
            gnome_vfs_close(priv->vfs_handle);
            priv->vfs_handle = NULL;
        }

        gnome_vfs_uri_unref(priv->vfs_URI);
        priv->vfs_URI = NULL;

        if (priv->vfs_URI_gateway)
        {
            gnome_vfs_uri_unref(priv->vfs_URI_gateway);
            priv->vfs_URI_gateway = NULL;
        }

        app_data->state = PDF_VIEWER_STATE_EMPTY;

	if (freepriv) {
	    g_free(priv->uri_from_gateway);
	    g_free(priv->password_from_gateway);
	}
        return RESULT_INVALID_URI;
    }

    TDB(("pdf_viewer_open 25\n"));
    /* initialize the output device */
    if (priv->output_dev)
    {
        TDB(("pdf_viewer_open 25a\n"));
        priv->output_dev->startDoc(priv->pdf_doc->getXRef());
    }

    if (app_data->low_memory)
    {
        /* reset number of pages */
        priv->num_pages = PDF_PAGE_INIT;
        priv->current_page = PDF_PAGE_INIT;

        /* remove the document from memory */
        delete priv->pdf_doc;
        priv->pdf_doc = NULL;

        /* close the file descriptor */
        if (priv->vfs_handle)
        {
            gnome_vfs_close(priv->vfs_handle);
            priv->vfs_handle = NULL;
        }

        /* remove URI */
        gnome_vfs_uri_unref(priv->vfs_URI);
        priv->vfs_URI = NULL;

        if (priv->vfs_URI_gateway)
        {
            gnome_vfs_uri_unref(priv->vfs_URI_gateway);
            priv->vfs_URI_gateway = NULL;
        }

        /* change state */
        app_data->state = PDF_VIEWER_STATE_EMPTY;

        result = RESULT_INSUFFICIENT_MEMORY;

        app_data->low_memory = FALSE;
	if (freepriv) {
	    g_free(priv->uri_from_gateway);
	    g_free(priv->password_from_gateway);
	}
        return result;
    }

    gtk_range_set_value(GTK_RANGE(priv->app_ui_data->hscroll), priv->x);
    gtk_range_set_value(GTK_RANGE(priv->app_ui_data->vscroll), priv->y);

    render_page();
    result = RESULT_LOAD_OK;

    if (freepriv) {
        g_free(priv->uri_from_gateway);
        g_free(priv->password_from_gateway);
    }
    return result;
}


static void
pdf_remove_leftright_source(AppUIData * app_ui_data)
{
    if (app_ui_data->arrow_left_id)
    {
        g_source_remove(app_ui_data->arrow_left_id);
        app_ui_data->arrow_left_id = 0;
    }
    if (app_ui_data->arrow_right_id)
    {
        g_source_remove(app_ui_data->arrow_right_id);
        app_ui_data->arrow_right_id = 0;
    }
}

/**
	Navigates document pages

	@param navigate_to First, previous, next or last page
	@return void
*/
void
pdf_viewer_navigate(PDFNavigate navigate_to)
{
    ui_hide_arrows_if_exists(priv->app_ui_data, TRUE); 
    pdf_remove_leftright_source(priv->app_ui_data);
    ui_arrow_hide(priv->app_ui_data);
    g_return_if_fail(priv != NULL);

    if (!priv->pdf_doc || !priv->app_ui_data)
        return;

    TDB("pdf_viewer_navigate");
    
    /* 
     * when switching page the inner layout coordinates shall be resetted to
     * 0,0 coordinates */
    priv->x = priv->y = 0;

    PDF_FLAGS_UNSET(priv->app_ui_data->flags, PDF_FLAGS_PAGE_ERROR);

    switch (navigate_to)
    {
        case DOC_NAVI_FIRST:
            if (priv->current_page == 1)
                return;
            priv->current_page = 1;
            break;
        case DOC_NAVI_PREVIOUS:
            if (priv->current_page == 1) {
	            ui_show_banner(GTK_WIDGET(priv->app_ui_data->app_view),
					_("pdfv_ib_first_page_reached"));            
                return;
            }
            priv->current_page--;
	    /* don't save ovr_image_orig*/
	    priv->app_ui_data->ovr_image_orig = NULL; 
            break;

        case DOC_NAVI_NEXT:
            if (priv->current_page == priv->num_pages) {
	            ui_show_banner(GTK_WIDGET(priv->app_ui_data->app_view),
					_("pdfv_ib_last_page_reached"));
                return;
            }
            priv->current_page++;
	    /* don't save ovr_image_orig*/
	    priv->app_ui_data->ovr_image_orig = NULL;
            break;

        case DOC_NAVI_LAST:
            if (priv->current_page == priv->num_pages)
                return;
            priv->current_page = priv->num_pages;
            break;
    }  
    
    disable_all_ui();
    
    /* 
     * when switching page the inner layout coordinates shall be resetted to
     * 0,0 coordinates */
    gtk_image_set_from_pixmap(GTK_IMAGE(priv->app_ui_data->page_image),
                              NULL, NULL);
    priv->x = priv->y = 0;
    gtk_range_set_value(GTK_RANGE(priv->app_ui_data->hscroll), 0);
    gtk_range_set_value(GTK_RANGE(priv->app_ui_data->vscroll), 0);


    if ((navigate_to == DOC_NAVI_PREVIOUS)
        && (PDF_FLAGS_IS_SET(priv->app_ui_data->flags, PDF_FLAGS_FULLSCREEN)))
    {
        priv->app_ui_data->arrow_left_id =
            g_idle_add(ui_put_left_arrow_on_idle, priv->app_ui_data);
    }

    if ((navigate_to == DOC_NAVI_NEXT)
        && (PDF_FLAGS_IS_SET(priv->app_ui_data->flags, PDF_FLAGS_FULLSCREEN)))
    {
        priv->app_ui_data->arrow_right_id =
            g_idle_add(ui_put_right_arrow_on_idle, priv->app_ui_data);
    		
    }
    priv->need_show_info = TRUE;

    /*priv->app_ui_data->opening_banner =
        ui_show_progress_banner(GTK_WINDOW(priv->app_ui_data->app_view),
                                _("pdfv_ib_opening"));*/
    render_page();
    
}


/**
	Navigates to desired document page

	@param page page number
	@return void
*/
void
pdf_viewer_navigate_page(int page)
{

    TDB("pdf_viewer_navigate_page");

    if ((priv->pdf_doc) && (page >= 1) && (page <= (int) priv->num_pages))
    {
        disable_all_ui();
        priv->current_page = (unsigned int) page;
        priv->x = priv->y = 0;
        gtk_range_set_value(GTK_RANGE(priv->app_ui_data->hscroll), 0);
        gtk_range_set_value(GTK_RANGE(priv->app_ui_data->vscroll), 0);

        PDF_FLAGS_UNSET(priv->app_ui_data->flags, PDF_FLAGS_PAGE_ERROR);
        gtk_image_set_from_pixmap(GTK_IMAGE(priv->app_ui_data->page_image),
                                  NULL, NULL);
        /*priv->app_ui_data->opening_banner =
            ui_show_progress_banner(GTK_WINDOW(priv->app_ui_data->app_view),
                                    _("pdfv_ib_opening"));*/
        render_page();
    }
}


/**
	Zooms document page

	@param zoom_level In, out, page, width, or predefined zoom level
	@return void
*/
void
pdf_viewer_zoom(PDFZoom zoom_level)
{
    double custom_dpi = -1;
    gboolean refresh = FALSE;
    gint current_dpi = (gint) priv->dpi;
    //GtkWidget *banner = NULL;

    disable_all_ui();

    if (!priv->pdf_doc
        || (priv->app_ui_data->app_data->low_memory
	    || PDF_FLAGS_IS_SET(priv->app_ui_data->flags, PDF_FLAGS_PAGE_ERROR) && zoom_level == DOC_ZOOM_IN))
    {
	return;
    }

    PDF_FLAGS_UNSET(priv->app_ui_data->flags, PDF_FLAGS_PAGE_ERROR);

    /*banner = ui_show_progress_banner(GTK_WINDOW(priv->app_ui_data->app_view),
                                     _("pdfv_pb_zooming"));*/
   // priv->app_ui_data->zooming_banner = banner;

    /* don't save the ovr_image_orig after zoom in/out operations*/
    priv->app_ui_data->ovr_image_orig = NULL;

    switch (zoom_level)
    {

        case DOC_ZOOM_IN:
            priv->zoom_level =
                (PDFZoom) custom_ceil(dpi_array, &current_dpi, 7, sizeof(int),
                                      compare);

            if (priv->dpi != dpi_array[priv->zoom_level])
            {
                priv->dpi = dpi_array[priv->zoom_level];

                refresh = TRUE;
            }
            break;

        case DOC_ZOOM_OUT:
            priv->zoom_level =
                (PDFZoom) custom_floor(dpi_array, &current_dpi, 7,
                                       sizeof(int), compare);

            if (priv->dpi != dpi_array[priv->zoom_level])
            {
                priv->dpi = dpi_array[priv->zoom_level];

                refresh = TRUE;
            }
            break;

        case DOC_ZOOM_WIDTH:

            custom_dpi = SCREEN_DPI * (double) get_custom_zoom_level(TRUE);
            priv->zoom_level = DOC_ZOOM_WIDTH;

        case DOC_ZOOM_PAGE:
            if (custom_dpi == -1)
            {
                custom_dpi =
                    SCREEN_DPI * (double) get_custom_zoom_level(FALSE);
                priv->zoom_level = DOC_ZOOM_PAGE;
            }

            if (priv->dpi != custom_dpi)
            {
                priv->dpi = custom_dpi;
                refresh = TRUE;
            }

            break;

        case DOC_ZOOM_INVALID:
        case DOC_ZOOM_50:
        case DOC_ZOOM_100:
        case DOC_ZOOM_150:
        case DOC_ZOOM_200:
        case DOC_ZOOM_250:
        case DOC_ZOOM_300:
        case DOC_ZOOM_400:
            break;

    }

    priv->app_ui_data->dpi = priv->dpi;

    /* set the toolbar zoom level indicator */
    ui_set_current_zoom(priv->app_ui_data, pdf_viewer_get_zoom_percent());

    if (refresh)
    {
        gtk_image_set_from_pixmap(GTK_IMAGE(priv->app_ui_data->page_image),
                                  NULL, NULL);

        adjust_focus_point(current_dpi);

        resize_layout();

        gtk_range_set_value(GTK_RANGE(priv->app_ui_data->hscroll),
                            priv->scroll_x);
        gtk_range_set_value(GTK_RANGE(priv->app_ui_data->vscroll),
                            priv->scroll_y);
 
	ui_show_zoom_banner(priv->app_ui_data);

        render_page();
    }

    if (!refresh)
    {
        /*if (banner)
        {
            gtk_widget_destroy(banner);
            priv->app_ui_data->zooming_banner = NULL;
        }*/

        enable_all_ui();
    }

    
}

/**
        Toggles if images are shown or not
*/
void
pdf_viewer_toggle_images()
{
    /* redraw page if there is document loaded */
    if (priv->pdf_doc)
    {
        disable_all_ui();
        ui_close_all_banners(priv->app_ui_data->app_data);

        PDF_FLAGS_IS_SET(priv->app_ui_data->flags, PDF_FLAGS_SHOW_IMAGES) ?
            priv->app_ui_data->show_images_banner =
            ui_show_progress_banner(GTK_WINDOW(priv->app_ui_data->app_view),
                                    _("pdfv_pb_show_images")) : priv->
            app_ui_data->hide_images_banner =
            ui_show_progress_banner(GTK_WINDOW(priv->app_ui_data->app_view),
                                    _("pdfv_pb_hide_images"));
        globalParams->
            setShowImages(PDF_FLAGS_IS_SET
                          (priv->app_ui_data->flags, PDF_FLAGS_SHOW_IMAGES));
        /* display_page(); resize_layout();
         * ui_close_all_banners(priv->app_ui_data->app_data); */
        render_page();
    }
}

/**
	Returns document's uri.
	Accessor.

	@return document uri
*/
GnomeVFSURI *
pdf_viewer_get_uri()
{
    if (priv->is_gateway == TRUE)
    {
        return priv->vfs_URI_gateway;
    }
    else
    {
        return priv->vfs_URI;
    }
}


/**
	Returns number of pages in the document.
	Accessor.

	@return number of pages
*/
int
pdf_viewer_get_num_pages()
{
    if (priv == NULL)
    {
        return 0;
    }
    else
    {
        return priv->num_pages;
    }
}

/**
	Returns currently set zoom level in dpi.

	@return current zoom level
*/

int
pdf_viewer_get_current_zoom(void)
{
    return ((int) priv->dpi);
}

/**
	Returns current page of the document.
	Accessor.

	@return current page
*/
int
pdf_viewer_get_current_page()
{
    return ((priv == NULL) ? 0 : priv->current_page);
}

void
pdf_viewer_scroller_changed(PDFScroll scrl)
{
    gboolean render = FALSE;

    if (priv->dpi > FULL_RENDER_DPI)
    {
        GtkAdjustment *gadj;
#ifdef LOWMEM
        int buf_w, buf_h;

        if (!PDF_FLAGS_IS_SET(priv->app_ui_data->flags, PDF_FLAGS_FULLSCREEN))
        {
            buf_w = VIEWPORT_BUFFER_WIDTH;
            buf_h = VIEWPORT_BUFFER_HEIGHT;
        }
        else
        {
            buf_w = FULLSCREEN_BUFFER_WIDTH;
            buf_h = FULLSCREEN_BUFFER_HEIGHT;
        }
#endif
        switch (scrl)
        {
            case SCROLL_HOR:
                gadj =
                    gtk_layout_get_hadjustment(GTK_LAYOUT
                                               (priv->app_ui_data->layout));
#ifndef LOWMEM
                if ((priv->x + BUFFER_WIDTH < gadj->upper)
                    && ((priv->x + BUFFER_WIDTH / 2) <= gadj->value))
                {
                    priv->x = gadj->value - BUFFER_WIDTH / 4;
                    render = TRUE;
                }
                if ((BUFFER_WIDTH / 4 < priv->x) && (gadj->value < priv->x))
                {
                    priv->x = gadj->value - BUFFER_WIDTH / 4;
                    render = TRUE;
                }
#endif
#ifdef LOWMEM
                if (((priv->x + buf_w) < gadj->upper)
                    && ((priv->x) <= gadj->value))
                {
                    priv->x = gadj->value;
                    render = TRUE;
                }

                if ((gadj->value < priv->x))
                {
                    priv->x = gadj->value;
                    render = TRUE;
                }
#endif
                break;
            case SCROLL_VER:
                gadj =
                    gtk_layout_get_vadjustment(GTK_LAYOUT
                                               (priv->app_ui_data->layout));
#ifndef LOWMEM
                if ((priv->y + BUFFER_HEIGHT < gadj->upper)
                    && ((priv->y + BUFFER_HEIGHT / 2) <= gadj->value))
                {
                    priv->y = gadj->value - BUFFER_HEIGHT / 4;
                    render = TRUE;
                }
                if ((BUFFER_HEIGHT / 4 < priv->y) && (gadj->value < priv->y))
                {
                    priv->y = gadj->value - BUFFER_HEIGHT / 4;
                    render = TRUE;
                }
#endif
#ifdef LOWMEM
                if (((priv->y + buf_h) < gadj->upper)
                    && ((priv->y) <= gadj->value))
                {
                    priv->y = gadj->value;
                    render = TRUE;
                }
                if ((gadj->value < priv->y))
                {
                    priv->y = gadj->value;
                    render = TRUE;
                }
#endif
                break;
        }

        if (render)
        {
            render_page();
        }
    }
}

/**
   Wrapper public function for resize_layout()
*/
void
pdf_viewer_toggle_fullscreen()
{
    g_return_if_fail(priv != NULL);

    if (priv->pdf_doc == NULL)
        return;

    resize_layout();

    g_idle_add(scrollbar_change_idle, priv->app_ui_data);

}

gboolean
scrollbar_change_idle(gpointer app_ui_data)
{
    DTRY(gdk);
    GDK_THR_ENTER;
    DLOCKED(gdk);
    gtk_range_set_value(GTK_RANGE(priv->app_ui_data->hscroll),
                        priv->scroll_x);
    gtk_range_set_value(GTK_RANGE(priv->app_ui_data->vscroll),
                        priv->scroll_y);
    GDK_THR_LEAVE;
    DUNLOCKED(gdk);
    return FALSE;

}

/**
	Retrieves document information.
	Typical keys "Title", "Author", "Subject", "Keywords".

	NOTE Caller must free allocated memory using g_free
	when non-NULL value is returned.

	@param key key to document information dictionary
	@return value corresponding to the key in UTF8; 
                NULL if error or not found
*/
gchar *
pdf_viewer_get_info(char *key)
{
    Object info, obj;
    gchar *value = NULL;

    if (!priv)
        return NULL;

    if (!priv->pdf_doc || !key)
        return NULL;

    /* attempt to get document information */
    priv->pdf_doc->getDocInfo(&info);

    /* get value for specified key from the dictionary */
    if (info.isDict() && (info.getDict()->lookup(key, &obj)->isString()))
    {
        GString *str = obj.getString();
        gsize bytes_read = 0;
        gsize bytes_written = 0;
        GError *err = NULL;

        // check if BOM exists
        if (str->getLength() >= 2 &&
            str->getChar(0) == '\xfe' && str->getChar(1) == '\xff')
        {
            // UTF16 big-endian
            value = g_convert(str->getCString() + 2, str->getLength() - 2,
                    "UTF-8", "UTF-16BE", &bytes_read, &bytes_written, &err);
        }
        else
        {
            // assume it's latin1
            value = g_convert(str->getCString(), str->getLength(),
                    "UTF-8", "ISO-8859-1", &bytes_read, &bytes_written, &err);
        }
        if (err)
            g_warning("error in conversion: %s", err->message);
        g_debug("pdf info field %s = %s", key, value);
    }

    info.free();
    obj.free();

    return value;
}

/**
	Copies state information from PDFViewerPrivate to AppState.

	NOTE Caller is responsible for freeing allocated strings.

	@param app_state AppState object
	@return void
*/
void
pdf_viewer_get_state(AppState * app_state, gchar ** uri_str, gchar ** passwd)
{
    g_assert(app_state != NULL);

    *uri_str = (priv->pdf_doc != NULL) ?
        gnome_vfs_uri_to_string(pdf_viewer_get_uri(),
                                GNOME_VFS_URI_HIDE_NONE) : NULL;

    *passwd = (priv->password != NULL) ?
        g_strdup(priv->password->getCString()) : NULL;

    app_state->current_page = priv->current_page;
    app_state->dpi = (gint) priv->dpi;
    app_state->zoom_level = (gint) priv->zoom_level;

    app_state->show_images =
        PDF_FLAGS_IS_SET(priv->app_ui_data->flags, PDF_FLAGS_SHOW_IMAGES);

}

/**
 Save opened pdf document

 @param dst - destination URI
 @return GnomeVFSResult or -1 if no pdf_doc open, else GnomeVFSResult
*/
PDFViewerResult
pdf_viewer_save(const char *dst)
{
    GnomeVFSURI *src_uri = NULL;
    GnomeVFSURI *dst_uri = NULL;
    GnomeVFSURI *dst_parent = NULL;
    gchar *uri_scheme = NULL;
    gboolean dst_exists = TRUE;
    // guint timer_id = 0;
    GTimer *save_time = NULL;
    GTimer **tm = &save_time;
    GnomeVFSResult vfs_res;
    PDFViewerResult res;

    /* check if there's a document to save */
    if (!priv->pdf_doc || !dst)
    {
        return RESULT_INVALID_INTERFACE;
    }

    priv->cancelled = FALSE;
    priv->is_mmc = FALSE;

    priv->save_dst = g_strdup(dst);
    
	/* g_print( "dst: '%s', save_dst: '%s'\n", dst, priv->save_dst ); */

    /* change state to saving */
    priv->app_ui_data->app_data->state = PDF_VIEWER_STATE_SAVING;

    /* get the proper uri for destination */
    uri_scheme = gnome_vfs_get_uri_scheme(dst);
    if (!uri_scheme)
        dst = gnome_vfs_get_uri_from_local_path(dst);
    else
        g_free(uri_scheme);

    /* create gnomeVFSURIs */
    src_uri = priv->vfs_URI;
    dst_uri = gnome_vfs_uri_new(dst);

    /* check for errors */
    if (!gnome_vfs_uri_exists(src_uri))
    {
        res = RESULT_INVALID_URI;
        goto done;
    }

    dst_parent = gnome_vfs_uri_get_parent(dst_uri);
    dst_exists = gnome_vfs_uri_exists(dst_parent);
    gnome_vfs_uri_unref(dst_parent);

    if (!dst_exists)
    {
        res = RESULT_SAVE_FAILED;
        goto done;
    }

    /* check if destination exists */
    if (gnome_vfs_uri_exists(dst_uri))
    {
        GtkWidget *replace_dialog = NULL;
        gchar *text = NULL;
        gchar *formatted_name = NULL;
        gint ret = 0;

        formatted_name = get_basename_for_display(dst);

        text = g_strdup_printf("%s\n%s",
                                D_("docm_nc_replace_file"), formatted_name);

        if (text != NULL)
        {
                replace_dialog =
                        GTK_WIDGET(hildon_note_new_confirmation
                                  (GTK_WINDOW(priv->app_ui_data->app_view),
                                  text));
		priv->app_ui_data->replace_dialog = replace_dialog;

                ret = gtk_dialog_run(GTK_DIALOG(replace_dialog));
                gtk_widget_destroy(GTK_WIDGET(replace_dialog));
		priv->app_ui_data->replace_dialog = NULL;		
        }

        if (formatted_name != NULL)
                g_free(formatted_name);

        if (text != NULL)
                g_free(text);
        
        if (ret != GTK_RESPONSE_OK)
        {
            /* cancel saving to this destination */
            res = RESULT_SAVE_CANCELLED;
            goto done;
        }
        /* 
         * else continue and rewrite w/o check
         * GNOME_VFS_XFER_OVERWRITE_MODE_REPLACE */
    }

    /* 
     * adding timer for saving, if saving takes more than SAVE_TIMEOUT, then
     * a 'Saving...' banner appears. SAVE_TIMEOUT defined in constant.h */
    save_time = g_timer_new();

    if (!save_time)
    {
        res = RESULT_SAVE_FAILED;
        goto done;
    }

    ui_enable_document_open(priv->app_ui_data, FALSE);
    /* copy the src to dst, no overwrite. */
    pb_banner = NULL;
    vfs_res = gnome_vfs_xfer_uri(src_uri, dst_uri,
                                 GNOME_VFS_XFER_DEFAULT,
                                 GNOME_VFS_XFER_ERROR_MODE_ABORT,
                                 GNOME_VFS_XFER_OVERWRITE_MODE_REPLACE,
                                 on_progress_info, tm);

    if (save_time)
        g_timer_destroy(save_time);
    if (pb_banner)
        gtk_widget_destroy(pb_banner);

    ui_enable_document_open(priv->app_ui_data, TRUE);

    /* 
     * transform gnomevfserror to pdfviewererror (if dst location unmounted
     * during save gnome-vfs-result is _still_ GNOME_VFS_OK for some reason) */
    if (priv->cancelled)
    {

        /* 
         * the volume got unmounted, but we have specified infobanner just
         * for MMC */

        if (priv->is_mmc)
            res = RESULT_INTERRUPTED_MMC_OPEN;
        else
            res = RESULT_SAVE_FAILED;

    }
    else
    {

        switch (vfs_res)
        {
            case GNOME_VFS_OK:
            {
                GnomeVFSFileInfo info = { 0 };
                info.atime = info.mtime = info.ctime = time(NULL);
                gnome_vfs_set_file_info_uri(dst_uri, &info,
                                            GNOME_VFS_SET_FILE_INFO_TIME);
                res = RESULT_SAVE_OK;
            }
                break;

            case GNOME_VFS_ERROR_NO_SPACE:
            case GNOME_VFS_ERROR_NO_MEMORY:
                res = RESULT_NO_SPACE_ON_DEVICE;
                break;

            case GNOME_VFS_ERROR_NOT_PERMITTED:
            case GNOME_VFS_ERROR_READ_ONLY_FILE_SYSTEM:
            case GNOME_VFS_ERROR_ACCESS_DENIED:
            case GNOME_VFS_ERROR_READ_ONLY:
                res = RESULT_SAVE_NOT_ALLOWED;
                break;

            default:
                res = RESULT_SAVE_FAILED;
        }
    }

  done:
    gnome_vfs_uri_unref(dst_uri);

    g_free(priv->save_dst);
    priv->save_dst = NULL;

    priv->app_ui_data->app_data->state = PDF_VIEWER_STATE_LOADED;

    return res;
}

void
pdf_viewer_unload(void)
{
    if (!priv)
        return;

    ui_close_all_banners(priv->app_ui_data->app_data);
    if (priv->app_ui_data->details_dialog)
        gtk_dialog_response(GTK_DIALOG(priv->app_ui_data->details_dialog),
                            GTK_RESPONSE_OK);
    if( IS_RENDERING( priv ) ) {
        ui_show_banner(GTK_WIDGET(priv->app_ui_data->app_view),
                       _("pdfv_error_memorycard"));
    }
    pdf_viewer_empty_document();
}

gboolean
pdf_viewer_is_rendering() {
	return IS_RENDERING( priv );
}

void
pdf_viewer_cancel_if_render() {
    OSSO_LOG_DEBUG(__FUNCTION__);

    /* pdf_viewer_cancel_if_render is called from gtk signal handler, which
     * locks gdk. We need to unlock it until rendering thread has finished */
    gdk_threads_leave(); 
    DUNLOCKED(gdk);

    /* cancel rendering thread */
    cancel_if_render();

    DTRY(gdk);
    gdk_threads_enter();
    DLOCKED(gdk);
}

/* EOF */
