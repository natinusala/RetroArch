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

-- A widget is a table containing at least the following keys
local widget_keys = {
   "name",                    -- widget name
   "on_init",                 -- function called when the widget it initialized
   "on_free",                 -- function called when the widget is freed
   "on_context_reset",        -- function called when the video context gets reset
   "on_iterate",              -- function called every frame from the main thread
   "on_frame",                -- function called every frame from the video thread
   "on_layout",               -- function called to layout the widget (after context reset or after a resolution change)
   "on_context_destroyed"     -- function called when the video context gets destroyed
}

-- Widgets management
local widgets_table = {}

local function widgets_register(widget)
   -- Widget integrity check
   if widget["name"] == nil then
      retroarch.err("[Kraken]: Cannot register a widget without a name")
      return
   end

   name = widget.name
   for k,v in pairs(widget_keys) do
      if widget[v] == nil then
         retroarch.err(string.format("[Kraken]: Widget \"%s\" is missing key %s", name, v))
         return
      end
   end

   -- Register it
   table.insert(widgets_table, widget)
   retroarch.log(string.format("[Kraken]: Widget \"%s\" registered", name))
end

-- Print widget
-- Not a regular widget because we need to ensure it's
-- handled after all others
local print_strings = {}
local print_longest = ""
local print_longest_len = 0.0

local function widgets_add_print_string(str)
   table.insert(print_strings, str)

   local len = string.len(str)
   if (len > print_longest_len) then
      print_longest_len = len
      print_longest = str
   end
end

local function widgets_print_int(name, value)
   widgets_add_print_string(string.format("%s = %d", name, value))
end

local function widgets_print_hex(name, value)
   widgets_add_print_string(string.format("%s = 0x%X", name, value))
end

local function widgets_print_str(name, value)
   widgets_add_print_string(string.format("%s = %s", name, value))
end

local function print_widget_on_iterate()
   print_widget_width = print_longest_len * 18.0 -- TODO: use get_measured_width * scale here on the string instead
end

local function print_widget_on_frame(video_info)
   local y_advance = 0.0
   local padding = print_widget_padding
   local width = print_widget_width
   local height = print_widget_height

   for k,str in pairs(print_strings) do
      -- Backdrop
      display.draw_quad(
         0.0,
         y_advance,
         width + padding * 2.0,
         height + padding * 2.0,
         0x000000,
         0.75,
         video_info
      )

      -- Text
      local font, font_height = widgets.get_font_regular()
      display.cache_text(
         font,
         str,
         padding * 4.0,
         y_advance + padding + font_height,
         0xFFFFFF,
         display.text_align.LEFT,
         1.0,
         false,
         0.0,
         false,
         video_info
      )

      -- Advance
      y_advance = y_advance + height + padding * 2.0
   end

   -- everything has been drawn, empty the table
   -- and cleanup
   local count = #print_strings
   for i = 0, count do print_strings[i] = nil end
   print_longest = ""
   print_longest_len = 0.0
end

-- TODO: scale
local function print_widget_on_layout(width, height)
   print_widget_height = 50.0 -- TODO: use font height here instead
   print_widget_padding = 5.0
end

-- Global functions (called by C)
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

   print_widget_on_layout(width, height)
end

function kraken_widgets_frame(video_info)
   for k,widget in pairs(widgets_table) do
      widget.on_frame(video_info)
   end

   print_widget_on_frame(video_info)
end

function kraken_widgets_iterate()
   for k,widget in pairs(widgets_table) do
      widget.on_iterate()
   end

   print_widget_on_iterate()
end

-- Exposed widgets module
return {
   register          = widgets_register,
   get_font_regular  = widgets_get_font_regular,
   get_font_bold     = widgets_get_font_bold,
   flush_font        = widgets_flush_font,

   print_int         = widgets_print_int,
   print_hex         = widgets_print_hex,
   print_str         = widgets_print_str
}
