/* ========================================================================= *
 * File: nolock.h
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
 *    Empty implementation of locking object to provide no-lock-policy.
 *    Usage of this object is necessary when no multithread support is required.
 *
 * History:
 *
 * 18-Jul-2006 Leonid Moiseichuk
 * - initial version created.
 * ========================================================================= */

#ifndef OBJECT_POOL_NOLOCK_H_USED
#define OBJECT_POOL_NOLOCK_H_USED

/* ========================================================================= *
 * Includes.
 * ========================================================================= */

#include "defines.h"

/* ========================================================================= *
 * Interface.
 * ========================================================================= */

namespace ObjectPool
{

   class NoLockPolicy
   {
      public:

         inline NoLockPolicy()
         {}

         inline ~NoLockPolicy()
         {}

         inline void lock()
         {}

         inline void unlock()
         {}

      private:    /* Usage of these methods is prohibited */
         NoLockPolicy(const NoLockPolicy&);
         void operator=(const NoLockPolicy&);

   }; /* Class NoLockPolicy */

} /* Namespace ObjectPool */

#endif /* OBJECT_POOL_NOLOCK_H_USED */
