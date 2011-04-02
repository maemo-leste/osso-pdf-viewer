/* ========================================================================= *
 * File: allocatable.h
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
 *    Class-level support of object pool allocator: pool allocatable object.
 *    Alternative way - using OBJECT_POOL_ALLOCATION macro to introduce
 *    this allocation to a class.
 *
 * History:
 *
 * 25-Jul-2006 Leonid Moiseichuk
 * - added macro to intoduce pool allocation without changes in class hierarchy.
 *
 * 20-Jul-2006 Leonid Moiseichuk
 * - initial version created.
 * ========================================================================= */

#ifndef OBJECT_POOL_ALLOCATABLE_H_USED
#define OBJECT_POOL_ALLOCATABLE_H_USED

/* ========================================================================= *
 * Includes.
 * ========================================================================= */

#include <stdio.h>
#include <new>

#include "lock.h"
#include "nolock.h"
#include "pool.h"


/* ========================================================================= *
 * Interface.
 * ========================================================================= */

namespace ObjectPool
{
   /* This version of allocatable object wraps specified class */
   /* and adds custom versions of new and delete operators.    */
   /* The same code can be added in any Customer class without */
   /* inheritance and templates if necessary                   */
   template <class Base, class AccessPolicy=NoLockPolicy>
   class Allocatable: public Base
   {
      public:

         static void* operator new(std::size_t size);
         static void  operator delete(void* object);
         static void info(FILE* output=stderr);

      private:

         /* This object is used to store allocations of Base class */
         static Pool<AccessPolicy>  ourPool;
   }; /* Class Allocatable */

   /* Common static data, shall be initialized outside class */
   template <class Base, class AccessPolicy>
   Pool<AccessPolicy> Allocatable<Base,AccessPolicy> :: ourPool(sizeof(Allocatable<Base,AccessPolicy>));

   template <class Base, class AccessPolicy>
   inline void* Allocatable<Base,AccessPolicy> :: operator new(std::size_t size)
   {
      return ourPool.allocate(size);
   } /* operator new */

   template <class Base, class AccessPolicy>
   inline void Allocatable<Base,AccessPolicy> :: operator delete(void* object)
   {
      ourPool.release(object);
   } /* operator delete */

   template <class Base, class AccessPolicy>
   void Allocatable<Base,AccessPolicy> :: info(FILE* output)
   {
#if OP_STATISTICS
      fprintf (output, "\ninformation from %s\n", __PRETTY_FUNCTION__);
      ourPool.info(output);
#endif
   } /* info */


   /* This is an alternative allocation - introduce new and delete */
   /* using macro. Good for quick testing without serious class    */
   /* hierarchy changes. Required corresponded _SUPPORT macro.     */
   #define OBJECT_POOL_ALLOCATION()                                  \
         private:                                                    \
            static ObjectPool::Pool<ObjectPool::LockPolicy> ourPool; \
         public:                                                     \
            static inline void* operator new(std::size_t size)       \
            {                                                        \
               return ourPool.allocate(size);                        \
            }                                                        \
            static inline void operator delete(void* object)         \
            {                                                        \
               ourPool.release(object);                              \
            }

   #define OBJECT_POOL_ALLOCATION_SUPPORT(klass,chunkSize)     \
         ObjectPool::Pool<ObjectPool::LockPolicy> klass :: ourPool(sizeof(klass), chunkSize)

   #define OBJECT_POOL_ALLOCATION_SUPPORT_COMPACT(klass)       OBJECT_POOL_ALLOCATION_SUPPORT(klass, OP_MIN_CHUNK_SIZE)
   #define OBJECT_POOL_ALLOCATION_SUPPORT_DEFAULT(klass)       OBJECT_POOL_ALLOCATION_SUPPORT(klass, OP_DEF_CHUNK_SIZE)

} /* Namespace ObjectPool */

#endif /* OBJECT_POOL_ALLOCATABLE_H_USED */
