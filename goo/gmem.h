/*
 * gmem.h
 *
 * Memory routines with out-of-memory checking.
 *
 * Copyright 1996-2003 Glyph & Cog, LLC
 */

#ifndef GMEM_H
#define GMEM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


/* Switching off this staff by default because it mostly useless */
#define GOO_MEMORY	0

#if GOO_MEMORY
	
/*
 * Same as malloc, but prints error message and exits if malloc()
 * returns NULL.
 */
extern void *gmalloc(int size);

/*
 * Same as realloc, but prints error message and exits if realloc()
 * returns NULL.  If <p> is NULL, calls malloc instead of realloc().
 */
extern void *grealloc(void *p, int size);

/*
 * These are similar to gmalloc and grealloc, but take an object count
 * and size.  The result is similar to allocating nObjs * objSize
 * bytes, but there is an additional error check that the total size
 * doesn't overflow an int.
 */
extern void *gmallocn(int nObjs, int objSize);
extern void *greallocn(void *p, int nObjs, int objSize);

/*
 * Same as free, but checks for and ignores NULL pointers.
 */
extern void gfree(void *p);

#ifdef DEBUG_MEM
/*
 * Report on unfreed memory.
 */
extern void gMemReport(FILE *f);
#else
#define gMemReport(f)
#endif

/*
 * Allocate memory and copy a string into it.
 */
extern char *copyString(char *s);


#else

#ifdef __cplusplus
#define VISIBILITY		::
#else
#define VISIBILITY
#endif

#define gmalloc(size)			(VISIBILITY malloc(size))
#define grealloc(ptr,size)		(VISIBILITY realloc(ptr, size))
#define gmallocn(n_obj,s_obj)		(VISIBILITY malloc((n_obj) * (s_obj)))
#define greallocn(ptr,n_obj,s_obj)	(VISIBILITY realloc(ptr, (n_obj) * (s_obj)))
#define gfree(ptr)			(VISIBILITY free(ptr))



/* Faster version of string duplication function */
static inline char* copyString(const char* s) 
{
   const size_t length = (s ? strlen(s) + 1 : 1);
   char* copy = (char*)gmalloc(length);

   if (length > 1)
	memcpy(copy, s, length);
   else
	*copy = (char)0;

   return copy;
} /* copyString */


#endif /* if GOO_MEMORY */


#ifdef __cplusplus
}
#endif

#endif
