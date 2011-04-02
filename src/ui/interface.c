/**
    @file interface.c
	
    General user interface functions

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


#ifdef HAVE_CONFIG_H
#include <aconf.h>
#endif

    

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libgnomevfs/gnome-vfs-result.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libosso.h>
#include <hildon/hildon-defines.h>
#include <hildon/hildon-program.h>
#include <hildon/hildon-window.h>
#include <hildon/hildon-number-editor.h>
#include <hildon/hildon-file-chooser-dialog.h>
#include <hildon/hildon-file-details-dialog.h>
#include <hildon/hildon-file-system-model.h>
#include <hildon/hildon-get-password-dialog.h>
#include <hildon/hildon-caption.h>
#include <hildon/hildon-banner.h>
#include <hildon/hildon-helper.h>
#include <hildon/hildon-note.h>

/* For Zoom buttons*/
#include <gdk/gdkx.h>
#include <X11/Xatom.h> 
/**/

#include <comapp_details.h>
#include <comapp_opensave.h>

#include "ErrorCodes.h"

#include "i18n.h"
#include "utility.h"
#include "interface.h"
#include "ui.h"
#include "callbacks.h"
#include "constant.h"
#include "pdfviewer.h"
#include "debug.h"
#include "thread_debug.h"

#define HELP_ICON_SIZE HILDON_ICON_SIZE_TOOLBAR

#define CURRENT_PAGE_BUTTON_MIN_WIDTH 90
//#define CURRENT_PAGE_BUTTON_MIN_WIDTH 80
#define CURRENT_ZOOM_WIDGET_CHARS      6
#define POP_MENU_ITEM_WIDTH 70
#define NAVIGATE_IMAGE_HEIGHT 48
#define NAVIGATE_IMAGE_WIDTH 48
#define ARROWS_HIDE_TIMEOUT 4000

#define debug(x) x
/*******************************************************************************
 **** Private data
 **/



/*******************************************************************************
 **** Prototypes for private functions
 **/

/* Application constructors */
static void add_custom_actions(GtkActionGroup * actions,
                               AppUIData * app_ui_data);
static GtkToolbar *build_toolbar(GtkActionGroup * actions,
                                 AppUIData * app_ui_data);
static void build_application_area(AppUIData * app_ui_data);

/* Switch to page widget maintainers */
static void ui_set_current_page(AppUIData * app_ui_data, gint current_page_number, gint total_pages);

/* Menu/ToolBar Item logic controllers */
void ui_enable_document_controls(AppUIData * app_ui_data, gboolean enable);

/* Creates and/or gives reference on HildonFileSystemModel */
static HildonFileSystemModel *get_file_system_model(GtkWidget * ref_widget);

static gboolean ui_arrow_hide_on_idle(gpointer data);

void ui_create_menu(AppUIData * app_ui_data);


/* bug: 87157 STARTS */
static
void encrypt_file_banner(gpointer *data)
{
	gchar *str = (gchar *)data;
        ui_show_banner(NULL, str);
	g_idle_remove_by_data(data);	
}
/* bug: 87157 ENDS */


/*******************************************************************************
 **** Public Functions
 ***/


/* Close all possible banners */
void
ui_close_all_banners(AppData * app_data )
{
    if (app_data->app_ui_data->show_images_banner)
    {
        gtk_widget_destroy(app_data->app_ui_data->show_images_banner);
        app_data->app_ui_data->show_images_banner = NULL;
    }
    if (app_data->app_ui_data->hide_images_banner)
    {
        gtk_widget_destroy(app_data->app_ui_data->hide_images_banner);
        app_data->app_ui_data->hide_images_banner = NULL;
    }
    /*if (app_data->app_ui_data->zooming_banner)
    {
        gtk_widget_destroy(app_data->app_ui_data->zooming_banner);
        app_data->app_ui_data->zooming_banner = NULL;
    }*/
    if (app_data->app_ui_data->opening_banner)
    {
        gtk_widget_destroy(app_data->app_ui_data->opening_banner);
        app_data->app_ui_data->opening_banner = NULL;
    }
    return;
}


/**
	Creates application user interface.
	Creates corresponding actions, UI manager, menus and the toolbar.
	
	@param app HildonApp
	@return void
*/
void
ui_create_main_window(AppData * app_data)
{
    HildonProgram *app;
    HildonWindow *app_view;
    //GtkSettings *settings;
    GtkUIManager *ui_manager;
    GtkActionGroup *actions;
    GError *error = NULL;
    gpointer user_data = NULL;
    AppUIData *app_ui_data;
	GtkAccelGroup * accel_group;
	GtkWidget *popup_widget;

    app_ui_data = app_data->app_ui_data;
    g_assert(app_ui_data != NULL);

    /* enabling some flags by default */
    PDF_FLAGS_SET(app_ui_data->flags, PDF_FLAGS_SELECT_KEY_ALLOWED);

    /* Create app_view and application */
    app_view = HILDON_WINDOW(hildon_window_new());
    g_set_application_name(_("pdfv_ap_pdf_viewer_name"));
    g_return_if_fail(app_view != NULL);
    app_ui_data->app_view = app_view;

	/* Portrait mode */
	hildon_gtk_window_set_portrait_flags (GTK_WINDOW (app_view),
										HILDON_PORTRAIT_MODE_SUPPORT);

    app = HILDON_PROGRAM(hildon_program_get_instance());
    if (app == NULL)
    {
        gtk_object_destroy(GTK_OBJECT(app_ui_data->app_view));
        return;
    }
    hildon_program_add_window(app, app_view);
    app_ui_data->app = app;

    /* Set user_data to make it accessible to action callback(s) */
    user_data = app_ui_data;

    /* Create new action group */
    actions = gtk_action_group_new("Actions");
    g_assert(actions);
    /* Translation domain need to be set _before_ adding actions */
    gtk_action_group_set_translation_domain(actions, GETTEXT_PACKAGE);
    gtk_action_group_add_actions(actions, action_entries, n_action_entries,
                                 user_data);
    gtk_action_group_add_toggle_actions(actions, toggle_action_entries,
                                        n_toggle_action_entries, user_data);

    /* Add any custom actions */
    add_custom_actions(actions, app_ui_data);

    /* Create new UI manager */
    ui_manager = gtk_ui_manager_new();
    g_assert(ui_manager);
    app_ui_data->ui_manager = ui_manager;
    gtk_ui_manager_insert_action_group(ui_manager, actions, 0);

    if (!gtk_ui_manager_add_ui_from_string(ui_manager, ui_info, -1, &error))
    {
        OSSO_LOG_ERR("Building ui_manager failed: %s", error->message);
        g_error_free(error);
        g_assert(FALSE);
    }

    popup_widget = gtk_ui_manager_get_widget (ui_manager,"/Popup/pdfv_me_menu_page_previous");
     gtk_widget_set_size_request (GTK_WIDGET(popup_widget), -1, POP_MENU_ITEM_WIDTH);    
   
      popup_widget = gtk_ui_manager_get_widget (ui_manager,"/Popup/pdfv_me_menu_page_next");
     gtk_widget_set_size_request (GTK_WIDGET(popup_widget), -1, POP_MENU_ITEM_WIDTH);    
   
   popup_widget = gtk_ui_manager_get_widget (ui_manager,"/Popup/pdfv_me_menu_screen_zoom_in");
   gtk_widget_set_size_request (GTK_WIDGET(popup_widget), -1, POP_MENU_ITEM_WIDTH);    
   
   popup_widget = gtk_ui_manager_get_widget (ui_manager,"/Popup/pdfv_me_menu_screen_zoom_out");
   gtk_widget_set_size_request (GTK_WIDGET(popup_widget), -1, POP_MENU_ITEM_WIDTH);    

    popup_widget = gtk_ui_manager_get_widget (ui_manager,"/Popup/pdfv_me_menu_document_details");
   gtk_widget_set_size_request (GTK_WIDGET(popup_widget), -1, POP_MENU_ITEM_WIDTH);    	

    /* Transfer menu items to app_view menu */
    ui_create_menu(app_ui_data);
    g_assert(app_ui_data->menu);

	/* Create accgroup */
	accel_group = gtk_ui_manager_get_accel_group( ui_manager );
	gtk_window_add_accel_group( GTK_WINDOW( app_ui_data->app_view ),
		accel_group );

    /* Bind toolbar with app_view */
    app_ui_data->toolbar = build_toolbar(actions, app_ui_data);
    g_assert(app_ui_data->toolbar);
    hildon_program_set_common_toolbar(app_ui_data->app,
                                      GTK_TOOLBAR(app_ui_data->toolbar));
    gtk_widget_show_all(GTK_WIDGET(app_ui_data->toolbar));

    /* Build application area */
    build_application_area(app_ui_data);
		
    /* Finally some signal-handlers */

    g_signal_connect(G_OBJECT(app_view), "key_release_event",
                     G_CALLBACK(key_release), (gpointer) app_ui_data);

    g_signal_connect(G_OBJECT(app_view), "key_press_event",
                     G_CALLBACK(key_press), (gpointer) app_ui_data);

    /* catch top/untop events for state saving */
    g_signal_connect(G_OBJECT(app), "notify::is-topmost",
                     G_CALLBACK(top_changed), app_ui_data);

    /* catch delete event */
    g_signal_connect(G_OBJECT(app_view), "delete_event",
                     G_CALLBACK(on_delete_event), app_ui_data);

    g_signal_connect(G_OBJECT(app_view), "window-state-event",
                     G_CALLBACK(window_state_changed), app_ui_data); 

	/* Portrait mode support */
	g_signal_connect(G_OBJECT(app_ui_data->app_view), "configure-event",
					G_CALLBACK(configure_event_cb), app_ui_data);
     
    /* Show all widgets */
    gtk_widget_show_all(GTK_WIDGET(app_view));
    
    /* taking care of Zoom buttons : starts */
    GdkDisplay *display;
    Atom atom;
    unsigned long val = 1;
    
    display = gdk_drawable_get_display(GTK_WIDGET(app_ui_data->app_view)->window);
    
    atom = gdk_x11_get_xatom_by_name_for_display(display, "_HILDON_ZOOM_KEY_ATOM");
    
    XChangeProperty (GDK_DISPLAY_XDISPLAY(display), GDK_WINDOW_XID(GTK_WIDGET(app_ui_data->app_view)->window), atom, XA_INTEGER, 32, PropModeReplace,
			    (unsigned char *) &val, 1);
    
    /* taking care of Zoom buttons : ends */
    
    /* set the UI */
    ui_update(app_ui_data);
}

