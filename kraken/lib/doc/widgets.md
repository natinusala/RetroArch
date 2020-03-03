# Widgets module

## Functions

### `widgets.register(widget)`

Registers and enables your widget.

A widget is a table containing at least the following functions:

- `on_init()`: called when the widget it initialized
- `on_free()`: called when the widget is freed
- `on_context_reset()`: called when the video context gets reset
- `on_iterate()`: called every frame from the main thread
- `on_frame(video_info)`: called every frame from the video thread
- `on_layout(width, height)`: called to layout the widget (after context reset or after a resolution change)
- `on_context_destroyed()`: called when the video context gets destroyed

You may only use display functions from the `on_frame()` function as it is running on the video thread. All widget functions are synchronized and cannot run simultaneously.

### `widgets.get_font_regular()`

Returns the global regular font used by all widgets. To be used with `display.cache_font`.

### `widgets.get_font_bold()`

Returns the global bold font used by all widgets. To be used with `display.cache_font`.

### `widgets_flush_font(font, video_info)`

Draws all cached text for that font on screen and flushes the cache. See the `display` module documentation for more information on text caching.
