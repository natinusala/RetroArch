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
#include "../kraken_lib.h"

#include <lua.h>

#include "display.h"

#include "../../verbosity.h"

#include "../../gfx/gfx_display.h"

#include "c/display.inc.h"

/*
   display.draw_quad(
      x: number,
      y: number,
      width: number,
      height: number,
      color: integer,
      alpha: number,
      video_info: lightuserdata)
*/
static int kraken_display_draw_quad(lua_State* state)
{
   int argc = lua_gettop(state);
   if (
      argc != 7 ||
      !lua_isnumber(state, 1) ||       //x
      !lua_isnumber(state, 2) ||       //y
      !lua_isnumber(state, 3) ||       //width
      !lua_isnumber(state, 4) ||       //height
      !lua_isinteger(state, 5) ||      //color
      !lua_isnumber(state, 6) ||       //alpha
      !lua_islightuserdata(state, 7)   //video_info
   )
   {
      RARCH_ERR("[Kraken]: display.draw_quad: invalid arguments\n");
      return 0;
   }

   video_frame_info_t* video_info   = (video_frame_info_t*) lua_topointer(state, 7);
   int x                            = round(lua_tonumber(state, 1));
   int y                            = round(lua_tonumber(state, 2));
   int w                            = round(lua_tonumber(state, 3));
   int h                            = round(lua_tonumber(state, 4));
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

/*
   display.cache_text(
      font: lightuserdata,
      text: string,
      x : number,
      y : number,
      color: integer,
      alignment: integer,
      scale: number,
      shadow: boolean,
      shadow_offset: number,
      draw_outside: boolean,
      video_info: lightuserdata
   )
*/
static int kraken_display_cache_text(lua_State* state)
{
   int argc = lua_gettop(state);
   if (
      argc != 11 ||
      !lua_islightuserdata(state, 1) ||   //font
      !lua_isstring(state, 2) ||          //text
      !lua_isnumber(state, 3) ||          //x
      !lua_isnumber(state, 4) ||          //y
      !lua_isinteger(state, 5) ||         //color
      !lua_isinteger(state, 6) ||         //alignment
      !lua_isnumber(state, 7) ||          //scale
      !lua_isboolean(state, 8) ||         //shadow
      !lua_isnumber(state, 9) ||          //shadow_offset
      !lua_isboolean(state, 10) ||        //draw_outside
      !lua_islightuserdata(state, 11)     //video_info
   )
   {
      RARCH_ERR("[Kraken]: display.cache_text: invalid arguments\n");
      return 0;
   }

   font_data_t* font                = (font_data_t*) lua_topointer(state, 1);
   video_frame_info_t* video_info   = (video_frame_info_t*) lua_topointer(state, 11);
   const char* text                 = lua_tostring(state, 2);
   float x                          = (float) lua_tonumber(state, 3);
   float y                          = (float) lua_tonumber(state, 4);
   int width                        = video_info->width;
   int height                       = video_info->height;
   uint32_t color                   = lua_tointeger(state, 5);
   enum text_alignment text_align   = (enum text_alignment) lua_tointeger(state, 6);
   float scale                      = (float) lua_tonumber(state, 7);
   bool shadows_enable              = lua_toboolean(state, 8);
   float shadow_offset              = (float) lua_tonumber(state, 9);
   bool draw_outside                = lua_toboolean(state, 10);

   gfx_display_draw_text(
      font,
      text,
      x,
      y,
      width,
      height,
      color,
      text_align,
      scale,
      shadows_enable,
      shadow_offset,
      draw_outside
   );

   return 0;
}

static void kraken_display_register(lua_State* state)
{
   lua_register(state, "display_draw_quad", kraken_display_draw_quad);
   lua_register(state, "display_cache_text", kraken_display_cache_text);
}

kraken_module_t kraken_module_display = {
   "display",
   display_lua,
   sizeof(display_lua),
   kraken_display_register
};