/**
	Creates application view menu.

	@param app HildonApp
	@return void
*/
void
ui_create_menu(AppUIData * app_ui_data) 
{
    GtkWidget *button;
    HildonSizeType buttonsize = HILDON_SIZE_FINGER_HEIGHT | HILDON_SIZE_AUTO_WIDTH;
    GtkAccelGroup *accel = gtk_accel_group_new ();

    app_ui_data->menu = HILDON_APP_MENU(hildon_app_menu_new());

    /* Open */
    button = hildon_gtk_button_new (buttonsize);
    gtk_button_set_label (GTK_BUTTON (button), _("pdfv_me_menu_document_open"));
    g_signal_connect_after (button, "clicked", G_CALLBACK (on_document_open), app_ui_data);
    hildon_app_menu_append (app_ui_data->menu, GTK_BUTTON (button));
    app_ui_data->menu_open = button; /* store reference */
    gtk_widget_add_accelerator (button, "activate", accel, GDK_o, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    /* Save */
    button = hildon_gtk_button_new (buttonsize);
    gtk_button_set_label (GTK_BUTTON (button), _("pdfv_me_menu_document_save"));
    g_signal_connect_after (button, "clicked", G_CALLBACK (on_document_save), app_ui_data);
    hildon_app_menu_append (app_ui_data->menu, GTK_BUTTON (button));
    app_ui_data->menu_save = button; /* store reference */
    gtk_widget_add_accelerator (button, "activate", accel, GDK_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    /* Details */
    button = hildon_gtk_button_new (buttonsize);
    gtk_button_set_label (GTK_BUTTON (button), _("pdfv_me_menu_document_details"));
    g_signal_connect_after (button, "clicked", G_CALLBACK (on_document_details), app_ui_data);
    hildon_app_menu_append (app_ui_data->menu, GTK_BUTTON (button));
    app_ui_data->menu_details = button; /* store reference */

    gtk_widget_show_all(GTK_WIDGET(app_ui_data->menu));

    hildon_window_set_app_menu (HILDON_WINDOW (app_ui_data->app_view), 
				app_ui_data->menu);
    gtk_window_add_accel_group (GTK_WINDOW (app_ui_data->app_view), accel);
    g_object_unref (accel);

    return;
}

/**
 * Runs the open or save dialog.
 *
 * @param app_ui_data Application UI Data
 * @param action whether to run open or save
 */
void
ui_run_file_chooser(AppUIData * app_ui_data, GtkFileChooserAction action)
{
    /* construct uri */

    gchar *uri_str = NULL;

    if (app_ui_data->last_uri)
    {
        uri_str = g_strdup(app_ui_data->last_uri);
    }
    else
    {
        uri_str = get_factory_default_folder();
    }

	/* Replace unnormal paths with default path (move this to libcomapp?) */
	if( (strncmp( uri_str, "file:///var/tmp/", 16 ) == 0 )/* for files opened from web*/
             || ((strncmp( uri_str, "/var/tmp/", 9 ) == 0 ))) {/*for files opened as an email attachment*/	
                gchar * filename = strrchr( uri_str, '/' );
		filename = gnome_vfs_unescape_string(filename, NULL); // handles the spaces between the words
		gchar * new_path = g_build_filename( get_factory_default_folder(), filename, NULL );
	    gchar * new_uri_str = gnome_vfs_make_uri_from_input( new_path );
	    g_free( new_path );
		g_free( uri_str );
		uri_str = new_uri_str;
	}

    /* prepare libcomapp opensave structure */

    ComappOpenSave *cos = g_new0(ComappOpenSave, 1);

    cos->parent = GTK_WIDGET(app_ui_data->app_view);
    cos->action = action;
    cos->model = get_file_system_model(GTK_WIDGET(app_ui_data->app_view));
    cos->open_window_title = _("pdfv_ti_open_document");
    cos->save_window_title = _("ckdg_ti_save_object_doc");
    cos->empty_text = NULL;
    cos->osso = app_ui_data->app_data->comapp_system->osso;
    cos->mime_types = NULL;
    cos->current_uri = uri_str;

    cos = comapp_opensave_new(cos);

    /* disable multiple selection and set filters */

    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(cos->fc_dialog),
                                         FALSE);
    GtkFileFilter *filter = get_filter_for_supported_formats();

    if (filter != NULL)
    {
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(cos->fc_dialog), filter);
        gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(cos->fc_dialog), filter);
    }

    app_ui_data->opensave_dialog = cos->fc_dialog;
    comapp_fc_result res = comapp_opensave_run(cos);

    gchar *filename = NULL;

    if (res == COMAPP_FILE_CHOOSER_SELECTED)
    {
        filename = cos->result_uri;

        /* ensure that user did not just click OK */
        if (filename != NULL)
        {
            if (action == GTK_FILE_CHOOSER_ACTION_OPEN)
            {

                /* start opening the file */
                ui_open_document(app_ui_data, filename, NULL);

            }
            else if (action == GTK_FILE_CHOOSER_ACTION_SAVE)
            {
                PDFViewerResult result = pdf_viewer_save(filename);

                if (app_ui_data->last_uri)
                    g_free (app_ui_data->last_uri);
                app_ui_data->last_uri = g_strdup(filename);

                if (result == RESULT_SAVE_OK)
                {
                    ui_open_document(app_ui_data, filename, NULL);
                    //ui_update(app_ui_data);
                }

                /* show the result */
                ui_show_result(app_ui_data, result);
            }
            g_free(filename);
        }
    }
    else if (res == COMAPP_FILE_CHOOSER_ERROR)
    {
	 ui_show_banner(GTK_WIDGET(app_ui_data->app_view),  _("pdfv_error_savefail"));
    }	    

    g_free(uri_str);
    g_object_unref(cos->model);
    app_ui_data->opensave_dialog = NULL;

}

static GtkWidget *global_password_dialog = NULL;

/**
 	Open document with given file name.
	Presents user with a password dialog if necessary.
	
	@param app_ui_data Application UI data structure
	@param filename filename
	@param password password (optional, can be NULL)
	@return pdf_viewer_open return code
*/
static PDFViewerResult
_ui_open_document(AppUIData * app_ui_data,
                  const gchar * filename, const gchar * password)
{
    GtkWidget *password_dialog = NULL;
    gchar *uri;
    gint response;
    int retries;
    PDFViewerResult result;
    gboolean gpd;
    
    g_assert( filename != NULL );
    g_debug( "%s: '%s'", __FUNCTION__, filename );

    /* Validates the file name to URI format (free before return!) */
    uri = gnome_vfs_make_uri_from_input(filename);
    g_debug( "%s: uri: '%s'", __FUNCTION__, uri );
    
    /* check if the currently opened document is the same as the upcoming one */
    if (pdf_viewer_get_uri() != NULL)
    {
        gint strcmp_res;
        gchar *current_uri = gnome_vfs_uri_to_string(pdf_viewer_get_uri(),
                                                     GNOME_VFS_URI_HIDE_NONE);

        /* the file is already opened */
        strcmp_res = strcmp(current_uri, uri);

        /* destroy the prev file's URI */
        g_free(current_uri);

        if (strcmp_res == 0)
        {
            g_free(uri);
            return RESULT_LOAD_OK;
        }
    }

    /* open document with the xpdf engine */
    retries = 0;
    do
    {
        if (retries == 0)
        {
            /* show empty document & bring it up, then start rendering
             * to get reasonable response time */
            pdf_viewer_empty_document();

            /* open document initially with supplied password, if any */
            result = pdf_viewer_open(uri, password);
            if (result == RESULT_COPY_STARTED)
            {
                g_free(uri);
                return RESULT_COPY_STARTED;
            }
        }
        else
        {

            /* get password using the password dialog */
            password_dialog =
                hildon_get_password_dialog_new(GTK_WINDOW
                                               (app_ui_data->app_view),
                                               FALSE);
            gtk_window_set_modal(GTK_WINDOW(password_dialog), TRUE);

            global_password_dialog = password_dialog;
	    app_ui_data->password_dialog = password_dialog;
            GDK_THR_ENTER;
            response = gtk_dialog_run(GTK_DIALOG(password_dialog));
            GDK_THR_LEAVE;

			/* Did user cancel the dialog */
            if ((gpd = (global_password_dialog == password_dialog)))
                global_password_dialog = NULL;
		app_ui_data->password_dialog  = NULL;
		
            if (response != GTK_RESPONSE_OK)
            {
                gtk_widget_destroy(password_dialog);
		app_ui_data->password_dialog  = NULL;
                /* user cancelled the password dialog */
                if (gpd)
                    ui_enable_document_open(app_ui_data, TRUE);

                g_free(uri);
                return RESULT_ENCRYPTED_FILE;
            } else {
                const gchar * pass = hildon_get_password_dialog_get_password(
                	HILDON_GET_PASSWORD_DIALOG(password_dialog));

				/* attempt to open pdf document with supplied password */
				result = pdf_viewer_open(uri, pass);
	            gtk_widget_destroy(password_dialog);
	            
				if (result == RESULT_COPY_STARTED)
				{
					g_free(uri);
					return RESULT_COPY_STARTED;
				}
			}

        }

        /* check if document was opened successfully */
        if (result == RESULT_LOAD_OK)
        {
            /* update the UI */
            ui_update(app_ui_data);

            break;
        }
        else
        {
            ui_close_all_banners(app_ui_data->app_data);

            if (result != RESULT_ENCRYPTED_FILE)
            {
                /* we only need to show the info banner/note */
                break;
            }
        }

        /* errEncrypted */
        if (retries == 0)
        {
            /* first round, display an infobanner that document is encrypted */
	    /* bug: 87157 STARTS */
	    g_idle_add((GSourceFunc) encrypt_file_banner, _("pdfv_ib_encrypted_file"));
	    /* bug: 87157 ENDS */
        }
        else
        {
            /* incorrect password */
	    /* bug: 87157 STARTS */
	    g_idle_add((GSourceFunc) encrypt_file_banner, _("pdfv_ib_incorrect_password"));
	    /* bug: 87157 ENDS  */
        }

    } while (retries++ < MAX_PASSWORD_RETRIES);

    if (result != RESULT_LOAD_OK) {    
        pdf_viewer_empty_document();
        ui_enable_document_open(app_ui_data, TRUE);
    	ui_update(app_ui_data);        
    }
    
    g_free(uri);
    return result;
}

