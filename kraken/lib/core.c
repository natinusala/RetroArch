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

#include "core.h"

#include "c/core.inc.h"

#include "../../retroarch.h"

//core.is_running()
static int kraken_core_is_running(lua_State* state)
{
   lua_pushboolean(state, rarch_ctl(RARCH_CTL_CORE_IS_RUNNING, NULL));
   return 1;
}

static void kraken_core_register(lua_State* state)
{
   lua_register(state, "core_is_running", kraken_core_is_running);
}

kraken_module_t kraken_module_core = {
   "core",
   core_lua,
   sizeof(core_lua),
   kraken_core_register
};
