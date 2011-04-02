/* ========================================================================= *
 * File: defines.h
 *
 * Copyright (C) 2006 Nokia. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307 USA
 *
 * Author: Leonid Moiseichuk <leonid.moiseichuk@nokia.com>
 *
 * Description:
 *    Compile-time settings and macros for Object Pool allocator.
 *    Note: All definitions has OP_ prefix.
 *
 * History:
 *
 * 18-Jul-2006 Leonid Moiseichuk
 * - initial version created.
 * ========================================================================= */

#ifndef OBJECT_POOL_DEFINES_H_USED
#define OBJECT_POOL_DEFINES_H_USED

/* Detection of operating system - Windows or UNIX */
#if defined(WINDOWS) || defined(_WINDOWS) || defined(WIN32) || defined(__WIN32__)
#define OP_WINDOWS
#else
#define OP_UNIX
#endif

/* Minimal chunk size - piece of memory requested from the OS */
#define OP_MIN_CHUNK_SIZE     (4 * 1024)

/* Default size of chunk size (piece of memory requested from the OS) */
#define OP_DEF_CHUNK_SIZE     (32 * 1024)

/* Alignment of objects, according to ANSI C Standard shall be 16 */
#define OP_MEMORY_ALIGNMENT   (8)


/* Make capacity calculation for some array */
#define OP_CAPACITY(a)        (sizeof(a) / sizeof(*a))

/* Alignment of some value using a that is power of two */
#define OP_ALIGN(v,a)         (0 == ((v) & ((a) - 1)) ? (v) : (((v) + (a)) & ~((a) - 1)))

/* Enable statistics */
#define OP_STATISTICS         1

/* Enable tracing of internal variables */
#if 0
#include <stdio.h>
#define OP_TRACE(v)     fprintf(stdout, "%s %u: %s == %u\n", __FILE__, __LINE__, #v, (unsigned)(v))
#else
#define OP_TRACE(v)     ((void)0)
#endif

#endif /* OBJECT_POOL_DEFINES_H_USED */