typedef struct {
    AppUIData *app_ui_data;
    gchar *filename;
    gchar *password;
} tOpenDoc;

static gint idle_id = 0;

static gboolean
idle_open(gpointer data)
{
    tOpenDoc *p = (tOpenDoc *) data;
    PDFViewerResult result;
    idle_id = 0;
    
    g_assert( p != NULL );
    
    if (global_password_dialog)
    {
    	GtkWidget * dialog = global_password_dialog;
        global_password_dialog = NULL;
        GDK_THR_ENTER;
        gtk_dialog_response(GTK_DIALOG(dialog),
                            GTK_RESPONSE_CANCEL);
        gtk_widget_hide(dialog);
        GDK_THR_LEAVE;
    }
    result = _ui_open_document(p->app_ui_data, p->filename, p->password);
    if (result != RESULT_COPY_STARTED)
    {
        GDK_THR_ENTER;
        ui_show_result(p->app_ui_data, result);
        GDK_THR_LEAVE;
    }
    
    return FALSE;
}

void
idle_delete(gpointer data)
{
    tOpenDoc *p = (tOpenDoc *) data;
    g_assert( p != NULL );
    if (!p->app_ui_data->copy_from_gw)
    {
        TDB("Freeing filename, password\n");
        g_free(p->filename);
        g_free(p->password);
        g_free(p);
    }
}

void
ui_open_document(AppUIData * app_ui_data,
                 const gchar * filename, const gchar * password)
{
	
    if (app_ui_data->app_data->state != PDF_VIEWER_STATE_SAVING)
    {
        tOpenDoc *p = NULL;

        if (app_ui_data->last_uri)
            g_free (app_ui_data->last_uri);
        app_ui_data->last_uri = g_strdup(filename);

        if (!app_ui_data->copy_from_gw)
        {
            p = g_new(tOpenDoc, 1);
            p->app_ui_data = app_ui_data;
            p->filename = g_strdup(filename);
            p->password = g_strdup(password);
            app_ui_data->open_document_structure = p;
        }
        else
        {
            p = app_ui_data->open_document_structure;
            /* This can't be set null here or passwd protected files over bt will
               not work
            app_ui_data->open_document_structure = NULL; */
        }
        if (idle_id)
            g_source_remove(idle_id);
        idle_id =
            g_idle_add_full(G_PRIORITY_DEFAULT_IDLE, idle_open, p, idle_delete);
    }
}


/**
	Runs switch to page dialog.
	
	@param app_ui_data Application UI data structure
	@return void
*/
void
ui_run_switch_page_dialog(AppUIData * app_ui_data)
{
    GtkWidget *dialog, *vbox, *label, *number_editor;
    GtkWidget *caption_control = NULL;
    gchar *text = NULL;
    gint result;
    gint idle_id;
    gboolean valid_number = FALSE;
    gint min = PDF_PAGE_INIT + 1;
    gint max = pdf_viewer_get_num_pages();
    gint value = pdf_viewer_get_current_page();
    const gchar *PDFstr = SUPPRESS_FORMAT_WARNING
	                            (_("pdfv_fi_switch_page_amount"));

    /* Create new stock dialog with ok and cancel buttons */
    app_ui_data->switch_page_dialog = dialog =
        gtk_dialog_new_with_buttons(_("pdfv_ti_switch_page_title"),
                                    GTK_WINDOW(app_ui_data->app_view),
                                    GTK_DIALOG_MODAL |
                                    GTK_DIALOG_DESTROY_WITH_PARENT |
                                    GTK_DIALOG_NO_SEPARATOR,
                                    _("pdfv_bd_switch_page_ok"),
                                    GTK_RESPONSE_OK,
                                     NULL);
    g_assert(dialog);

    /* Disable resize, already modal */
    gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
    vbox = GTK_DIALOG(dialog)->vbox;

    /* "Current page" label, 1st argument current page, 2nd argument last
     * page */
    text =
        g_strdup_printf(PDFstr/* SUPPRESS_FORMAT_WARNING
                        (_("pdfv_fi_switch_page_amount")) */, value, max);
    label = gtk_label_new(text);
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
    g_free(text);

    gtk_box_pack_start(GTK_BOX(vbox), label, TRUE, TRUE, 5);

    /* Number editor */
    number_editor = hildon_number_editor_new(min, max);
    g_assert(number_editor);

    /* Print custom message if user gives out-of-range value */
    g_object_set_data(G_OBJECT(number_editor), "idle-id", (gpointer) 0);
    g_signal_connect(G_OBJECT(number_editor), "range-error",
                     G_CALLBACK(on_number_editor_error), app_ui_data);


    hildon_number_editor_set_value(HILDON_NUMBER_EDITOR(number_editor),
                                   value);
    caption_control = hildon_caption_new(NULL,
                                         _("pdfv_fi_switch_page_description"),
                                         number_editor,
                                         NULL, HILDON_CAPTION_MANDATORY);

    gtk_box_pack_start(GTK_BOX(vbox), caption_control, TRUE, TRUE, 5);
    
    /* Show all widgets and run dialog */
    gtk_widget_show_all(dialog);
    do
    {
        result = gtk_dialog_run(GTK_DIALOG(dialog));

        idle_id =
            (gint) g_object_get_data(G_OBJECT(number_editor), "idle-id");
        if (result == GTK_RESPONSE_OK)
        {
            valid_number = idle_id == 0;
            if (valid_number)
            {
                result =
                    hildon_number_editor_get_value(HILDON_NUMBER_EDITOR
                                                   (number_editor));
                /* Dialog was accepted, navigate to desired page */
                /* valid_number = (result != pdf_viewer_get_current_page()); */
            }
            else
            {
                g_source_remove(idle_id);
                g_object_set_data(G_OBJECT(number_editor), "idle-id",
                                  (gpointer) 0);
                gtk_widget_grab_focus(GTK_WIDGET(number_editor));
            }
        }
        else
        {
            /* the number is not true, but button was cancel so we quit */
            if (idle_id)
                g_source_remove(idle_id);
            break;
        }
    }
    while (valid_number == FALSE);
    /* else cancelled */
    gtk_widget_destroy(dialog);
    app_ui_data->switch_page_dialog = NULL;

    if (valid_number)
    {
        pdf_viewer_navigate_page(result);
        ui_set_current_page(app_ui_data, pdf_viewer_get_current_page(), pdf_viewer_get_num_pages());
    }

}


/**
 * Show file details dialog.
 *
 * @param app_ui_data Application UI Data structure
 */
void
ui_run_details_dialog(AppUIData * app_ui_data)
{
    GSList *details = NULL;
    ComappDetailItem *cdi = NULL;

    /* Construct the detail items */
    cdi = g_new0(ComappDetailItem, 1);
    cdi->name = g_strdup_printf("%s", _("pdfv_fi_document_details_title"));
    cdi->content = pdf_viewer_get_info(DOCUMENT_DETAILS_TITLE);
    if( cdi->content == NULL )
    	cdi->content = g_strdup( " " );
    details = g_slist_append(details, cdi);

    cdi = g_new0(ComappDetailItem, 1);
    cdi->name = g_strdup_printf("%s", _("pdfv_fi_document_details_author"));
    cdi->content = pdf_viewer_get_info(DOCUMENT_DETAILS_AUTHOR);
    if( cdi->content == NULL )
    	cdi->content = g_strdup( " " );
    details = g_slist_append(details, cdi);

    cdi = g_new0(ComappDetailItem, 1);
    cdi->name =
        g_strdup_printf("%s", _("pdfv_fi_document_details_subject"));
    cdi->content = pdf_viewer_get_info(DOCUMENT_DETAILS_SUBJECT);
    if( cdi->content == NULL )
    	cdi->content = g_strdup( " " );
    details = g_slist_append(details, cdi);

    cdi = g_new0(ComappDetailItem, 1);
    cdi->name =
        g_strdup_printf("%s", _("pdfv_fi_document_details_keywords"));
    cdi->content = pdf_viewer_get_info(DOCUMENT_DETAILS_KEYWORDS);
    if( cdi->content == NULL )
    	cdi->content = g_strdup( " " );
    details = g_slist_append(details, cdi);

    /* fill libcomapp's details dialog structure and let it create the dialog 
     */

    gchar *uri = gnome_vfs_uri_to_string(pdf_viewer_get_uri(),
                                         GNOME_VFS_URI_HIDE_NONE);
    ComappDetailsData cdd = {
        .details_data = details,
        .window_title = _("pdfv_me_menu_document_details"),
        .label_title = NULL,
        .osso = app_ui_data->app_data->comapp_system->osso,
        .fs_model = get_file_system_model(GTK_WIDGET(app_ui_data->app_view)),
        .parent = GTK_WIDGET(app_ui_data->app_view),
        .uri = uri
    };

    app_ui_data->details_dialog = comapp_details_new(&cdd);

    /* show the dialog */

    gtk_dialog_run(GTK_DIALOG(app_ui_data->details_dialog));
    gtk_widget_destroy(app_ui_data->details_dialog);
    app_ui_data->details_dialog = NULL;


    /* free detail items */

    g_object_unref(cdd.fs_model);
    g_free(uri);

    GSList *tmp = NULL;
    for (tmp = details; tmp; tmp = tmp->next)
    {
        g_free(((ComappDetailItem *) tmp->data)->name);
        g_free(((ComappDetailItem *) tmp->data)->content);
    }
    g_slist_free(details);

}

