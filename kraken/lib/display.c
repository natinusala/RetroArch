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

#include <lua.h>

#include "display.h"

#include "../../verbosity.h"

#include "../../gfx/gfx_display.h"

#include "c/display.inc.h"

/*
   display.draw_quad(
      x: integer,
      y: integer,
      width: integer,
      height: integer,
      color: integer,
      alpha: number,
      video_info: lightuserdata)
*/
static int kraken_display_draw_quad(lua_State* state)
{
   int argc = lua_gettop(state);
   if (
      argc != 7 ||
      !lua_isinteger(state, 1) ||      //x
      !lua_isinteger(state, 2) ||      //y
      !lua_isinteger(state, 3) ||      //width
      !lua_isinteger(state, 4) ||      //height
      !lua_isinteger(state, 5) ||      //color
      !lua_isnumber(state, 6) ||       //alpha
      !lua_islightuserdata(state, 7)   //video_info
   )
   {
      RARCH_ERR("[Kraken]: display.draw_quad: invalid arguments\n");
      return 0;
   }

   video_frame_info_t* video_info   = (video_frame_info_t*) lua_topointer(state, 7);
   int x                            = lua_tointeger(state, 1);
   int y                            = lua_tointeger(state, 2);
   int w                            = lua_tointeger(state, 3);
   int h                            = lua_tointeger(state, 4);
   unsigned width                   = video_info->width;
   unsigned height                  = video_info->height;

   int color                        = lua_tointeger(state, 5);
   float alpha                      = (float) lua_tonumber(state, 6);

   float color_float[16] = COLOR_HEX_TO_FLOAT(color, alpha);

   gfx_display_draw_quad(
      video_info,
      x,
      y,
      w,
      h,
      width,
      height,
      color_float
   );

   return 0;
}

void kraken_display_load(lua_State* state)
{
   lua_register(state, "display_draw_quad", kraken_display_draw_quad);

   kraken_lib_load_module(state, "display", display_lua);
}
