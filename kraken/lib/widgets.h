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

#ifndef _KRAKEN_LIB_WIDGETS_H
#define _KRAKEN_LIB_WIDGETS_H

#include <lua.h>

void kraken_widgets_load(lua_State* state);

void kraken_widgets_init();
void kraken_widgets_free();
void kraken_widgets_context_reset();
void kraken_widgets_context_destroyed();
void kraken_widgets_layout();
void kraken_widgets_iterate();
void kraken_widgets_frame();

#endif