/**
	Toggles fullscreen mode and updates user interface accordingly,
	including main menu and context sensitive menu toggles.


	@param app_ui_data Application UI data structure
	@param fullscreen TRUE if switch to fullscreen, FALSE if normal
	@return void
*/
void
ui_toggle_fullscreen(AppUIData * app_ui_data, gboolean fullscreen)
{
    GtkAction *action;
    gboolean active;

    g_return_if_fail(app_ui_data != NULL);
    g_return_if_fail(app_ui_data->ui_manager != NULL);
#if 0
    GtkWidget *widget;
 
    /* attempt to retrieve the full screen menu item widget */
    widget = gtk_ui_manager_get_widget(app_ui_data->ui_manager,
                                       ACTION_PATH_FULL_SCREEN);

    g_return_if_fail(widget != NULL);
 
    /* exit if full screen menu item widget is insensitive */
    if (GTK_WIDGET_SENSITIVE(widget) == FALSE)
    {
        return;
    }

#endif
    /* attempt to retrieve the full screen menu action */
    action =
        gtk_ui_manager_get_action(app_ui_data->ui_manager,
                                  ACTION_PATH_FULL_SCREEN);

    g_assert(action != NULL);

    /* retrieve active state of the menu toggle only enable or disable
     * fullscreen mode if necessary */
    active = gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action));
    if (active != fullscreen)
    {
        /* Toggle fullscreen state. The actual fullscreen mode setting is
         * performed via a callback in ui_set_fullscreen(). */
        gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action), !active);
    }
}

void
ui_calculate_arrows_cords(AppUIData * app_ui_data, Arrow_Cords * cords)
{
    guint window_width = 0, window_height = 0;
    guint scrollbar_pos =
        (guint) gtk_range_get_value(GTK_RANGE(app_ui_data->vscroll));

    window_height = GTK_WIDGET(app_ui_data->layout)->allocation.height;
    window_width = GTK_WIDGET(app_ui_data->layout)->allocation.width;
    cords->y_pos =
        scrollbar_pos + (window_height / 2 - NAVIGATE_IMAGE_HEIGHT / 2);

    scrollbar_pos =
        (guint) gtk_range_get_value(GTK_RANGE(app_ui_data->hscroll));
    cords->x_pos_left = scrollbar_pos;
    cords->x_pos_right = scrollbar_pos + window_width - NAVIGATE_IMAGE_WIDTH;
}

void
ui_calculate_overlay_cords(AppUIData * app_ui_data, Arrow_Cords * cords)
{
    guint window_width = 0, window_height = 0;
    guint scrollbar_pos =
    (guint) gtk_range_get_value(GTK_RANGE(app_ui_data->vscroll));
		
    window_height = GTK_WIDGET(app_ui_data->layout)->allocation.height;
    window_width = GTK_WIDGET(app_ui_data->layout)->allocation.width;
    cords->y_pos =
	        scrollbar_pos + (window_height - NAVIGATE_IMAGE_HEIGHT );

    scrollbar_pos =
	        (guint) gtk_range_get_value(GTK_RANGE(app_ui_data->hscroll));
   
   /* Fullscreen overlay button at the lower middle part of the screen*/
   //cords->x_pos_left = scrollbar_pos + window_width/2 - NAVIGATE_IMAGE_WIDTH/2;
   
   /* Fullscreen overlay button at the lower right part of the screen */
   cords->x_pos_left = scrollbar_pos + window_width - NAVIGATE_IMAGE_WIDTH;

   cords->x_pos_right = scrollbar_pos + window_width/2 + NAVIGATE_IMAGE_WIDTH/2;
}

void
ui_put_arrow(AppUIData * app_ui_data, GdkPixbuf * arrow_image, gint dest_x,
             gint dest_y, GdkPixbuf ** temporary_copy)
{
    GdkPixbuf *arrowdraw = NULL;
    arrowdraw = gdk_pixbuf_get_from_drawable( NULL,
    	GTK_WIDGET(app_ui_data->page_image)->window,
		NULL, dest_x, dest_y, 0, 0,
		NAVIGATE_IMAGE_WIDTH, NAVIGATE_IMAGE_HEIGHT);
		
	if( arrowdraw == NULL ) {
		g_warning( "NULL arrow" );
	}
		
    if( temporary_copy != NULL )
        *temporary_copy = gdk_pixbuf_copy(arrowdraw);
    
    gdk_pixbuf_composite(arrow_image, arrowdraw, 0, // dest_x
                         0,     // dest_y
                         NAVIGATE_IMAGE_WIDTH,  // dest_width
                         NAVIGATE_IMAGE_HEIGHT, // dest_height
                         0,     // offset_x
                         0,     // offset_y
                         1.0,   // scale_x
                         1.0,   // scale_y
                         GDK_INTERP_NEAREST, 255);

    gdk_draw_pixbuf(GTK_WIDGET(app_ui_data->page_image)->window,
                    GTK_WIDGET(app_ui_data->page_image)->style->black_gc,
                    arrowdraw,
                    0, 0,
                    dest_x, dest_y,
                    NAVIGATE_IMAGE_WIDTH, NAVIGATE_IMAGE_HEIGHT,
                    GDK_RGB_DITHER_NORMAL, 0, 0);
                    
    if(arrowdraw != NULL) // CID 2226
    {
    	g_object_unref( arrowdraw );
    }
                    
}

static void
ui_put_image(AppUIData * app_ui_data, GdkPixbuf * image, gint dest_x,
             gint dest_y)
{

    gdk_draw_pixbuf(GTK_WIDGET(app_ui_data->page_image)->window,
                    GTK_WIDGET(app_ui_data->page_image)->style->black_gc,
                    image, 0, 0, dest_x, dest_y,
                    NAVIGATE_IMAGE_WIDTH, NAVIGATE_IMAGE_HEIGHT,
                    GDK_RGB_DITHER_NORMAL, 0, 0);

}

void ui_hide_overlay_image(AppUIData * app_ui_data, GdkPixbuf * image)
{
    Arrow_Cords     cords;
    if(!app_ui_data->fullscreen || !app_ui_data->ovrshown)
    {
          return;
    }

#if 0    
    cords = app_ui_data->last_ovr_cord;
    /* When we enter to the Fullscreen mode from normal mode
     * the coordinates of ovr_image == ovr_image_orig*/
    if (cords.x_pos_left == 0)
#endif	    
    ui_calculate_overlay_cords(app_ui_data, &cords);        

    gdk_draw_pixbuf(GTK_WIDGET(app_ui_data->page_image)->window,
    GTK_WIDGET(app_ui_data->page_image)->style->black_gc, 
		  image, 0, 0, cords.x_pos_left, cords.y_pos, 
                  NAVIGATE_IMAGE_WIDTH, NAVIGATE_IMAGE_HEIGHT,
                  GDK_RGB_DITHER_NORMAL, 0, 0);
    app_ui_data->ovrshown = FALSE;
	    
    return;
}
				
void ui_show_ovr_image(AppUIData *app_ui_data)
{
    Arrow_Cords     cords;
    if(!app_ui_data->fullscreen || app_ui_data->ovrshown)  
    {
          return;
    }

    ui_calculate_overlay_cords(app_ui_data, &app_ui_data->last_ovr_cord);   
    cords = app_ui_data->last_ovr_cord;
    ui_put_arrow(app_ui_data, app_ui_data->ovr_image, cords.x_pos_left,
                 cords.y_pos, &app_ui_data->ovr_image_orig);
                 app_ui_data->ovrshown = TRUE;

   return;
}
				

gboolean
ui_put_left_arrow_on_idle(gpointer data)
{

    DTRY(gdk);
    gdk_threads_enter();
    DLOCKED(gdk);
    AppUIData *app_ui_data = (AppUIData *) data;
    Arrow_Cords arrow_cords;
    ui_calculate_arrows_cords(app_ui_data, &app_ui_data->last_arrow_cord);
    arrow_cords = app_ui_data->last_arrow_cord;
    ui_put_arrow(app_ui_data, app_ui_data->left_image, arrow_cords.x_pos_left,
                 arrow_cords.y_pos, &app_ui_data->left_image_orig);

    gdk_threads_leave();
    DUNLOCKED(gdk);
    app_ui_data->arrow_left_id = 0;
    
    app_ui_data->arrow_hide_id =
        g_timeout_add(ARROWS_HIDE_TIMEOUT, ui_arrow_hide_on_idle, data);     
    return FALSE;
}

