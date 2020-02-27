/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2020 - natinusala
 *
 *  RetroArch is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RetroArch.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include <lua.h>
#include "lichen_lua_user.h"

#include <stdlib.h>

#ifdef HAVE_THREADS
#include "../verbosity.h"
#include <rthreads/rthreads.h>

static slock_t* gil;

void lichen_init_lock(void)
{
   gil = slock_new();

   if (!gil)
      RARCH_ERR("[Lichen]: Unable to create GIL!");
}

void lichen_free_lock(void)
{
   slock_free(gil);
}

void lichen_lock(lua_State* state)
{
   slock_lock(gil);
}

void lichen_unlock(lua_State* state)
{
   slock_unlock(gil);
}
#endif
