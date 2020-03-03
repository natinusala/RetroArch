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

retroarch = require("retroarch")

-- A widget is a table containing at least the following functions
local widgets_funcs = {
   "on_init",                 -- called when the widget it initialized
   "on_free",                 -- called when the widget is freed
   "on_context_reset",        -- called when the video context gets reset
   "on_iterate",              -- called every frame from the main thread
   "on_frame",                -- called every frame from the video thread
   "on_layout",               -- called to layout the widget (after context reset or after a resolution change)
   "on_context_destroyed"     -- called when the video context gets destroyed
}

-- Widgets management functions
local widgets_table = {}

local function widgets_register(name, widget)
   -- Widget integrity check
   for k,func in pairs(widgets_funcs) do
      if widget[func] == nil then
         retroarch.err(string.format("[Widgets]: Widget %s is missing function %s", name, func))
         return
      end
   end

   -- Register it
   table.insert(widgets_table, widget)
   retroarch.log(string.format("[Widgets]: %s widget registered", name))
end

function kraken_widgets_init()
   for k,widget in pairs(widgets_table) do
      widget.on_init()
   end
end

function kraken_widgets_free()
   for k,widget in pairs(widgets_table) do
      widget.on_free()
   end
end

function kraken_widgets_context_reset()
   for k,widget in pairs(widgets_table) do
      widget.on_context_reset()
   end
end

function kraken_widgets_context_destroyed()
   for k,widget in pairs(widgets_table) do
      widget.on_context_destroyed()
   end
end

function kraken_widgets_layout(width, height)
   for k,widget in pairs(widgets_table) do
      widget.on_layout(width, height)
   end
end

function kraken_widgets_frame(video_info)
   for k,widget in pairs(widgets_table) do
      widget.on_frame(video_info)
   end
end

function kraken_widgets_iterate()
   for k,widget in pairs(widgets_table) do
      widget.on_iterate()
   end
end

-- Exposed widgets module
return {
   register          = widgets_register,
   get_font_regular  = widgets_get_font_regular,
   get_font_bold     = widgets_get_font_bold,
   flush_font        = widgets_flush_font
}
