widgets = require("widgets")
display = require("display")
retroarch = require("retroarch")
animations = require("animations")
core = require("core")

local function widget_on_init()
   retroarch.log("Test widget init")
end

local function widget_on_free()
   retroarch.log("Test widget free")
end

local function widget_on_context_reset()
   retroarch.log("Test widget context reset")
end

local function widget_on_iterate()
   is_running = core.is_running()
   is_menu_open = retroarch.is_menu_open()

   -- don't do anything if core is not running or menu is open
   if ((not is_running) or (is_menu_open)) then
      return
   end

   -- read game memory
   -- 0x7E13E0: player pose
   -- 0x7E0100: game mode
   -- 0x7E bank is mapped to beginning of RAM so translation is just remove 7E
   pose, game_mode = core.read_bytes(core.memory.SYSTEM_RAM, 0x13E0, 0x0100)
   if (pose == 0x3E) then -- death pose
      retroarch.log("you lost")
      retroarch.shutdown()
   end
end

local function widget_on_frame(video_info)
end

local function widget_on_layout(width, height)
   retroarch.log(string.format("Test widget layout w/ resolution %d x %d", width, height))
end

local function widget_on_context_destroyed()
   retroarch.log("Test widget context destroyed")
end

local widget = {
   name                 = "test",
   on_init              = widget_on_init,
   on_free              = widget_on_free,
   on_context_reset     = widget_on_context_reset,
   on_iterate           = widget_on_iterate,
   on_frame             = widget_on_frame,
   on_layout            = widget_on_layout,
   on_context_destroyed = widget_on_context_destroyed
}

widgets.register(widget)
