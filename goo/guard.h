/* ========================================================================= *
 * File: guard.h
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
 *    Special object that uses constructor and desctuctor to protect
 *    code section using provided access policy.
 *    Probably I saw somethig in Boost but idea is clean.
 *
 * History:
 *
 * 20-Jul-2006 Leonid Moiseichuk
 * - initial version created.
 * ========================================================================= */

#ifndef OBJECT_POOL_GUARD_H_USED
#define OBJECT_POOL_GUARD_H_USED

/* ========================================================================= *
 * Includes.
 * ========================================================================= */


/* ========================================================================= *
 * Interface.
 * ========================================================================= */

namespace ObjectPool
{

   template <class AccessPolicy>
   class Guard
   {
      private:
         AccessPolicy&  myAccess;

      public:

         inline Guard(AccessPolicy& access)
            : myAccess(access)
         {
            myAccess.lock();
         }

         inline ~Guard()
         {
            myAccess.unlock();
         }

      private:    /* Usage of these methods is prohibited */
         Guard(const Guard<AccessPolicy>&);
         void operator=(const Guard&);

   }; /* Class Guard */

} /* Namespace ObjectPool */

#endif /* OBJECT_POOL_GUARD_H_USED */
