-- Display module

--[[  RetroArch - A frontend for libretro.
      Copyright (C) 2020 - natinusala

      RetroArch is free software: you can redistribute it and/or modify it under the terms
      of the GNU General Public License as published by the Free Software Found-
      ation, either version 3 of the License, or (at your option) any later version.

      RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
      without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
      PURPOSE.  See the GNU General Public License for more details.

      You should have received a copy of the GNU General Public License along with RetroArch.
      If not, see <http://www.gnu.org/licenses/>.
--]]

-- Exposed display module
display = {
   -- Functions
   draw_quad   = display_draw_quad,
   cache_text  = display_cache_text,

   -- Constants
   TEXT_ALIGN_LEFT   = 0,
   TEXT_ALIGN_RIGHT  = 1,
   TEXT_ALIGN_CENTER = 2
}
