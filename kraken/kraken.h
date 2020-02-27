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

#ifndef _LUA_H
#define _LUA_H

#include <lua.h>

#include <boolean.h>

// Must be called from the main thread
bool kraken_init(void);
void kraken_deinit(void);

// Registers the RetroArch API C functions to the given Lua state
void kraken_register_functions(lua_State *state);

// Gets the state to use when calling Lua functions from C
// (depends on the current thread)
lua_State* kraken_get_state(void);

#endif
