/**
    @file OssoStream.h

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


#ifndef OSSOSTREAM_H
#define OSSOSTREAM_H

#include "gtk-switch.h"
#include <libgnomevfs/gnome-vfs-handle.h>
#include "gtk-switch.h"
#include "Object.h"
#include "Stream.h"

#define gnomeVFSStreamBufSize fileStreamBufSize

class OssoStream: public BaseStream {
public:
	
        OssoStream(GnomeVFSHandle *handleA, Guint startA, GBool limitedA,
		   Guint lengthA, Object *dictA);
	virtual ~OssoStream();

	virtual Stream* makeSubStream(Guint startA, GBool limitedA,
				      Guint lengthA, Object *dictA);
	virtual StreamKind getKind() { return strFile;}
	virtual void reset();
	virtual void close();
	virtual int getChar() { return (buffPtr >= buffEnd && !fillBuff()) ? EOF : (*buffPtr++ & 0xff); }
	virtual int lookChar() { return (buffPtr >= buffEnd && !fillBuff()) ? EOF : (*buffPtr & 0xff); }
	virtual int getPos() { return buffPos + (buffPtr - buff); }
	virtual void setPos(Guint pos, int dir = 0);
	virtual GBool isBinary(GBool last = gTrue) { return last; }
	virtual Guint getStart() { return start; }
	virtual void moveStart(int delta);

private:
	
	GBool fillBuff();

	GnomeVFSHandle *handle;
	Guint start;
	GBool limited;
	Guint length;
	char buff[gnomeVFSStreamBufSize];
	char *buffPtr;
	char *buffEnd;
	Guint buffPos;
	int savePos;
	GBool saved;
};

#endif
