widgets = require("widgets")
display = require("display")
retroarch = require("retroarch")
animations = require("animations")

xpos = 50.0

function test_widget_animation_finished()
   retroarch.log("Animation finished :spook:")
end

local function widget_on_init()
   retroarch.log("Test widget init")

   animations.push("xpos", 500.0, 5000, animations.easing.OUT_BOUNCE, "test_widget_animation_finished")
end

local function widget_on_free()
   retroarch.log("Test widget free")
end

local function widget_on_context_reset()
   retroarch.log("Test widget context reset")
end

local function widget_on_iterate()
end

local function widget_on_frame(video_info)
   display.draw_quad(xpos, 50.0, 200.0, 200.0, 0x32cd32, 0.75, video_info)
   font = widgets.get_font_regular()

   display.cache_text(
      font,
      "Hello from Lua",
      150,
      150,
      0xFFFFFF,
      display.text_align.LEFT,
      1,
      true,
      5,
      false,
      video_info
   )
end

local function widget_on_layout()
   retroarch.log("Test widget layout")
end

local function widget_on_context_destroyed()
   retroarch.log("Test widget context destroyed")
end

local widget = {
   on_init              = widget_on_init,
   on_free              = widget_on_free,
   on_context_reset     = widget_on_context_reset,
   on_iterate           = widget_on_iterate,
   on_frame             = widget_on_frame,
   on_layout            = widget_on_layout,
   on_context_destroyed = widget_on_context_destroyed
}

widgets.register("test", widget)
