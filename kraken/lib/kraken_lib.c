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

#include "../kraken.h"

#include "kraken_lib.h"

#include "retroarch.h"
#include "widgets.h"

#include "../../verbosity.h"

void kraken_load_module(lua_State* state, const char* name, char* data)
{
   if (luaL_dostring(state, data))
      RARCH_ERR("[Kraken]: Unable to load library module %s: %s\n", name, kraken_get_error(state));
}

void kraken_load_lib(lua_State* state)
{
   kraken_retroarch_load(state);
   kraken_widgets_load(state);
}
