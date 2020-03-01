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

return {
   -- Functions
   create_subject = animations_create_subject,
   push           = animations_push,

   -- Constants
   easing = {
      -- Linear
      LINEAR        = 0,
      -- Quad
      IN_QUAD       = 1,
      OUT_QUAD      = 2,
      IN_OUT_QUAD   = 3,
      OUT_IN_QUAD   = 4,
      -- Cubic
      IN_CUBIC      = 5,
      OUT_CUBIC     = 6,
      IN_OUT_CUBIC  = 7,
      OUT_IN_CUBIC  = 8,
      -- Quart
      IN_QUART      = 9,
      OUT_QUART     = 10,
      IN_OUT_QUART  = 11,
      OUT_IN_QUART  = 12,
      -- Quint
      IN_QUINT      = 13,
      OUT_QUINT     = 14,
      IN_OUT_QUINT  = 15,
      OUT_IN_QUINT  = 16,
      -- Sine
      IN_SINE       = 17,
      OUT_SINE      = 18,
      IN_OUT_SINE   = 19,
      OUT_IN_SINE   = 20,
      -- Expo
      IN_EXPO       = 21,
      OUT_EXPO      = 22,
      IN_OUT_EXPO   = 23,
      OUT_IN_EXPO   = 24,
      -- Circ
      IN_CIRC       = 25,
      OUT_CIRC      = 26,
      IN_OUT_CIRC   = 27,
      OUT_IN_CIRC   = 28,
      -- Bounce
      IN_BOUNCE     = 29,
      OUT_BOUNCE    = 30,
      IN_OUT_BOUNCE = 31,
      OUT_IN_BOUNCE = 32
   }
}
