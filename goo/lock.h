/* ========================================================================= *
 * File: lock.h
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
 *    Minimalistic wraper for standard mutex implementation to support lock-policy.
 *    POSIX and Windows versions are supported.
 *
 * History:
 *
 * 18-Jul-2006 Leonid Moiseichuk
 * - initial version created.
 * ========================================================================= */

#ifndef OBJECT_POOL_LOCK_H_USED
#define OBJECT_POOL_LOCK_H_USED

/* ========================================================================= *
 * Includes.
 * ========================================================================= */

#include "defines.h"

#ifdef OP_WINDOWS
#include <windows.h>
#else
#include <pthread.h>
#endif


/* ========================================================================= *
 * Interface.
 * ========================================================================= */

namespace ObjectPool
{

   class LockPolicy
   {
      /* No idea how to make it smarter but now not necessary to make */
      /* two copies for Windows and POSIX separately, so use defines. */

      private:
#ifdef OP_WINDOWS
         CRITICAL_SECTION  myLock;
#else
         pthread_mutex_t   myLock;
#endif

      public:

         inline LockPolicy()
         {
#ifdef OP_WINDOWS
            InitializeCriticalSection(&myLock);
#else
            pthread_mutex_init(&myLock, 0);
#endif
         }

         inline ~LockPolicy()
         {
#ifdef OP_WINDOWS
            DeleteCriticalSection(&myLock);
#else
            pthread_mutex_destroy(&myLock);
#endif
         }

         inline void lock()
         {
#ifdef OP_WINDOWS
            EnterCriticalSection(&myLock);
#else
            pthread_mutex_lock(&myLock);
#endif
         }

         inline void unlock()
         {
#ifdef OP_WINDOWS
            LeaveCriticalSection(&myLock);
#else
            pthread_mutex_unlock(&myLock);
#endif
         }

      private:    /* Usage of these methods is prohibited */
         LockPolicy(const LockPolicy&);
         void operator=(const LockPolicy&);

   }; /* Class LockPolicy */

} /* Namespace ObjectPool */

#endif /* OBJECT_POOL_LOCK_H_USED */
