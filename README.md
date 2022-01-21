# OpenMV

![screenshot](https://raw.githubusercontent.com/veridisquot/openmv/master/media/001.png)

An open source Metroidvania.

Created from the ground up in C99.

## Features
 - [x] Basic Rendering.
 - [x] Resource management.
 - [x] Code hot-reloading for game logic.
 - [x] Windows support.
 - [x] Physics.
 - [x] Audio.
 - [x] In-game GUI.
 - [x] Debug IMGUI.
 - [x] Save-game system.
 - [x] Room system.
 - [x] Resource Packing.
 - [x] Tilemap system.
 - [x] Dialogue system for NPCs.
 - [ ] Command system for allow for easy configuration.
 - [ ] Advanced Rendering (Post processing fx, etc.).
 - [ ] Actually make a damn game.

## Building
Generate a Makefile using Premake and use that to compile everything.

Before you can run the game in release mode, build the `packer` project and run
it from the project root to create a file named `res.pck` which contains all of
the game's resources. The game reads from this file in release mode so that
shipping the entire resource folder isn't required. The packer packs all of
the files listed in `packed.include`; This file is sometimes out of date,
so if the game struggles to load in release mode, try debug mode instead.

Dependencies on Linux:
 - `libdl`
 - `libGL`
 - `libm`
 - `libX11`
 - `libpthread`

Dependencies on Windows:
 - `opengl32`
 - `gdi32`
 - `user32`
 - `kernel32`
 - `winmm`

I develop everything under 64-bit GNU/Linux, with Clang and glibc. There's no
reason why it wouldn't work under a 32-bit system with GCC, though (The Premake
scripts will need to be modified to build under 32-bit). Windows *should* work,
but I rarely test it, so some minor modifications might be needed here and there.

## Creating New Projects
OpenMV's core can be used for new projects, including all the code for things
that games commonly need, such as loading resources and drawing graphics. A
utility in `util/mksdk` exists so that manually copying files isn't required
in order to create a new project. Simply build this project and run it from
the project root to create a file named `sdk.tar`. Extract this file and
start creating. Note that this utility won't work on Windows, at least
not without something that makes it POSIX-compliant, such as Cygwin.

The default project creates a blank window and spams the console every
frame. Edit the code in `logic/src/main.c` to create your game logic. All the
code compiled in this project is hot-reloadable, meaning you can re-compile
it and have the code update in real time without needing to restart the program.
The default project also creates a loading screen while the `on_init` function
in the logic project is running. Comment out lines 32-65 in
`bootstrapper/src/main.c` to remove this if you don't want it.

## Levels
Levels are created using the Tiled level editor and exported using a custom binary
format. An extension for Tiled to add this format can be found in
`tiledext/mapformat.js`.

## Stuff by Other People
Below is a list of pieces of code that I didn't write:
 - [glad](https://github.com/Dav1dde/glad).
 - [microtar](https://github.com/rxi/microtar).
 - [miniaudio](https://miniaud.io/index.html).
 - [stb_rect_pack](https://github.com/nothings/stb/blob/master/stb_rect_pack.h).
 - [stb_truetype](https://github.com/nothings/stb/blob/master/stb_truetype.h).

Everything else was written by me, from scratch.

## Contributing
If you have plans to contribute code to this repository: First of all, thank-you
for your time! Second, please follow the general style of the existing code as
much as possible. This includes: Using tabs for indentation (not spaces), using
`double` instead of `float` for timing and not `typedef`'ing structures or
enumerations. Everything should adhere to standard C99 where possible or at
the very least compile with Clang (minimise the use of GNU extensions). External
libraries should also be avoided, though single-header utilities are fine.
