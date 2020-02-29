# RetroArch Kraken library

This is the Lua library to interface with RetroArch.

Each library module has two sets of functions:

- C functions called from Lua: they should NEVER be called from C code, hence why they are static to each module file
- Lua functions called from C

Run `regenerate.sh` to regenerate the C files after editing Lua code (you need xxd).

To add a module:

1. Create the Lua, C and header files for your module in the `lib` folder, add the C file to `Makefile.common`
2. In the Lua module, return the table containing the functions you want to export (`return { ... }` at the end of the file)
3. If you need to, add a `register` function in the C code in which you register all the C functions your module provides
4. Run `regenerate.sh` to generate the C header containing the Lua code
5. In `kraken_lib.c`:
    1. Include the generated header containing the Lua code as well as the header of the module
    2. Add your module to the list by providing:
        - Its name
        - The Lua code (from the generated header)
        - The function to register the C functions (created in step 3), or `NULL` if you don't have one
6. Run the code of your module where needed in RetroArch
7. Write some documentation for the new module
8. Done!
