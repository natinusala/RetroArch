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

#include "../../verbosity.h"

//core.is_running(): boolean
static int kraken_core_is_running(lua_State* state)
{
   lua_pushboolean(state, rarch_ctl(RARCH_CTL_CORE_IS_RUNNING, NULL));
   return 1;
}

//core.read_byte(type: integer, address: integer): integer
static int kraken_core_read_byte(lua_State* state)
{
   int argc = lua_gettop(state);
   if (argc != 2 || !lua_isinteger(state, 1) || !lua_isinteger(state, 2))
   {
      RARCH_ERR("[Kraken]: core.read_byte: invalid arguments\n");

      lua_pushnil(state);
      return 1;
   }

   /* Ensure core is running */
   bool running = rarch_ctl(RARCH_CTL_CORE_IS_RUNNING, NULL);

   if (!running)
   {
      RARCH_ERR("[Kraken]: core.read_byte: core is not running, cannot read memory\n");
      lua_pushnil(state);
      return 1;
   }

   /* Get memory */
   unsigned type     = lua_tointeger(state, 1);
   unsigned address  = lua_tointeger(state, 2);

   retro_ctx_memory_info_t meminfo;
   meminfo.id = type;

   if (!core_get_memory(&meminfo) || meminfo.size == 0 || !meminfo.data)
   {
      RARCH_ERR("[Kraken]: core.read_byte: cannot read memory\n");
      lua_pushnil(state);
      return 1;
   }

   if (address >= meminfo.size)
   {
      RARCH_ERR("[Kraken]: core.read_byte: address %d is higher than memory size %d\n", address, meminfo.size);
      lua_pushnil(state);
      return 1;
   }

   uint8_t* bytes = (uint8_t*) meminfo.data;
   lua_pushinteger(state, (int) bytes[address]);
   return 1;
}

static void kraken_core_register(lua_State* state)
{
   lua_register(state, "core_is_running", kraken_core_is_running);
   lua_register(state, "core_read_byte", kraken_core_read_byte);
}

kraken_module_t kraken_module_core = {
   "core",
   core_lua,
   sizeof(core_lua),
   kraken_core_register
};
