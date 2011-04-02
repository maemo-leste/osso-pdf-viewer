/**
    @file constant.h

    Constants.

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


#ifndef CONSTANT_H
#define CONSTANT_H


/* TODO remove these once they are available in hildon headers */
#define HILDON_RESPONSE_FIND  -5
#define HILDON_RESPONSE_CLOSE -6


/**
	D-BUS
*/

/* D-BUS service definitions */
#define OSSO_PDFVIEWER_PACKAGE     "osso_pdfviewer"
#define OSSO_PDFVIEWER_SERVICE     "com.nokia.osso_pdfviewer"
#define OSSO_PDFVIEWER_OBJECT_PATH "/com/nokia/osso_pdfviewer"
#define OSSO_PDFVIEWER_INTERFACE   "com.nokia.osso_pdfviewer"

/* Define d-bus messages, to which application have to react */

/* Testing d-bus messaging with displaying infoprint */
#define DBUS_METHOD_DISPLAY_INFOPRINT "osso_pdfviewer_display_infoprint"

/* mime_open message */
#define DBUS_METHOD_MIME_OPEN "mime_open"


/**
	G-Conf
*/

/* Application specific GConf keys */
#define GCONF_KEY_DPI          "/apps/osso/pdfviewer/dpi"
#define GCONF_KEY_FIRST_RUN    "/apps/osso/pdfviewer/first_run"
#define GCONF_KEY_IMAGES       "/apps/osso/pdfviewer/images"
#define GCONF_KEY_LAST_FILE    "/apps/osso/pdfviewer/last_file"
#define GCONF_KEY_PASSWORD     "/apps/osso/pdfviewer/passwd"

#define SETTINGS_FACTORY_DEFAULT_FOLDER "MyDocs/.documents/"

/**
	Other
*/

/* under /usr/share/applications; required for mimetypes parsing. */
#define DESKTOP_FILE_NAME "hildon/osso_pdfviewer.desktop"

/* 
 * Threshold to start panning not moving the stylus over 30pix */
#define PANNING_THRESHOLD 30

/* Uncomment to enable memory-saving mode (slows down >150% zooms) */
// #define LOWMEM
#undef LOWMEM

/* Buffer size in case of partial rendering */
#ifndef LOWMEM
#define BUFFER_WIDTH  1200      // was: 1600
#define BUFFER_HEIGHT (720*2)   // was: 960*2
#endif

#ifdef LOWMEM
#define VIEWPORT_BUFFER_WIDTH  672
#define VIEWPORT_BUFFER_HEIGHT 362
#define FULLSCREEN_BUFFER_WIDTH  800
#define FULLSCREEN_BUFFER_HEIGHT 480
#endif

/* Maximum number of recent documents to store */
#define MAX_PASSWORD_RETRIES 3

/* PDF file extension */
#define PDFV_FILE_EXTENSION_DOT ".pdf"

/* viewport size, excluding scrollbars */
#define SCROLLBAR_SIZE       22
#define HSCROLLBAR_INCREMENT 20.0
#define VSCROLLBAR_INCREMENT 20.0
#define VIEWPORT_WIDTH       672
#define VIEWPORT_HEIGHT      362
#define FULLSCREEN_WIDTH     800
#define FULLSCREEN_HEIGHT    480

/* Timeouts */
/* time required to hold stylus down for CSM */
#define TIME_HOLD    1200

/* SAVE TIMEOUT in seconds */
#define SAVE_TIMEOUT 0.5

/* Application errors */
typedef enum {
    PDFV_NO_ERROR = 0,
    PDFV_ERROR_INSUFFICIENT_MEMORY
} AppError;


/* Keys for document details */
#define DOCUMENT_DETAILS_TITLE    "Title"
#define DOCUMENT_DETAILS_AUTHOR   "Author"
#define DOCUMENT_DETAILS_SUBJECT  "Subject"
#define DOCUMENT_DETAILS_KEYWORDS "Keywords"


/* enabling/disabling page controls */
typedef enum {
    DIM_ZOOM_IN = 0,
    DIM_ZOOM_OUT,
    DIM_ERROR,
    DIM_PREV,
    DIM_NEXT,
    DIM_SWITCH_TO,
    DIM_ALL
} PDFDim;

/* initial page number */
#define PDF_PAGE_INIT 0

/* the default screen DPI */
#define SCREEN_DPI 72

/* enviromental variable, set where the MMC is mounted */
#define MMC_MOUNTPOINT_ENV "MMC_MOUNTPOINT"
#define TEMP_DIR_PATH      "/var/tmp"
#define URI_FILE_PREFIX   "file://"

/* temporary file name when opening via bluetooth from gateway device */
#define GATEWAY_TMP_FILE "/var/tmp/.__gateway.pdf"

/* Use this macro to get rid of warnings of localization strings give when
 * used in printf's */
#define SUPPRESS_FORMAT_WARNING(x) ((char *)(long)(x))

/* Reserved space on flash partition on N800
   Made little bigger in Sep07, was 2408+300 */
#define RESERVED_SPACE 4000
#define KB_SIZE 1024

#endif /* CONSTANT_H */
