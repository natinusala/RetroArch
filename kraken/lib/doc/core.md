# libretro core module

## Module functions

### `core.is_running()`

Returns `true` if a libretro core is currently running, `false` otherwise.

### `core.read_bytes(type, ...)`

Returns the byte of the content memory at given addresses. Type can be one of the `core.memory` constants.

Example: `pose, game_mode = core.read_bytes(core.memory.SYSTEM_RAM, 0x13E0, 0x0100)`

## Constants

### `core.memory.SAVE_RAM`

Save RAM memory type.

### `core.memory.RTC`

Real Time Clock memory type.

### `core.memory.SYSTEM_RAM`

Main RAM memory type.

### `core.memory.VIDEO_RAM`

Video RAM memory type.
