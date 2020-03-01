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

#ifndef _KRAKEN_LIB_H
#define _KRAKEN_LIB_H

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <stdbool.h>

typedef struct kraken_module
{
   const char* name;
   const char* lua_buf;
   const int lua_buf_len;
   void (*register_c_funcs)(lua_State* state);
   bool loaded;
} kraken_module_t;

#define KRAKEN_ENSURE_LOADED(module) if (!module.loaded) return;
#define KRAKEN_ENSURE_LOADED_RET(module, retval) if (!module.loaded) return retval;

// Loads the Kraken library in the given state
void kraken_load_lib(lua_State* state);

#endif