gboolean
ui_put_right_arrow_on_idle(gpointer data)
{

    DTRY(gdk);
    gdk_threads_enter();
    DLOCKED(gdk);
    AppUIData *app_ui_data = (AppUIData *) data;
    Arrow_Cords arrow_cords;
    ui_calculate_arrows_cords(app_ui_data, &app_ui_data->last_arrow_cord);
    arrow_cords = app_ui_data->last_arrow_cord;
    ui_put_arrow(app_ui_data, app_ui_data->right_image,
                 arrow_cords.x_pos_right, arrow_cords.y_pos,
                 &app_ui_data->right_image_orig);
    gdk_threads_leave();
    DUNLOCKED(gdk);
    app_ui_data->arrow_right_id = 0;
    
    app_ui_data->arrow_hide_id =
        g_timeout_add(ARROWS_HIDE_TIMEOUT, ui_arrow_hide_on_idle, data);    
    return FALSE;
}


void
ui_hide_arrows_if_exists(AppUIData * app_ui_data, gboolean destroy_put_source)
{

	g_debug( __FUNCTION__ );

    if (PDF_FLAGS_IS_SET(app_ui_data->flags, PDF_FLAGS_FULLSCREEN))
    {
        if (destroy_put_source && app_ui_data->arrow_put_id)
        {
            g_source_remove(app_ui_data->arrow_put_id);
            app_ui_data->arrow_put_id = 0;
        }
        if (app_ui_data->arrow_hide_id)
        {
            g_source_remove(app_ui_data->arrow_hide_id);
            app_ui_data->arrow_hide_id = 0;
            ui_arrow_hide(app_ui_data);
        }
    }
}

void
ui_arrow_hide(gpointer data)
{
    AppUIData *app_ui_data = (AppUIData *) data;
    Arrow_Cords arrow_cords;
    arrow_cords = app_ui_data->last_arrow_cord;

    if (app_ui_data->left_image_orig) {
        ui_put_image(app_ui_data, app_ui_data->left_image_orig,
                     arrow_cords.x_pos_left, arrow_cords.y_pos);
        g_object_unref(app_ui_data->left_image_orig);
        app_ui_data->left_image_orig = NULL;
    }
    
    if (app_ui_data->right_image_orig) { 
        ui_put_image(app_ui_data, app_ui_data->right_image_orig,
                     arrow_cords.x_pos_right, arrow_cords.y_pos);
        g_object_unref(app_ui_data->right_image_orig);
        app_ui_data->right_image_orig = NULL;
    }

    memset(&app_ui_data->last_arrow_cord, 0, sizeof(Arrow_Cords));
}

static gboolean
ui_arrow_hide_on_idle(gpointer data)
{

    DTRY(gdk);	
    gdk_threads_enter();
    DLOCKED(gdk);
    AppUIData *app_ui_data = (AppUIData *) data;
    ui_arrow_hide(data);
    gdk_threads_leave();
    DUNLOCKED(gdk);
    app_ui_data->arrow_hide_id = 0;
    return FALSE;
}

static gboolean
ui_put_arrows_idle(gpointer data)
{

    DTRY(gdk);
    gdk_threads_enter();
    DLOCKED(gdk);
    AppUIData *app_ui_data = (AppUIData *) data;
    Arrow_Cords arrow_cords;
    
    ui_calculate_overlay_cords(app_ui_data, &app_ui_data->last_arrow_cord);
    arrow_cords = app_ui_data->last_arrow_cord;
#if 0
    /* dont draw the overlay button when zooming is taking place*/
    if (app_ui_data->zooming_banner == NULL) 
    {
    	/* don't draw the overlay button when opening/rendering of a page is happening*/
	if (!(pdf_viewer_is_rendering())) 
    	{
	    ui_put_arrow(app_ui_data, app_ui_data->ovr_image, arrow_cords.x_pos_left,
	                      arrow_cords.y_pos, &app_ui_data->ovr_image_orig);
    	    app_ui_data->ovrshown = TRUE; 
    	}
    }
#endif
    ui_calculate_arrows_cords(app_ui_data, &app_ui_data->last_arrow_cord);
    arrow_cords = app_ui_data->last_arrow_cord;


    ui_put_arrow(app_ui_data, app_ui_data->left_image, arrow_cords.x_pos_left,
                 arrow_cords.y_pos, &app_ui_data->left_image_orig);
    ui_put_arrow(app_ui_data, app_ui_data->right_image,
                 arrow_cords.x_pos_right, arrow_cords.y_pos,
                 &app_ui_data->right_image_orig);

    gdk_threads_leave();
    DUNLOCKED(gdk);
    
    app_ui_data->arrow_put_id = 0;
    
    app_ui_data->arrow_hide_id =
        g_timeout_add(ARROWS_HIDE_TIMEOUT, ui_arrow_hide_on_idle, data);
       
    return FALSE;
}

/**
	Sets fullscreen mode on or off.

	NOTE this function must _only_ be called from the on_screen_full_screen()
	callback. All other users, please use ui_toggle_fullscreen() instead.

	@param app_ui_data Application UI data structure
	@param fullscreen TRUE if switch to fullscreen, FALSE if normal
	@return void
*/
void
ui_set_fullscreen(AppUIData * app_ui_data, gboolean fullscreen)
{
    ui_hide_arrows_if_exists(app_ui_data, TRUE);
    if (PDF_FLAGS_IS_SET(app_ui_data->flags, PDF_FLAGS_FULLSCREEN) !=
        fullscreen)
    {
        PDF_FLAGS_TOGGLE(app_ui_data->flags, PDF_FLAGS_FULLSCREEN);
        pdf_viewer_move_after_fullscreen_togle();
        /* hide toolbars when going fullscreen */
        if (fullscreen)
        {
            gtk_widget_hide_all(GTK_WIDGET(app_ui_data->toolbar));
            gtk_window_fullscreen(GTK_WINDOW(app_ui_data->app_view));

        }
        else
        {
            gtk_window_unfullscreen(GTK_WINDOW(app_ui_data->app_view));
            gtk_widget_show_all(GTK_WIDGET(app_ui_data->toolbar));
        }

	app_ui_data->fullscreen = fullscreen; //bangalore
        /* 
         * enable or disable scrollbars as necessary this is done via the
         * pfdviewer component */

        pdf_viewer_toggle_fullscreen();
        if (fullscreen)
        {
            app_ui_data->arrow_put_id =
                g_idle_add(ui_put_arrows_idle, app_ui_data);
        }
    }
}


/**
	Updates current page label number.
	Gets current page number from pdfviewer.

	@param app_ui_data AppUIData structure
	@return void
*/
void
ui_update_current_page(AppUIData * app_ui_data)
{
    ui_set_current_page(app_ui_data, pdf_viewer_get_current_page(), pdf_viewer_get_num_pages());
}

/**
   Update every menu/toolbar item on the UI
   according to the application's state

   @param AppUIData
*/
void
ui_update(AppUIData * app_ui_data)
{
    PDFViewerState cur_state;

    gchar *file = NULL;
    gchar *title = NULL;

    /* check input */
    g_return_if_fail(app_ui_data != NULL);
    g_return_if_fail(app_ui_data->app_data != NULL);

    /* get application state */
    cur_state = app_ui_data->app_data->state;

    if (cur_state == PDF_VIEWER_STATE_EMPTY)
    {
        /* set the title empty */

        /* set page numbers */
        ui_set_current_page(app_ui_data, PDF_PAGE_INIT, PDF_PAGE_INIT);

        /* disable page controls */
        ui_enable_page_controls(app_ui_data, DIM_ALL, FALSE);

        /* disable scrolling/panning */
        gtk_layout_set_size(GTK_LAYOUT(app_ui_data->layout), 1, 1);
        ui_enable_scrollbars(app_ui_data, FALSE, FALSE);
        
        /* Make sure that we don't show any ghost name */
        /*NB#93860*/
        #ifdef __FREMANTLE_SW__
        gtk_window_set_title( GTK_WINDOW(app_ui_data->app_view), _("pdfv_ap_pdf_viewer_name"));
        #else	
        gtk_window_set_title( GTK_WINDOW(app_ui_data->app_view), "" );
        #endif
    }
    else
    {
    
        /* document was successfully opened, update user interface */
        ui_set_current_page(app_ui_data, pdf_viewer_get_current_page(), pdf_viewer_get_num_pages());

        /* get the newly opened document's location */
        file = gnome_vfs_uri_to_string(pdf_viewer_get_uri(),
                                       GNOME_VFS_URI_HIDE_NONE);

        /* get the base name (w/o path) */
        title = get_basename_for_display(file);
        g_free(file);

        /* lower every char in title */
        file = g_ascii_strdown(title, -1);

        /* check if it has .pdf extension */
        if (g_str_has_suffix(file, PDFV_FILE_EXTENSION_DOT))
        {
            title[strlen(title) - strlen(PDFV_FILE_EXTENSION_DOT)] = '\0';
        }
        g_free(file);
   
    #ifdef __FREMANTLE_SW__	
	gchar *result = g_strconcat(_("pdfv_ap_pdf_viewer_name"),
	                            " - ",
	                            title,
	                            NULL);

        gtk_window_set_title( GTK_WINDOW(app_ui_data->app_view), result);
	g_free(result);
    #else 	
        /* update application title */
        gtk_window_set_title( GTK_WINDOW(app_ui_data->app_view), title );
    #endif
        g_free(title);

    }
}

