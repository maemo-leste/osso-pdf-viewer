/* ========================================================================= *
 * File: pool.h
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
 *    Support for Object Pool - top level object to control allocation and deallocation.
 *
 * History:
 *
 * 19-Jul-2006 Leonid Moiseichuk
 * - initial version created.
 * ========================================================================= */

#ifndef OBJECT_POOL_POOL_H_USED
#define OBJECT_POOL_POOL_H_USED

/* ========================================================================= *
 * Includes.
 * ========================================================================= */

#include <stdio.h>
#include <stdlib.h>

#include "defines.h"
#include "guard.h"
#include "chunk.h"


/* ========================================================================= *
 * Interface.
 * ========================================================================= */

namespace ObjectPool
{

   /* Pool is managing a set of objects that located in chunk(s).      */
   /* Every object will have predefined size not bigger than requested */
   /* If you trying to allocate big object the standard new operator   */
   /* be called automatical.                                           */
   template <class AccessPolicy>
   class Pool
   {
      private:

         /* Object that necessary for synchronization of access to this pool */
         AccessPolicy   myAccess;

         /* Shared chunks information */
         Chunk::Shared  myShared;

         /* Pointers to first chunk in list */
         Chunk*         myChunk;

      public:

         /* Constructor of pool object. You can specify 2 parameters:    */
         /* - objectSizeOf - size of every allocated object, pool will   */
         /*   provide the requested or greater size. Bigger objects will */
         /*   be allocated using standard operator new.                  */
         /* - chunkSize - prefered size of chunk, maybe a bit bigger due */
         /*   to chunk support information and alignment.                */
         Pool(unsigned objectSizeOf, unsigned chunkSize=OP_DEF_CHUNK_SIZE);
         ~Pool();

         void* allocate(size_t size);
         void  release(void* ptr);

         void  info(FILE* output);

      private:    /* Usage of these methods is prohibited outside */
         Pool();
         Pool(const Pool&);
         void operator=(const Pool&);

         void escalate(Chunk* pred);

   }; /* Class Pool */


   template <class AccessPolicy>
   inline Pool<AccessPolicy> :: Pool()
   {}

   template <class AccessPolicy>
   inline Pool<AccessPolicy> :: Pool(unsigned objectSizeOf, unsigned chunkSize)
      : myAccess()
   {
      Guard<AccessPolicy> guard(myAccess);
      Chunk::setup(myShared, objectSizeOf, chunkSize);
      myChunk = NULL;
   } /* Pool */

   template <class AccessPolicy>
   Pool<AccessPolicy> :: ~Pool()
   {
      Guard<AccessPolicy> guard(myAccess);
      while ( myChunk )
      {
         Chunk* helper = myChunk;
         myChunk = myChunk->succ();
         free(helper);
      }
   } /* ~Pool */

   template <class AccessPolicy>
   inline void Pool<AccessPolicy> :: escalate(Chunk* pred)
   {
      if ( pred )
      {
         Chunk* curr = pred->succ();

         pred->succ(curr->succ());
         curr->succ(myChunk);
         myChunk = curr;
      }
   } /* escalate */


   template <class AccessPolicy>
   void* Pool<AccessPolicy> :: allocate(size_t size)
   {
      Guard<AccessPolicy> guard(myAccess);

      if (size <= myShared.objectSizeOf)
      {
         Chunk* previous;
         Chunk* cursor;

         /* Search for chunk that can allocate object */
         for (previous = NULL, cursor = myChunk; cursor; previous = cursor, cursor = cursor->succ())
         {
            if (void* object = cursor->allocate(myShared))
            {
               escalate(previous);
               return object;
            }
         }

         /* No free slots in chunks - add a new chunk */
         cursor = Chunk::create(myShared);
         if ( cursor )
         {
            cursor->succ(myChunk);
            myChunk = cursor;
            return cursor->allocate(myShared);
         }
      }

      /* We can also return null but be safe */
      OP_TRACE(size);
      return ::operator new(size, std::nothrow);
   } /* allocate */

   template <class AccessPolicy>
   void Pool<AccessPolicy> :: release(void* ptr)
   {
      Guard<AccessPolicy> guard(myAccess);
      Chunk* previous;
      Chunk* cursor;

      for (previous = NULL, cursor = myChunk; cursor; previous = cursor, cursor = cursor->succ())
      {
         if ( cursor->release(myShared, (unsigned char*)ptr) )
         {
            if ( cursor->empty() )
            {
               if ( previous )
                  previous->succ(cursor->succ());
               else
                  myChunk = cursor->succ();

               free(cursor);
            }
            else
            {
               /* Escalate this chunk because it has free slots */
               escalate(previous);
            }

            return;
         }
      }

      /* Object is located not in pool */
      OP_TRACE(ptr);
      ::operator delete(ptr, std::nothrow);
   } /* release */

   template <class AccessPolicy>
   void Pool<AccessPolicy> :: info(FILE* output)
   {
#if OP_STATISTICS
      Guard<AccessPolicy> guard(myAccess);
      const Chunk* cursor = myChunk;

      fprintf(output, "characteristics of pool 0x%08x:\n", (unsigned)this);
      fprintf(output, "- objectSizeOf  %u\n", myShared.objectSizeOf);
      fprintf(output, "- objectCounter %u\n", myShared.objectCounter);
      fprintf(output, "- bitmapOffset  %u\n", myShared.bitmapOffset);
      fprintf(output, "- bitmapCounter %u\n", myShared.bitmapCounter);
      fprintf(output, "- memoryOffset  %u\n", myShared.memoryOffset);
      fprintf(output, "- memoryCounter %u\n", myShared.memoryCounter);
      fprintf(output, "- chunkSize     %u\n", myShared.chunkSize);

      for (cursor = myChunk; cursor; cursor = cursor->succ())
         cursor->info(output);
#endif
   } /* info */


} /* Namespace ObjectPool */

#endif /* OBJECT_POOL_POOL_H_USED */
