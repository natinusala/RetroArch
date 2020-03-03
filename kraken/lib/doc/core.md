# libretro core module

## Module functions

### `core.is_running()`

Returns `true` if a libretro core is currently running, `false` otherwise.

### `core.read_bytes(type, address)`

Returns the bytes of the content memory at given address. Type can be one of the `core.memory` constants.

## Constants

### `core.memory.SAVE_RAM`

Save RAM memory type.

### `core.memory.RTC`

Real Time Clock memory type.

### `core.memory.SYSTEM_RAM`

Main RAM memory type.

### `core.memory.VIDEO_RAM`

Video RAM memory type.