/*******************************************************************************
 **** Private functions
 **/


/**
	Registers any custom actions.
	These actions are not created automatically using
	gtk_action_group_add_actions().
	
	@param actions Initialized action group
	@param app_ui_data AppUIData structure
	@return void
*/
static void
add_custom_actions(GtkActionGroup * actions, AppUIData * app_ui_data)
{
    GtkAction *action;

    g_assert(actions);

    /* Create new custom action and add it to action group */
    action = gtk_action_new(ACTION_PAGE_SWITCH_TO,
                            _(ACTION_PAGE_SWITCH_TO), NULL, NULL);

    g_signal_connect(G_OBJECT(action),
                     "activate", G_CALLBACK(on_page_switch_to), app_ui_data);

    /* g_object_set(action, "sensitive", FALSE, NULL); */
    gtk_action_group_add_action(actions, GTK_ACTION(action));
}


/**
	Builds main toolbar.
	Uses ui manager created toolbar as a basis and adds
	any custom widgets to it.

	AppUIData ui_manager needs to be initialized.
	
	@param ui_manager UI manager
	@param actions Initialized action group
	@return initialized toolbar if successfull; NULL otherwise
*/
static GtkToolbar *
build_toolbar(GtkActionGroup * actions, AppUIData * app_ui_data)
{
    GtkToolbar *toolbar = NULL;
    GtkToolItem *tool_item;
    GtkWidget *label, *image, *box;
    GtkWidget *button;
    GtkAction *action;
    gint i, nitems;

    /* Get toolbar widget from ui manager */
    toolbar =
        GTK_TOOLBAR(gtk_ui_manager_get_widget
                    (app_ui_data->ui_manager, "/ToolBar"));

    // GTK_WIDGET_UNSET_FLAGS (, GTK_CAN_FOCUS);
    if (!toolbar)
        return NULL;

    /* Set toolbar properties */
    gtk_toolbar_set_orientation(toolbar, GTK_ORIENTATION_HORIZONTAL);
    gtk_toolbar_set_style(toolbar, GTK_TOOLBAR_ICONS);

    /* Set GtkToolButtons unfocusable */
    nitems = gtk_toolbar_get_n_items(toolbar);
    for (i = 0; i < nitems; i++)
    {
        tool_item = gtk_toolbar_get_nth_item(toolbar, i);
        gtk_tool_item_set_homogeneous(tool_item, FALSE);
        if (!GTK_IS_SEPARATOR_TOOL_ITEM(tool_item))
        {
            /* gtk_tool_item_set_expand(tool_item, TRUE); */
            GTK_WIDGET_UNSET_FLAGS(gtk_bin_get_child(GTK_BIN(tool_item)),
                                   GTK_CAN_FOCUS);

            gtk_tool_item_set_expand(tool_item, FALSE);
            //gtk_tool_item_set_expand(tool_item, TRUE);
            /* use icon theme TODO there has to be a more elegant way to bind 
             * GtkActionEntries with GtkIconTheme */
            if (GTK_IS_TOOL_BUTTON(tool_item)
                && (gtk_tool_button_get_stock_id(GTK_TOOL_BUTTON(tool_item))
                    != NULL))
            {
                /* for each tool button: 1. get custom stock id as defined in 
                 * ui.h GtkActionEntry 2. create a new image widget using an
                 * icon theme aware function and stock id 3. set (replace)
                 * image widget with new image */

                image =
                    gtk_image_new_from_icon_name
                    (gtk_tool_button_get_stock_id(GTK_TOOL_BUTTON(tool_item)),
                     HILDON_ICON_SIZE_TOOLBAR);
                if (image != NULL)
                {
                    gtk_widget_show(image);
                    gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON
                                                    (tool_item), image);
                }

            }
        }
        else
        {
            gtk_tool_item_set_expand(tool_item, FALSE);
            gtk_tool_item_set_homogeneous(tool_item, FALSE);
            
            gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM
                                                 (tool_item), FALSE);

        }
    }

    /* Create "current page" widget */
    tool_item = gtk_tool_item_new();
    gtk_tool_item_set_expand(tool_item, TRUE);
    gtk_tool_item_set_homogeneous(tool_item, FALSE);
    button = gtk_button_new();
    GTK_WIDGET_UNSET_FLAGS(button, GTK_CAN_FOCUS);

    /* set minimum horizontal size for button to avoid other toolbar buttons
     * moving around with longer page numbers */
    gtk_widget_set_size_request(button, CURRENT_PAGE_BUTTON_MIN_WIDTH, -1);
    gtk_button_set_alignment(GTK_BUTTON(button), (gfloat) 1.0, (gfloat) 0.5);
    gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
    label = gtk_label_new(NULL);
    gtk_container_add(GTK_CONTAINER(button), label);
    gtk_container_add(GTK_CONTAINER(tool_item), button);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),
                       tool_item, TOOLBAR_POS_CURRENT_PAGE_WIDGET);
    //gtk_label_set_width_chars(GTK_LABEL(label), 4);
    gtk_label_set_width_chars(GTK_LABEL(label), 10);

    /* with newer GTK, just use action's short-label */
    // app_ui_data->current_page = label;

    app_ui_data->current_page_item = GTK_WIDGET(tool_item);
    app_ui_data->current_page_button = GTK_WIDGET(button);
    /*hildon_helper_set_insensitive_message( app_ui_data->current_page_button,
    	_("pdfv_ib_menu_not_available") );*/

    /* Get previously created custom action and bind it to custom widget */

    action = gtk_action_group_get_action(actions, ACTION_PAGE_SWITCH_TO);
    app_ui_data->current_page = action;
    g_assert(action);
    gtk_action_connect_proxy(action, button);

    /* Create "total pages" widget */
#if 0    
    tool_item = gtk_tool_item_new();
    label = gtk_label_new(NULL);
    gtk_container_add(GTK_CONTAINER(tool_item), label);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), tool_item,
                       TOOLBAR_POS_CURRENT_PAGE_WIDGET + 1);
    app_ui_data->total_pages = label;
    gtk_label_set_width_chars(GTK_LABEL(label), 6);
#endif 

    /* Set initial page numbers */
    ui_set_current_page(app_ui_data, PDF_PAGE_INIT, PDF_PAGE_INIT);

    /* create Current Zoom widget */
    tool_item = gtk_tool_item_new();
	g_object_ref(tool_item);
    gtk_tool_item_set_homogeneous(tool_item, FALSE);
    gtk_tool_item_set_expand(tool_item, FALSE);
    box = gtk_hbox_new(FALSE, 0);
    label = gtk_label_new(NULL);
    /* width of the widget */
    gtk_box_pack_end(GTK_BOX(box), label, TRUE, FALSE, 0);
    gtk_label_set_single_line_mode(GTK_LABEL(label), TRUE);
    gtk_widget_set_size_request(GTK_WIDGET(box), 80, -1);

    gtk_container_add(GTK_CONTAINER(tool_item), box);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),
                       tool_item, TOOLBAR_POS_CURRENT_ZOOM_WIDGET);

    app_ui_data->current_zoom_value = label;
    app_ui_data->current_zoom_item = GTK_WIDGET(tool_item);

    /* create images for dynamic toolbar */
    app_ui_data->img_images_on =
        gtk_image_new_from_icon_name("general_image",
                                     HILDON_ICON_SIZE_TOOLBAR);
    g_object_ref(app_ui_data->img_images_on);

    app_ui_data->img_images_off =
        gtk_image_new_from_icon_name
        ("pdf_viewer_no_images",
         HILDON_ICON_SIZE_TOOLBAR);

    g_object_ref(app_ui_data->img_images_off);

    app_ui_data->fullscreen_toolbar_button =  GTK_WIDGET(gtk_toolbar_get_nth_item(toolbar, TOOLBAR_POS_FULLSCREEN));
     
    return toolbar;
}

