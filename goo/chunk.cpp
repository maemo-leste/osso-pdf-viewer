/* ========================================================================= *
 * File: chunk.cpp
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
 * 20-Jul-2006 Leonid Moiseichuk
 * - initial version created.
 * ========================================================================= */

/* ========================================================================= *
 * Includes.
 * ========================================================================= */

#include <string.h>
#include "chunk.h"

/* ========================================================================= *
 * Interface.
 * ========================================================================= */

namespace ObjectPool
{
   void Chunk :: info(FILE* output) const
   {
#if OP_STATISTICS
      fprintf(output, "  0x%p: free %u used %u succ 0x%p\n", (void*)this, myFreeCounter, myUsedCounter, (void*)mySuccessor);
#endif
   } /* info */


   void Chunk :: setup(Chunk::Shared& shared, unsigned objectSizeOf, unsigned chunkSize)
   {
      /* Clean-up whole shared structure */
      memset(&shared, 0, sizeof(Chunk::Shared));

      /* Initialize basic fields */
      shared.objectSizeOf = OP_ALIGN(objectSizeOf, OP_MEMORY_ALIGNMENT);
      shared.bitmapOffset = OP_ALIGN(sizeof(Chunk), OP_MEMORY_ALIGNMENT);

      /* Check requested chunk size */
      if (chunkSize < OP_MIN_CHUNK_SIZE)
         chunkSize = OP_MIN_CHUNK_SIZE;

      /* Calculate initial size of bitmap (and number of objects in chunk) */
      shared.bitmapCounter = (chunkSize / (shared.objectSizeOf << 1)) / BITMAP_CAPACITY;

      /* Calculate parameters of chunk for specified counter of objects */
      do
      {
         shared.bitmapCounter++;
         shared.objectCounter = shared.bitmapCounter * BITMAP_CAPACITY;
         shared.memoryOffset  = shared.bitmapOffset + OP_ALIGN(shared.bitmapCounter * sizeof(BITMAP), OP_MEMORY_ALIGNMENT);
         shared.memoryCounter = shared.objectCounter * shared.objectSizeOf;
         shared.chunkSize     = shared.memoryOffset  + shared.memoryCounter;
      } while (shared.chunkSize < chunkSize);
   } /* setup */


   #define TEST_FREE(t,m)  ((t) != ((t) & (m)))

   bool Chunk :: occupyFreeSlot(const Chunk::Shared& shared, unsigned& o_index)
   {
      /* Looking for free slot in bitmap */
      BITMAP*  bitmap = THIS_CHUNK_BITMAP(shared);
      unsigned index;
      unsigned bitno;

      for (index = 0; index < shared.bitmapCounter; index++)
      {
         const unsigned checked = bitmap[index];

         /* Check for available bits in this word */
         if (BITMAP_NO_FREE == checked)
            continue;

         /* Some bits are not used, determine which one      */
         /* WARNING: this statement is hardcoded for 32 bits */
         if ( TEST_FREE(0x0000ffff, checked) )
         {
            bitno = (TEST_FREE(0x000000ff, checked) ? 0 : 8);
         }
         else
         {
            bitno = (TEST_FREE(0x00ff0000, checked) ? 16 : 24);
         }

         /* Find the free block in bitmap */
         while (0 != (checked & (1 << bitno)))
            bitno++;

         /* We found that some bit in usage is zero --> occupy it */
         bitmap[index] |= (1 << bitno);
         o_index = index * BITMAP_CAPACITY + bitno;

         return true;

      }

      return false;
   } /* occupyFreeSlot */

} /* Namespace ObjectPool */

