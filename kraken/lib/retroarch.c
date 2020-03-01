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

#include "retroarch.h"

#include "../verbosity.h"

#include "c/retroarch.inc.h"

#include <stdlib.h>

#ifdef HAVE_THREADS
#include <rthreads/rthreads.h>
#endif

//retroarch.err(text: string)
static int kraken_retroarch_err(lua_State* state)
{
   int argc = lua_gettop(state);
   if (argc != 1 || !lua_isstring(state, 1))
   {
      RARCH_ERR("[Kraken]: RARCH_ERR: invalid arguments\n");
      return 0;
   }

   const char* text = lua_tostring(state, 1);
   RARCH_ERR("%s\n", text);

   return 0;
}

//retroarch.log(text: string)
static int kraken_retroarch_log(lua_State *state)
{
   int argc = lua_gettop(state);
   if (argc != 1 || !lua_isstring(state, 1))
   {
      RARCH_ERR("[Kraken]: RARCH_LOG: invalid arguments\n");
      return 0;
   }

   const char* text = lua_tostring(state, 1);
   RARCH_LOG("%s\n", text);

   return 0;
}

static void kraken_retroarch_register(lua_State *state)
{
   lua_register(state, "retroarch_err", kraken_retroarch_err);
   lua_register(state, "retroarch_log", kraken_retroarch_log);
}

kraken_module_t kraken_module_retroarch = {
   "retroarch",
   retroarch_lua,
   sizeof(retroarch_lua),
   kraken_retroarch_register
};