/**
	Builds application area.
	Application area consists of ScrolledWindow, Layout and Image.
	Ensures that application area is built only once.
	
	@param app HildonApp
	@return void
*/
static void
build_application_area(AppUIData * app_ui_data)
{
    GtkWidget *scrolled_window, *layout, *page_image;
    GtkWidget *popup = NULL;
    GtkAdjustment *vadj, *hadj;
    GtkIconTheme *icon_theme = NULL;
    GError *error = NULL;

    g_assert(app_ui_data);

    /* Application area: ScrolledWindow - Layout - Image */

    scrolled_window = gtk_table_new(2, 2, FALSE);
    g_assert(scrolled_window);

    gtk_container_add(GTK_CONTAINER(app_ui_data->app_view), scrolled_window);

    /* It is crucial to set size for the layout, otherwise scrolledwindow
     * cannot calculate scrollbar adjustments */
    layout = gtk_layout_new(NULL, NULL);
    g_assert(layout);
    gtk_container_set_border_width(GTK_CONTAINER(layout), 0);

    /* moved gtk_widget_add_events and g_signal_connects immediately after
     * creating GtkLayout, otherwise button_release_event mystically
     * disappears */

    /* Add events for panning */
    gtk_widget_add_events(layout,
                          (GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK
                           | GDK_BUTTON_MOTION_MASK |
                           GDK_POINTER_MOTION_MASK |
                           GDK_POINTER_MOTION_HINT_MASK));

    g_signal_connect(G_OBJECT(layout), "event",
                     G_CALLBACK(on_screen_event), app_ui_data);

    g_signal_connect(G_OBJECT(layout), "button_press_event",
                     G_CALLBACK(on_screen_press), app_ui_data);

    g_signal_connect(G_OBJECT(layout), "button_release_event",
                     G_CALLBACK(on_screen_release), app_ui_data);

    g_signal_connect(G_OBJECT(layout), "motion_notify_event",
                     G_CALLBACK(on_screen_motion), app_ui_data);

    gtk_settings_set_long_property(gtk_settings_get_default(),
		    		 "gtk-menu-images",
				 0,"");

    /* setup popup menu TODO: check if this is OK */
    popup = gtk_ui_manager_get_widget(app_ui_data->ui_manager, "/Popup");

    if (popup)
    {
        /* gtk_widget_tap_and_hold_setup(layout, popup, NULL,
         * GTK_TAP_AND_HOLD_PASS_PRESS); */
        g_signal_connect_after((gpointer) layout, "tap-and-hold",
                               G_CALLBACK(on_screen_tap_and_hold_event),
                               popup);
        gtk_widget_tap_and_hold_setup(layout, NULL, NULL,
                                      GTK_TAP_AND_HOLD_PASS_PRESS);
        gtk_menu_attach_to_widget(GTK_MENU(popup), layout, NULL);
    }

    /* attaching the layout to the GtkTable */
    gtk_table_attach(GTK_TABLE(scrolled_window), layout, 0, 1, 0, 1,  //0, 2, 0, 2
                     GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 0, 0);

    /* creating scrollbars, attaching to the GtkTable */
    app_ui_data->vscroll = gtk_vscrollbar_new(NULL);
    gtk_table_attach(GTK_TABLE(scrolled_window), app_ui_data->vscroll, 1,  // 2, 3, 0, 2
                     2, 0, 1, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK,
                     0, 0);
    app_ui_data->hscroll = gtk_hscrollbar_new(NULL);
    gtk_table_attach(GTK_TABLE(scrolled_window), app_ui_data->hscroll, 0,  // 0, 2, 2, 3
                     1, 1, 2, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK,
                     0, 0);

    /* associating the scrollbars with layout's adjustments */
    vadj = gtk_layout_get_vadjustment(GTK_LAYOUT(layout));
    gtk_range_set_adjustment(GTK_RANGE(app_ui_data->vscroll), vadj);
    hadj = gtk_layout_get_hadjustment(GTK_LAYOUT(layout));
    gtk_range_set_adjustment(GTK_RANGE(app_ui_data->hscroll), hadj);

    g_object_set(vadj, "step-increment", HSCROLLBAR_INCREMENT, NULL);
    g_object_set(hadj, "step-increment", VSCROLLBAR_INCREMENT, NULL);

    /* adding signal events for scrollbars' value change - for partial
     * rendering */
    g_signal_connect(G_OBJECT(hadj), "value-changed",
                     G_CALLBACK(on_screen_scroll),
                     GUINT_TO_POINTER(SCROLL_HOR));
    g_signal_connect(G_OBJECT(vadj), "value-changed",
                     G_CALLBACK(on_screen_scroll),
                     GUINT_TO_POINTER(SCROLL_VER));

   /* Add event handling for showing/hiding the overlay image */
   g_signal_connect(G_OBJECT(app_ui_data->hscroll), "button_press_event",
                    G_CALLBACK(on_scroll_press), app_ui_data);

   g_signal_connect(G_OBJECT(app_ui_data->vscroll), "button_press_event",
                    G_CALLBACK(on_scroll_press), app_ui_data);
  
   g_signal_connect(G_OBJECT(app_ui_data->hscroll), "button_release_event",
                    G_CALLBACK(on_scroll_release), app_ui_data);

   g_signal_connect(G_OBJECT(app_ui_data->vscroll), "button_release_event",
                    G_CALLBACK(on_scroll_release), app_ui_data);

    /* TODO change to appropriate widget */
    page_image = gtk_image_new_from_pixbuf(NULL);

    icon_theme = gtk_icon_theme_get_default();
    app_ui_data->right_image =
        gtk_icon_theme_load_icon(icon_theme, "qgn_forward_fsm", 
				 HILDON_ICON_PIXEL_SIZE_FINGER, 0, &error);

    app_ui_data->left_image =
        gtk_icon_theme_load_icon(icon_theme, "qgn_back_fsm", 
				 HILDON_ICON_PIXEL_SIZE_FINGER, 0, &error);

     app_ui_data->ovr_image = 
            gtk_icon_theme_load_icon(icon_theme, "general_fullsize", 
				 HILDON_ICON_PIXEL_SIZE_FINGER, 0, &error);

    g_assert(app_ui_data->right_image && app_ui_data->left_image && app_ui_data->ovr_image);

    gtk_layout_put(GTK_LAYOUT(layout), page_image, 0, 0);

    gtk_layout_set_size(GTK_LAYOUT(layout),
                        page_image->allocation.width + 1,
                        page_image->allocation.height + 1);

    /* Store ui references to AppData */
    app_ui_data->scrolled_window = scrolled_window;
    app_ui_data->layout = layout;
    app_ui_data->page_image = page_image;
}

/**
	Sets current page label number

	@param app_ui_data AppUIData structure
	@param value current page
	@return void
*/
static void
ui_set_current_page(AppUIData * app_ui_data, gint current_page_number, gint total_pages)
{
    gchar *text = g_strdup_printf("%d/%d", current_page_number, total_pages);

    g_object_set(app_ui_data->current_page, "short-label", text, NULL);

    g_free(text);
}

/**
	Enables or disabled document controls.
	Currently, sets certain menu items sensitive or insensitive.

	@param app_ui_data AppUIData structure
	@return void
*/
void
ui_enable_document_controls(AppUIData * app_ui_data, gboolean enable)
{
    GtkWidget *widget;
    static gboolean prev_enable = -1;

    // TDB("ui_enable_document_controls: %d\n", enable);

    if (enable == prev_enable)
    {
        /* primitive state caching */
        return;
    }

    /* use gtk_widget_set_sensitive instead of g_object_set Also, page
     * controls (next, prev, zoom etc.) are now handled in
     * ui_enable_page_controls() */

    /* Menu items */
    if (enable) {
        gtk_widget_show(app_ui_data->menu_save);
        gtk_widget_show(app_ui_data->menu_details);
    } else {
        gtk_widget_hide(app_ui_data->menu_save);
        gtk_widget_hide(app_ui_data->menu_details);
    }

    /* 'Document Details' */
    widget = gtk_ui_manager_get_widget(app_ui_data->ui_manager,
                                       "/Popup/pdfv_me_menu_document_details");
    if (widget != NULL)
        gtk_widget_set_sensitive(widget, enable);

    /* 'Fullscreen' */
    widget = gtk_ui_manager_get_widget(app_ui_data->ui_manager,
                                       "/ToolBar/"
				       "pdfv_me_menu_screen_full_screen");
    if (widget != NULL)
        gtk_widget_set_sensitive(widget, enable);
	    
}

void
ui_enable_document_open(AppUIData * app_ui_data, gboolean enable)
{
    if (enable) {
        gtk_widget_show(app_ui_data->menu_open);
    } else {
        gtk_widget_hide(app_ui_data->menu_open);
    }
}


/**
   Gets file system model 
  
   @param ref_widget - any widget on the screen
   @return HildonFileSystemModel object
*/
static HildonFileSystemModel *
get_file_system_model(GtkWidget * ref_widget)
{
    return
        HILDON_FILE_SYSTEM_MODEL(g_object_new
                                 (HILDON_TYPE_FILE_SYSTEM_MODEL, "ref_widget",
                                  ref_widget, NULL));
}


/**
   Enables page controls
   
   @param app_ui_data - application UI data
   @param dim - what to undim/dim
   @param enable - undim or dim
*/
void
ui_enable_page_controls(AppUIData * app_ui_data, PDFDim dim, gboolean enable)
{
    GtkWidget *widget;

    g_return_if_fail(app_ui_data != NULL);

    TDB("ui_enable_page_controls: %d, %d\n", dim, enable);

    switch (dim)
    {

        case DIM_ZOOM_IN:
            widget = gtk_ui_manager_get_widget(app_ui_data->ui_manager,
                                               "/ToolBar/pdfv_me_menu_screen_zoom_in");
            if (widget != NULL)
                gtk_widget_set_sensitive(widget, enable);

            widget = gtk_ui_manager_get_widget(app_ui_data->ui_manager,
                                               "/Popup/pdfv_me_menu_screen_zoom_in");
            if (widget != NULL)
                gtk_widget_set_sensitive(widget, enable);
            break;

        case DIM_ZOOM_OUT:
            widget = gtk_ui_manager_get_widget(app_ui_data->ui_manager,
                                               "/ToolBar/pdfv_me_menu_screen_zoom_out");
            if (widget != NULL)
                gtk_widget_set_sensitive(widget, enable);

            widget = gtk_ui_manager_get_widget(app_ui_data->ui_manager,
                                               "/Popup/pdfv_me_menu_screen_zoom_out");
            if (widget != NULL)
                gtk_widget_set_sensitive(widget, enable);
            break;

        case DIM_ERROR:
            /* error case of out-of-memory / corrupted page */

            /* zoom functionality */
            ui_enable_page_controls(app_ui_data, DIM_ZOOM_IN, enable);
            ui_enable_page_controls(app_ui_data, DIM_ZOOM_OUT, enable);
            break;

        case DIM_PREV:
            widget = gtk_ui_manager_get_widget(app_ui_data->ui_manager,
                                               "/ToolBar/pdfv_me_menu_page_previous");
            if (widget != NULL)
                gtk_widget_set_sensitive(widget, enable);

            widget = gtk_ui_manager_get_widget(app_ui_data->ui_manager,
                                               "/ToolBar/pdfv_me_menu_page_first");
            if (widget != NULL)
                gtk_widget_set_sensitive(widget, enable);

            widget = gtk_ui_manager_get_widget(app_ui_data->ui_manager,
                                               "/Popup/pdfv_me_menu_page_previous");
            if (widget != NULL)
                gtk_widget_set_sensitive(widget, enable);

            break;

        case DIM_NEXT:
            widget = gtk_ui_manager_get_widget(app_ui_data->ui_manager,
                                               "/ToolBar/pdfv_me_menu_page_next");
            if (widget != NULL)
                gtk_widget_set_sensitive(widget, enable);

            widget = gtk_ui_manager_get_widget(app_ui_data->ui_manager,
                                               "/ToolBar/pdfv_me_menu_page_last");
            if (widget != NULL)
                gtk_widget_set_sensitive(widget, enable);

            widget = gtk_ui_manager_get_widget(app_ui_data->ui_manager,
                                               "/Popup/pdfv_me_menu_page_next");
            if (widget != NULL)
                gtk_widget_set_sensitive(widget, enable);

            break;

        case DIM_SWITCH_TO:
            gtk_action_set_sensitive(app_ui_data->current_page, enable);
            break;
        case DIM_ALL:
            /* call other dim functions */
            ui_enable_page_controls(app_ui_data, DIM_ZOOM_IN, enable);
            ui_enable_page_controls(app_ui_data, DIM_ZOOM_OUT, enable);
            ui_enable_page_controls(app_ui_data, DIM_PREV, enable);
            ui_enable_page_controls(app_ui_data, DIM_NEXT, enable);
            ui_enable_page_controls(app_ui_data, DIM_SWITCH_TO, enable);

            /* change the state of any other document controls as well */
            ui_enable_document_controls(app_ui_data, enable);
            break;
    }
}


