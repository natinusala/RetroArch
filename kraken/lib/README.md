# RetroArch Kraken library

This is the Lua library to interface with RetroArch.

Each library module has two sets of functions:
   - the ones called from Lua: they should NEVER be called from C code, hence why they are static to each module file
   - the ones called from C

Run `regenerate.sh` to regenerate the C files after editing Lua code (you need xxd).
