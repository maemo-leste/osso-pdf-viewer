/**
    @file OssoStream.cc

    Copyright (C) 2005-06 Nokia Corporation

    This class is based on GnomeVFSStream class in GPdf.
    GPdf is available at: http://www.purl.org/NET/gpdf

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

#include "OssoStream.h"
#include "gtk-switch.h"
#include <gtk/gtk.h>
#include "gtk-switch.h"

#ifndef NO_DECRYPTION
#include "Decrypt.h"
#endif

OssoStream::OssoStream(GFileInputStream *handleA, Guint startA, GBool limitedA,
		       Guint lengthA, Object *dictA):BaseStream(dictA) {
	
  handle = handleA;
  start = startA;
  limited = limitedA;
  length = lengthA;
  buffPtr = buffEnd = buff;
  buffPos = start;
  savePos = 0;
  saved = gFalse;
}

OssoStream::~OssoStream(){
  close();
}

Stream* OssoStream::makeSubStream(Guint startA, GBool limitedA,
				  Guint lengthA, Object *dictA) {
  return new OssoStream(handle, startA, limitedA, lengthA, dictA);

}

void OssoStream::reset() {
  goffset offsetReturn;

  offsetReturn = g_seekable_tell((GSeekable*)handle);
    savePos = (Guint)offsetReturn;
    saved = gTrue;

  g_seekable_seek((GSeekable*)handle, start, G_SEEK_SET, NULL, NULL);
  
  buffPtr = buffEnd = buff;
  buffPos = start;
#ifndef NO_DECRYPTION
  if(decrypt)
    decrypt->reset();
#endif
}

void OssoStream::close(){
  if(saved) {
    g_seekable_seek((GSeekable*)handle, savePos, G_SEEK_SET, NULL, NULL);
    saved = gFalse;
  }
}

extern gboolean _pdf_abort_rendering;

GBool OssoStream::fillBuff() {
  int n;
  GError *error = NULL;
  gssize bytesRead;
#ifndef NO_DECRYPTION
  char *p;
#endif
	
/*    while (!_pdf_abort_rendering && gtk_events_pending ()) {
      gtk_main_iteration ();
    }*/
    
  if (_pdf_abort_rendering)
    return gFalse;
    
  buffPos += buffEnd - buff;
  buffPtr = buffEnd = buff;
  if( limited && buffPos >= start + length ) {
    return gFalse;
  }
  if( limited && buffPos + gioStreamBufSize > start + length ) {
    n = start + length - buffPos;
  } else {
    n = gioStreamBufSize;
  }
  bytesRead = g_input_stream_read((GInputStream*)handle, buff, n, NULL, &error);
  if (error != NULL ) {
    fprintf(stderr, "OssoStream::fillBuff g_input_stream_read: error: g_input_stream_read: %s\n", error->message);
    g_error_free(error);
    return gFalse;
  }

  buffEnd = buff + bytesRead;
  if( buffPtr >= buffEnd ) {
    return gFalse;
  }
	
#ifndef NO_DECRYPTION
  if (decrypt) {
    for (p = buff; p < buffEnd; ++p) {
      *p = (char)decrypt->decryptByte((Guchar)*p);
    }
  }
#endif
	
  return gTrue;
}

void OssoStream::setPos(Guint pos, int dir) {
  GError *error = NULL;
  goffset offsetReturn;

  if( dir >= 0 ) {

    g_seekable_seek((GSeekable*)handle, pos, G_SEEK_SET, NULL, &error);
    if (error == NULL) {
        buffPos = pos;
    } else {
        // VFS code also didn't do anything special on error.
        fprintf(stderr, "OssoStream::setPos: error: g_seekable_seek: %s\n", error->message);
        g_error_free(error);
    }
  } else {
    g_seekable_seek((GSeekable*)handle, 0, G_SEEK_END, NULL, &error);
    if (error != NULL) {
        // XXX: act on this?
        g_error_free(error);
        goto end;
    }

    offsetReturn = g_seekable_tell((GSeekable*)handle);
    buffPos = (Guint)offsetReturn;
    if (pos > buffPos) {
        pos = (Guint)buffPos;
    }

    g_seekable_seek((GSeekable*)handle, -(int)pos, G_SEEK_END, NULL, &error);
    if (error != NULL) {
        // XXX: act on this?
        g_error_free(error);
        goto end;
    }

    offsetReturn = g_seekable_tell((GSeekable*)handle);
    buffPos = (Guint)offsetReturn;
  }

end:
  buffPtr = buffEnd = buff;
}

void OssoStream::moveStart(int delta) {
  start += delta;
  buffPtr = buffEnd = buff;
  buffPos = start;
}
