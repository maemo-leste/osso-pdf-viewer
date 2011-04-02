/* ========================================================================= *
 * File: chunk.h
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
 *    Support for Object Pool chunk - space that requested from the OS
 *    and stores actual objects.
 *
 * History:
 *
 * 18-Jul-2006 Leonid Moiseichuk
 * - initial version created.
 * ========================================================================= */

#ifndef OBJECT_POOL_CHUNK_H_USED
#define OBJECT_POOL_CHUNK_H_USED

/* ========================================================================= *
 * Includes.
 * ========================================================================= */

#include <stdio.h>
#include <stdlib.h>

#include "defines.h"


/* ========================================================================= *
 * Interface.
 * ========================================================================= */

namespace ObjectPool
{

   /* This piece of memory requested from the OS. All chunks in pool have the same   */
   /* parameters like size and object sizeof, so these values are stored in the pool */

   class Chunk
   {
      public:

         /* Structure to store shared information in the Pool */
         struct Shared
         {
            /* Maximal size of object allocated in chunk */
            unsigned    objectSizeOf;
            /* Number of objects that can be allocated in chunk */
            unsigned    objectCounter;

            /* Offset to bitmap that stored usage/free bits */
            unsigned    bitmapOffset;
            /* Counter of items in bitmap (now unsigned words)*/
            unsigned    bitmapCounter;

            /* Offset to memory that used to allocate/free */
            unsigned    memoryOffset;
            /* Counter of bytes that allocated to store data */
            unsigned    memoryCounter;

            /* Size of whole chunk (header, bitmap and memory)*/
            unsigned    chunkSize;
         };

      private:

         /* Who is next? */
         Chunk*         mySuccessor;

         /* That is header data which is unique for every chunk */
         unsigned       myUsedCounter;
         unsigned       myFreeCounter;

         /* Data type for storing bitmap data */
         typedef unsigned  BITMAP;
         #define BITMAP_NO_FREE           ((BITMAP)-1)
         #define BITMAP_CAPACITY          (sizeof(BITMAP) * 8)

         /* Head of this chunk in memory */
         #define THIS_CHUNK_HEAD()        ((unsigned char*)this)

         /* Tail of this chunk in memory */
         #define THIS_CHUNK_TAIL(s)       (THIS_CHUNK_HEAD() + (s).chunkSize)

         /* Allocated bitmap data which aligned to OP_MEMORY_ALIGNMENT */
         #define THIS_CHUNK_BITMAP(s)    ((BITMAP*)(THIS_CHUNK_HEAD() + (s).bitmapOffset))

         /* The next is memory allocated for storing objects, also aligned  */
         #define THIS_CHUNK_MEMORY(s)    (THIS_CHUNK_HEAD() + (s).memoryOffset)

      public:

         /* Allocates memory slot and returns pointer or NULL if allocation failed */
         unsigned char* allocate(const Chunk::Shared& shared);

         /* Release memory, returns true if operation passed or false otherwise */
         bool release(const Chunk::Shared& shared, unsigned char* object);

         /* Returns true if memory for allocation is available */
         bool avail() const;
         /* Returns true if all memory is released */
         bool empty() const;

         /* Elements of sigle list control */
         Chunk*   succ() const;
         void     succ(Chunk* aSuccessor);

         /* Display information about chunk */
         void info(FILE* output) const;

         /* Initialize shared information to manage chunk in pool */
         /* Shall be called once during the pool initialization   */
         static void setup(Chunk::Shared& shared, unsigned objectSizeOf, unsigned chunkSize);

         /* Form of constructor to create chunk object with specified parameters */
         static Chunk* create(const Chunk::Shared& shared);

      private:    /* Usage of these methods is prohibited outside */
         Chunk();
         ~Chunk();
         Chunk(const Chunk&);
         void operator=(const Chunk&);

         bool occupyFreeSlot(const Chunk::Shared& shared, unsigned& o_index);

   }; /* Class Chunk */


   inline Chunk :: Chunk()
   {}

   inline Chunk :: ~Chunk()
   {}

   inline Chunk* Chunk :: create(const Chunk::Shared& shared)
   {
      /* In this operation malloc is faster than new */
      Chunk* chunk = (Chunk*)malloc(shared.chunkSize);
      if ( chunk )
      {
         BITMAP* bitmap = (BITMAP*)((unsigned char*)chunk + shared.bitmapOffset);
         BITMAP* finish = bitmap + shared.bitmapCounter;

         chunk->mySuccessor   = NULL;
         chunk->myFreeCounter = shared.objectCounter;
         chunk->myUsedCounter = 0;

         /* Bitmaps are usually short (less 128 words), so use memset are not effective */
         while (bitmap < finish)
            *bitmap++ = 0;
      }

      return chunk;
   } /* create */

   inline bool Chunk :: avail() const
   {
      return (myFreeCounter > 0);
   }

   inline bool Chunk :: empty() const
   {
      return (0 == myUsedCounter);
   }

   inline Chunk* Chunk :: succ() const
   {
      return mySuccessor;
   }

   inline void Chunk :: succ(Chunk* aSuccessor)
   {
      mySuccessor = aSuccessor;
   }

   inline unsigned char* Chunk :: allocate(const Chunk::Shared& shared)
   {
      if ( myFreeCounter > 0 )
      {
         unsigned index;

         if ( occupyFreeSlot(shared, index) )
         {
            myFreeCounter--;
            myUsedCounter++;

            return (THIS_CHUNK_MEMORY(shared) + index * shared.objectSizeOf);
         }
      }

      return NULL;
   } /* allocate */

   inline bool Chunk :: release(const Chunk::Shared& shared, unsigned char* object)
   {
      unsigned char* memory = THIS_CHUNK_MEMORY(shared);

      if (memory <= object && object < THIS_CHUNK_TAIL(shared))
      {
         BITMAP* bitmap = THIS_CHUNK_BITMAP(shared);
         /* Calculate object index and corresponded index and mask for bitmap */
         const unsigned o_index = (object - memory) / shared.objectSizeOf;
         const unsigned b_index = o_index / BITMAP_CAPACITY;
         const BITMAP   b_mask  = 1 << (o_index % BITMAP_CAPACITY);

         if (bitmap[b_index] & b_mask)
         {
            bitmap[b_index] &= ~b_mask;
            myUsedCounter--;
            myFreeCounter++;
         }

         return true;
      }

      return false;
   } /* release */

} /* Namespace ObjectPool */

#endif /* OBJECT_POOL_CHUNK_H_USED */
