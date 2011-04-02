/**
    @file OssoOutputDev.h

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


#ifndef OSSOOUTPUTDEV_H
#define OSSOOUTPUTDEV_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "appdata.h"

#include "SplashTypes.h"
#include "SplashOutputDev.h"
//------------------------------------------------------------------------

#define xOutMaxRGBCube 6	// max size of RGB color cube

//------------------------------------------------------------------------
// OssoOutputDev
//------------------------------------------------------------------------
class OssoOutputDev: public SplashOutputDev {
	
public:
	OssoOutputDev(GBool reverseVideoA, SplashColor paperColorA,
								GBool installCmapA, int rgbCubeSize,
								GBool incrementalUpdateA,
								void (*redrawCbkA)(void *data),
								void *redrawCbkDataA);
	virtual ~OssoOutputDev();

  //----- initialization and control

  // Start a page.
  virtual void startPage(int pageNum, GfxState *state);

  // End a page.
  virtual void endPage();

  // Dump page contents to display.
  virtual void dump();

  //----- update text state
  virtual void updateFont(GfxState *state);

  //----- text drawing
  virtual void drawChar(GfxState *state, double x, double y,
			double dx, double dy,
			double originX, double originY,
			CharCode code, int nBytes, Unicode *u, int uLen);
 
  virtual GBool beginType3Char(GfxState *state, double x, double y,
			       double dx, double dy,
			       CharCode code, Unicode *u, int uLen);

  //----- special access

  // Clear out the document (used when displaying an empty window).
  void clear();

  // Copy the rectangle (srcX, srcY, width, height) to (destX, destY)
  // in destDC.
/*  void redraw(int srcX, int srcY,
	      Drawable destDrawable, GC destGC,
	      int destX, int destY,
	      int width, int height);
*/

  void redraw(AppUIData *app_ui_data);
  
private:

  GBool incrementalUpdate;      // incrementally update the display?
  void (*redrawCbk)(void *data);
  void *redrawCbkData;

  Guint depth;			// visual depth
  GBool trueColor;		// set if using a TrueColor visual
  int rDiv, gDiv, bDiv;		// RGB right shifts (for TrueColor)
  int rShift, gShift, bShift;	// RGB left shifts (for TrueColor)
  int rgbCubeSize;		// size of color cube (for non-TrueColor)
  Gulong			// color cube (for non-TrueColor)
    colors[xOutMaxRGBCube * xOutMaxRGBCube * xOutMaxRGBCube];
};
void destroy_pixbuf_data(guchar *pixels, gpointer data);

#endif
