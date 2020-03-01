# Display module

## Module functions

### `display.draw_quad(x, y, width, height, color, alpha, video_info)`

Directly draws a quad of given size and color on screen.

`color` is a RGB integer (`0xFFFFFF` being white, `0x000000` being black).

`alpha` is a float from 0.0 (invisible) to 1.0 (fully visible).

### `display.cache_text(font, text, x, y, color, alignment, scale, shadow, shadow_offset, draw_outside, video_info)`

Caches text on screen for the given font. The font can be either created and managed manually or one of the global widget fonts.

For performance reason, text is cached to a vertex array before being drawn on screen (ideally all text is cached and the font is flushed only once per frame). You must flush the font at the end of the frame otherwise you won't see anything on screen.

If you are in a widget, you can use the global widget fonts that are managed for you and flushed at the end of every frame so you don't need to do anything but cache your text for the frame.

The most common use case for manual flushing is when using scissoring - text is only scissored when flushed, and not before. That means that to scissor text, you have to:

1. flush the font to draw any cached text added before you
2. enable scissoring
3. cache your text
4. flush the font again to draw the text on screen while the scissor is active
5. disable scissoring

If you're using a global widget font you need to use `widgets.flush_font()` to flush it. Otherwise you can flush it using the `display` module.

## Constants

### `display.text_align.LEFT`

To be used with `display.cache_text()` to cache left-aligned text.

### `display.text_align.RIGHT`

To be used with `display.cache_text()` to cache right-aligned text.

### `display.text_align.CENTER`

To be used with `display.cache_text()` to cache center-aligned text.
