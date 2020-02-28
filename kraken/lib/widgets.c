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

#include "widgets.h"

#include"../../verbosity.h"

#include "../kraken.h"
#include "kraken_lib.h"

#include "c/widgets.inc.h"

#include "../../gfx/gfx_widgets.h"

void kraken_widgets_free()
{
   lua_State* state = kraken_get_state();

   lua_getglobal(state, "kraken_widgets_free");

   if (lua_pcall(state, 0, 0, 0))
      RARCH_ERR("[Kraken]: Error while calling kraken_widgets_free: %s\n", kraken_get_error(state));
}

void kraken_widgets_init()
{
   lua_State* state = kraken_get_state();

   lua_getglobal(state, "kraken_widgets_init");

   if (lua_pcall(state, 0, 0, 0))
      RARCH_ERR("[Kraken]: Error while calling kraken_widgets_init: %s\n", kraken_get_error(state));
}

void kraken_widgets_context_reset()
{
   lua_State* state = kraken_get_state();

   lua_getglobal(state, "kraken_widgets_context_reset");

   if (lua_pcall(state, 0, 0, 0))
      RARCH_ERR("[Kraken]: Error while calling kraken_widgets_context_reset: %s\n", kraken_get_error(state));
}

void kraken_widgets_context_destroyed()
{
   lua_State* state = kraken_get_state();

   lua_getglobal(state, "kraken_widgets_context_destroyed");

   if (lua_pcall(state, 0, 0, 0))
      RARCH_ERR("[Kraken]: Error while calling kraken_widgets_context_destroyed: %s\n", kraken_get_error(state));
}

void kraken_widgets_iterate()
{
   lua_State* state = kraken_get_state();

   lua_getglobal(state, "kraken_widgets_iterate");

   if (lua_pcall(state, 0, 0, 0))
      RARCH_ERR("[Kraken]: Error while calling kraken_widgets_iterate: %s\n", kraken_get_error(state));
}

void kraken_widgets_frame(video_frame_info_t* video_info)
{
   lua_State* state = kraken_get_state();

   lua_getglobal(state, "kraken_widgets_frame");

   lua_pushlightuserdata(state, (void*) video_info);

   if (lua_pcall(state, 1, 0, 0))
      RARCH_ERR("[Kraken]: Error while calling kraken_widgets_frame: %s\n", kraken_get_error(state));
}

void kraken_widgets_layout()
{
   lua_State* state = kraken_get_state();

   lua_getglobal(state, "kraken_widgets_layout");

   if (lua_pcall(state, 0, 0, 0))
      RARCH_ERR("[Kraken]: Error while calling kraken_widgets_layout: %s\n", kraken_get_error(state));
}

//widgets.get_font_regular(): lightuserdata
static int kraken_widgets_get_font_regular(lua_State* state)
{
   font_data_t* font = gfx_widgets_get_font_regular();
   lua_pushlightuserdata(state, font);
   return 1;
}

//widgets.get_font_bold(): lightuserdata
static int kraken_widgets_get_font_bold(lua_State* state)
{
   font_data_t* font = gfx_widgets_get_font_bold();
   lua_pushlightuserdata(state, font);
   return 1;
}

//widgets.flush_font(font: lightuserdata, video_info: lightuserdata)
static int kraken_widgets_flush_font(lua_State* state)
{
   int argc = lua_gettop(state);
   if (
      argc != 2 ||
      !lua_islightuserdata(state, 1) ||   //font
      !lua_islightuserdata(state, 2)      //video_info
   )
   {
      RARCH_ERR("[Kraken]: widgets.flush_font: invalid arguments\n");
      return 0;
   }

   font_data_t* font                = (font_data_t*) lua_topointer(state, 1);
   video_frame_info_t* video_info   = (video_frame_info_t*) lua_topointer(state, 2);

   gfx_widgets_font_flush(font, video_info);

   return 0;
}

void kraken_widgets_load(lua_State* state)
{
   lua_register(state, "widgets_get_font_regular", kraken_widgets_get_font_regular);
   lua_register(state, "widgets_get_font_bold", kraken_widgets_get_font_bold);
   lua_register(state, "widgets_flush_font", kraken_widgets_flush_font);

   kraken_lib_load_module(state, "widgets", widgets_lua);
}
