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

#include "kraken.h"

#include "../verbosity.h"

#include <stdlib.h>

#ifdef HAVE_THREADS
#include <rthreads/rthreads.h>
#endif

static int kraken_rarch_log(lua_State *state)
{
   int argc = lua_gettop(state);
   if (argc != 1 || !lua_isstring(state, 1))
   {
      RARCH_ERR("[Kraken]: kraken_rarch_log: invalid arguments\n");
      return 0;
   }

   const char* text = lua_tostring(state, 1);
   RARCH_LOG("%s", text);

   return 0;
}

void kraken_register_functions(lua_State *state)
{
   lua_register(state, "RARCH_LOG", kraken_rarch_log);
}
