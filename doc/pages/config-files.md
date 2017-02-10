Configuration files
===================

Configure the engine's behavior
-------------------------------------------

The engine uses a header file called config.h were different aspects of its behavior are defined.

Work RAM usage, DRAM usage and management, debugging tools, etc., are configured according to the macros defined in the file.

The engine provides its own config.h file, but each game should provide its own to override the engine's default behavoir.

For a description of the configuration parameters check the config.h file.

Compilation options
-------------------------------------------

The engine supports various options for its compilation. Among these are:

- The compilation type:
	- Debug: adds lots of runtime assertions. Enables debugging tools too. It is recommended to use the sram sections for the memory pools when debugging in order to avoid stack overflows (SRAM as WRAM only works on emulators).
	- Release: Removes most asserts. For shipping only!
    - Tools: Adds debugging tools without all the debug checking.
    - Preprocessor: The .o files are preprocessor's output instead of compiler's.
- Optimization option passed to the compiler
- Warning option passed to the compiler
- Control over the usage of the frame pointer in the output code
- Control over the usage of prolog functions in the output code
- Control over the usage of the program's sections:
	- SRAM can be use as WRAM. It adds, theoretically, 16MB of WRAM where all non initialized variables can be allocated. This feature is experimental and only works properly on emulators. Since only 8KB of SRAM is available on real carts, more than that will only work on emulators.
	- DRAM can be used as WRAM too, you must edit the linker script vb.ld to accommodate this taking into account that the Param Table's last address normally is 0x0003D800, where the WORLD attributes start.
	- Allocation section for some of the game's global variables (data, sdata, bss, sbss, etc.)

> To make effective any change to these options, the whole project needs to be recompiled.
