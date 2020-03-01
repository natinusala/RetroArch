# RetroArch Kraken library

This is the Lua library to interface with RetroArch. Documentation of each individual module is available in the `doc` folder.

Each library module has two sets of functions:

- C functions called from Lua: they should NEVER be called from C code, hence why they are static to each module file
- Lua functions called from C

Run `regenerate.sh` to regenerate the C files after editing Lua code (you need xxd).

To add a module:

1. Create the Lua, C and header files for your module in the `lib` folder, add the C file to `Makefile.common`
2. In the Lua module, return the table containing the functions you want to export (`return { ... }` at the end of the file)
3. In the C code, add a `kraken_module_t` for the module (as you would for a classic RetroArch driver implementation)
    - The `register_c_funcs` function is where you need to call `lua_register`
4. Run `regenerate.sh` to generate the C header containing the Lua code
5. In `kraken_lib.c`, add your module to the list
6. Run the code of your module where needed in RetroArch
7. Write some documentation for the new module
8. Done!
