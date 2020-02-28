local function widget_on_init()
   RARCH_LOG("Test widget init\n")
end

local function widget_on_free()
   RARCH_LOG("Test widget free\n")
end

local function widget_on_context_reset()
   RARCH_LOG("Test widget context reset\n")
end

local function widget_on_iterate()
   RARCH_LOG("Test widget iterate\n")
end

local function widget_on_frame()
   RARCH_LOG("Test widget frame\n")
end

local function widget_on_layout()
   RARCH_LOG("Test widget layout\n")
end

local function widget_on_context_destroyed()
   RARCH_LOG("Test widget context destroyed\n")
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
