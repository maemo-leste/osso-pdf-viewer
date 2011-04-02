/**
    @file OssoOutputDev.cc

    Copyright (C) 2005-06 Nokia Corporation

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


#include "gtk-switch.h"
#include <gtk/gtk.h>
#include "appdata.h"
#include "gtk-switch.h"


#include "gmem.h"
#include "SplashTypes.h"
#include "SplashBitmap.h"
#include "Object.h"
#include "GfxState.h"

#include "OssoOutputDev.h"
#include "debug.h"
//#include "utility.h"

#include "thread_debug.h"

extern GThread *mainThread;

#define xoutRound(x) ((int)(x + 0.5))

OssoOutputDev::OssoOutputDev(GBool reverseVideoA, SplashColor paperColorA,
			     GBool installCmapA, int rgbCubeSize,
			     GBool incrementalUpdateA,
			     void (*redrawCbkA)(void *data),
			     void *redrawCbkDataA):
	SplashOutputDev(splashModeRGB8, 1, reverseVideoA, paperColorA), incrementalUpdate(1){
	
	//incrementalUpdate = incrementalUpdateA;
	redrawCbk = redrawCbkA;
	redrawCbkData = redrawCbkDataA;
}

OssoOutputDev::~OssoOutputDev(){
}

void OssoOutputDev::drawChar(GfxState *state, double x, double y,
			     double dx, double dy,
			     double originX, double originY,
			     CharCode code, int nBytes, Unicode *u, int uLen) {
  SplashOutputDev::drawChar(state, x, y, dx, dy, originX, originY,
			    code, nBytes, u, uLen);
}

GBool OssoOutputDev::beginType3Char(GfxState *state, double x, double y,
				       double dx, double dy,
				       CharCode code, Unicode *u, int uLen) {
  return SplashOutputDev::beginType3Char(state, x, y, dx, dy, code, u, uLen);
}

void OssoOutputDev::clear() {
  startDoc(NULL);
  startPage(0, NULL);
}

void OssoOutputDev::startPage(int pageNum, GfxState *state){
  SplashOutputDev::startPage(pageNum, state);
}	

void OssoOutputDev::endPage() {
  SplashOutputDev::endPage();
  if (!incrementalUpdate) {
    (*redrawCbk)(redrawCbkData);
  }
}

void OssoOutputDev::dump() {
TDB("OssoOutputDev::dump 1\n");
  if (incrementalUpdate && redrawCbk) {
    (*redrawCbk)(redrawCbkData);
  }
TDB("OssoOutputDev::dump 2\n");
}

void OssoOutputDev::updateFont(GfxState *state) {
  SplashOutputDev::updateFont(state);
}

void OssoOutputDev::redraw(AppUIData *app_ui_data) {
	
	//static GdkPixbuf *prev_pixbuf = NULL;
	int width, height;
	GdkPixbuf *pixbuf;
	int gdk_rowstride;
        //GdkPixbuf *p;
	
	g_return_if_fail(app_ui_data != NULL);

TDB("OssoOutputDev::redraw 1\n");
	/* disable the UI function while rendering */
//	PDF_FLAGS_SET( app_ui_data->flags, PDF_FLAGS_RENDERING );

	width = getBitmap()->getWidth();
	height = getBitmap()->getHeight();
	gdk_rowstride = getBitmap()->getRowSize();
	
//	OSSO_LOG_DEBUG("creating pixbuf from data; rendering: %d",
//		       PDF_FLAGS_IS_SET(app_ui_data->flags, PDF_FLAGS_RENDERING));	

TDB("OssoOutputDev::redraw 2 (%d,%d), %d, %p\n", width, height, gdk_rowstride, getBitmap()->getDataPtr());
	/* creating pixbuf from SplashRGB8P */
//  if (mainThread!=g_thread_self())
  DTRY(gdk);  
  GDK_THR_ENTER;
  DLOCKED(gdk);
	pixbuf = gdk_pixbuf_new_from_data(
		getBitmap()->getDataPtr(), GDK_COLORSPACE_RGB, FALSE, 8,
		width, height, gdk_rowstride, NULL, NULL);
TDB("OssoOutputDev::redraw 3, %p\n", pixbuf);
        if (pixbuf) {
                gtk_image_set_from_pixbuf(GTK_IMAGE(app_ui_data->page_image), pixbuf);
		g_object_unref(pixbuf);
          
        }
//        if (mainThread!=g_thread_self())
        GDK_THR_LEAVE;
	DUNLOCKED(gdk);
	
TDB("OssoOutputDev::redraw 6\n");
}

/* EOF */
