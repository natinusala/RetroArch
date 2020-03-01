/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2020 - natinusala
 *  Copyright (C) 2020 - Andre Leiradella
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

#include "kraken_lib.h"

#include "../../verbosity.h"

#include "lib/retroarch.h"
#include "lib/widgets.h"
#include "lib/display.h"

static kraken_module_t* modules[] = {
   &kraken_module_retroarch,
   &kraken_module_widgets,
   &kraken_module_display
};

static size_t modules_len = sizeof(modules) / sizeof(modules[0]);

static int kraken_searcher(lua_State* state)
{
   size_t i;
   const char* name = lua_tostring(state, 1);

   for (i = 0; i < modules_len; i++)
   {
      kraken_module_t* module = modules[i];
      if (strcmp(name, module->name) == 0)
      {
         /* Register the C functions if needed */
         if (module->register_c_funcs)
            module->register_c_funcs(state);

         /* Load the Lua code */
         int res = luaL_loadbufferx(state, module->lua_buf, module->lua_buf_len, name, "t");

         if (res != LUA_OK)
         {
            RARCH_ERR("[Kraken]: Unable to load module \"%s\"\n", kraken_get_error(state));
            return lua_error(state);
         }
         else
         {
            /* Mark the module as loaded */
            module->loaded = true;
            return 1;
         }
      }
   }

   RARCH_ERR("[Kraken]: Unknown module \"%s\"\n", name);
   lua_pushnil(state);
   return 1;
}

void kraken_load_lib(lua_State* state)
{
   /* Mark all modules as unloaded */
   for (size_t i = 0; i < modules_len; i++)
      modules[i]->loaded = false;

   /* Load the searcher */
   lua_getglobal(state, "package");
   lua_getfield(state, -1, "searchers");

   const size_t length = lua_rawlen(state, -1);

   lua_pushcfunction(state, kraken_searcher);
   lua_rawseti(state, -2, length + 1);

   lua_pop(state, 2);
}
