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
#include <libgnomevfs/gnome-vfs.h>
#include "gtk-switch.h"

#ifndef NO_DECRYPTION
#include "Decrypt.h"
#endif

OssoStream::OssoStream(GnomeVFSHandle *handleA, Guint startA, GBool limitedA,
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
  GnomeVFSFileSize offsetReturn;
  if( gnome_vfs_tell(handle, &offsetReturn) == GNOME_VFS_OK) {
    savePos = (Guint)offsetReturn;
    saved = gTrue;
  }
  gnome_vfs_seek(handle, GNOME_VFS_SEEK_START, start);
  buffPtr = buffEnd = buff;
  buffPos = start;
#ifndef NO_DECRYPTION
  if(decrypt)
    decrypt->reset();
#endif
}

void OssoStream::close(){
  if(saved) {
    gnome_vfs_seek(handle, GNOME_VFS_SEEK_START, savePos);
    saved = gFalse;
  }
}

extern gboolean _pdf_abort_rendering;

GBool OssoStream::fillBuff() {
  int n;
  GnomeVFSFileSize bytesRead;
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
  if( limited && buffPos + gnomeVFSStreamBufSize > start + length ) {
    n = start + length - buffPos;
  } else {
    n = gnomeVFSStreamBufSize;
  }

  if(gnome_vfs_read(handle, buff, n, &bytesRead) != GNOME_VFS_OK) {
  //fprintf(stderr, "error: %s\n",gnome_vfs_result_to_string(result));
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
  if( dir >= 0 ) {
    if(gnome_vfs_seek(handle, GNOME_VFS_SEEK_START, pos) == GNOME_VFS_OK) {
      buffPos = pos;
    }
  } else {
    GnomeVFSFileSize offsetReturn;
    if(gnome_vfs_seek(handle, GNOME_VFS_SEEK_END, 0) ==
       GNOME_VFS_OK && 
       gnome_vfs_tell(handle, &offsetReturn) == GNOME_VFS_OK) {
      buffPos = (Guint)offsetReturn;
      if( pos > buffPos )
	pos = (Guint)buffPos;
      if(gnome_vfs_seek(handle, GNOME_VFS_SEEK_END, -(int)pos) ==
	 GNOME_VFS_OK &&
	 gnome_vfs_tell(handle, &offsetReturn) == GNOME_VFS_OK){
	buffPos = (Guint)offsetReturn;
      }
    }
  }
  buffPtr = buffEnd = buff;
}

void OssoStream::moveStart(int delta) {
  start += delta;
  buffPtr = buffEnd = buff;
  buffPos = start;
}
