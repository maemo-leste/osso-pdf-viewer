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

#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>


/* #define TDB(args...) do { \ printf(args); \ FILE *bla; \
 * bla=fopen("/tmp/testpdf.txt","a+"); \ (fprintf(bla, "[pdfviewer]: "),
 * fprintf (bla, args)); \ fflush(bla); \ fclose(bla); \ } while (0) */


// #define TDB2(args...) (fprintf(bla, "[pdfviewer]: "), printf args)

#define TDB(args...) g_debug(args)

#define GDK_THR_ENTER do { \
  TDB("Locking: %s\n",__FUNCTION__); \
  gdk_threads_enter(); \
}while (0)


#define GDK_THR_LEAVE do { \
  TDB("UnLocking: %s\n",__FUNCTION__); \
  gdk_threads_leave(); \
}while (0)



#endif