/**
   Enables / disables scrollbars
   
   @param app_ui_data
   @param henable - enable horizontal scrollbar
   @param venable - enable vertical scrollbar
*/
void
ui_enable_scrollbars(AppUIData * app_ui_data,
                     gboolean henable, gboolean venable)
{
    g_return_if_fail(app_ui_data != NULL);
    g_return_if_fail(app_ui_data->hscroll != NULL);
    g_return_if_fail(app_ui_data->vscroll != NULL);

    henable ? gtk_widget_show(app_ui_data->hscroll) :
        gtk_widget_hide(app_ui_data->hscroll);
    venable ? gtk_widget_show(app_ui_data->vscroll) :
        gtk_widget_hide(app_ui_data->vscroll);
}

/**
   Set sensitive of scrollbars (to prevent scrolling)
   @param app_ui_data
   @param sensitive TRUE to set sensitive, FALSE to make insensitive
*/
void
ui_scrollbars_sensitive( AppUIData * app_ui_data, gboolean sensitive ) {
	gtk_widget_set_sensitive( app_ui_data->hscroll, sensitive );
	gtk_widget_set_sensitive( app_ui_data->vscroll, sensitive );	
}


void
ui_load_state(AppUIData * app_ui_data, AppState * app_state)
{
    /* check input */
    g_return_if_fail(app_ui_data != NULL);
    g_return_if_fail(app_state != NULL);

    /* adjust UI to the state */
    app_ui_data->dpi = app_state->dpi;

    /* set show images flag */
    if (app_state->show_images != PDF_FLAGS_IS_SET(app_ui_data->flags,
                                                   PDF_FLAGS_SHOW_IMAGES))
    {
        PDF_FLAGS_TOGGLE(app_ui_data->flags, PDF_FLAGS_SHOW_IMAGES);
    }

    /* set fullscreen mode on or off */
    ui_toggle_fullscreen(app_ui_data, app_state->fullscreen);
}

/**
   Gets if tool item is sensitive
   
   @param app_ui_data - structure
   @param PDFdim which item
   @return sensitive or not
*/
gboolean
ui_get_toolitem_is_sensitive(AppUIData * app_ui_data, PDFDim dim)
{
    gchar *path = NULL;

    g_assert(app_ui_data);
    g_return_val_if_fail(app_ui_data->ui_manager != NULL, FALSE);

    switch (dim)
    {
        case DIM_PREV:
            path = "/ToolBar/pdfv_me_menu_page_previous";
            break;
        case DIM_NEXT:
            path = "/ToolBar/pdfv_me_menu_page_next";
            break;
        case DIM_ZOOM_IN:
            path = "/ToolBar/pdfv_me_menu_screen_zoom_in";
            break;
        case DIM_ZOOM_OUT:
            path = "/ToolBar/pdfv_me_menu_screen_zoom_out";
            break;
        default:
            g_assert_not_reached();
    }


    return ((path != NULL) ?
            GTK_WIDGET_IS_SENSITIVE(gtk_ui_manager_get_widget
                                    (app_ui_data->ui_manager, path)) : FALSE);
}

/**
   Show  Maximum & Minimum zooming banners in application window

   @param app_ui_data AppUIData object (not NULL)
*/
    void ui_show_zoom_banner(AppUIData * app_ui_data)
{
    if (!app_ui_data->app_data->low_memory
        && !PDF_FLAGS_IS_SET(app_ui_data->flags, PDF_FLAGS_PAGE_ERROR)
        && (pdf_viewer_get_num_pages() > 0) &&
        pdf_viewer_get_current_zoom() == 288)
    {
	ui_show_banner(GTK_WIDGET(app_ui_data->app_view),
		       _("pdfv_ib_maximum_zoom"));	 
	return;
    }
		 
    if (!app_ui_data->app_data->low_memory
        && !PDF_FLAGS_IS_SET(app_ui_data->flags, PDF_FLAGS_PAGE_ERROR)
        && (pdf_viewer_get_num_pages() > 0
        && pdf_viewer_get_current_zoom() <= 36))
    {
	ui_show_banner(GTK_WIDGET(app_ui_data->app_view),
		      _("pdfv_ib_minimum_zoom"));	
    }
}

/**
   Shows info banner
   
   @param app_window - application window
   @param msg - text message
*/
void
ui_show_banner(GtkWidget * widget, const gchar * msg)
{

    // g_assert(widget);

    if (msg)
    {
        hildon_banner_show_information(widget, NULL, msg);
    }
}

/**
   Shows progress banner
   
   @param app_window - application windows
   @param msg - text message
*/
GtkWidget *
ui_show_progress_banner(GtkWindow * app_window, const gchar * msg)
{

	GtkWidget * w;
    g_assert(app_window);
    g_return_val_if_fail( msg != NULL, NULL );
    
	w = hildon_banner_show_animation( GTK_WIDGET( app_window ), NULL, msg );
	g_assert( w != NULL );
	
    gtk_widget_realize(w);
    gdk_window_process_all_updates();
    gdk_window_invalidate_rect(w->window, NULL, TRUE);
    gdk_window_process_all_updates();
    return w;
}

/**
   Shows infonote
   
   @param app_ui_data - application data
   @param msg - text message
*/
void
ui_show_note(AppUIData *app_ui_data, const gchar * msg)
{
    GtkWidget *note;

    g_assert(app_ui_data->app_view);

    if (msg)
    {
        app_ui_data->note_dialog = note = hildon_note_new_information(GTK_WINDOW(app_ui_data->app_view), msg);

        gtk_dialog_run(GTK_DIALOG(note));
        gtk_widget_destroy(GTK_WIDGET(note));
	app_ui_data->note_dialog = NULL;
    }
}

/**
   Shows result
   
   @param app_ui_data - structure
   @param result - the result to show
*/
void
ui_show_result(AppUIData * app_ui_data, PDFViewerResult result)
{

    gchar *msg = NULL;

    g_return_if_fail(app_ui_data != NULL);

    switch (result)
    {
        case RESULT_INVALID_INTERFACE:
            msg = _("pdfv_ib_incorrect_interface");
            break;
        case RESULT_SAVE_OK:
            msg = _("pdfv_error_successfulsave");
            break;
        case RESULT_SAVE_FAILED:
            msg = _("pdfv_error_savefail");
            break;
        case RESULT_UNSUPPORTED_FORMAT:
            ui_show_note(app_ui_data,
                         _("pdfv_ni_unsupported_file"));
            break;
        case RESULT_CORRUPTED_FILE:
            ui_show_note(app_ui_data,
                         _("pdfv_error_documentcorrupted"));
            break;
        case RESULT_INSUFFICIENT_MEMORY:
            ui_show_note(app_ui_data,
                         _("pdfv_ni_not_enough_memory"));
            break;
        case RESULT_INVALID_URI:
            msg = _("pdfv_ni_document_not_found");
            break;
        case RESULT_NO_SPACE_ON_DEVICE:
            msg = _("pdfv_error_notenoughmemorysave");
            break;
        case RESULT_INTERRUPTED_MMC_OPEN:
            msg = _("pdfv_error_memorycard");
            break;
        case RESULT_SAVING_NOT_COMPLETED:
            msg = _("pdfv_ib_fileinuse");
            break;
        default:
            msg = NULL;
    }

    if (msg)
        ui_show_banner(GTK_WIDGET(app_ui_data->app_view), msg);
}

/**
   Sets zoom
   
   @param app_ui_data
   @param value - new zoom value
*/
void
ui_set_current_zoom(AppUIData * app_ui_data, int value)
{
    gchar *zoomformat = _("pdfv_tb_zoom_level");
    gchar *tmp = g_strdup_printf(zoomformat, value);
    gchar *text = g_strdup_printf(value < 100 ? " %s " : "%s",
                                  tmp);

    gtk_widget_queue_resize_no_redraw(app_ui_data->current_zoom_value);
    g_object_set(app_ui_data->current_zoom_value, "label", text, NULL);
    g_free(text);
    g_free(tmp);
}
/* EOF */
