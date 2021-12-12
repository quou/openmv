# OpenMV
An open source metroidvania.

Created from the ground up in C99.

I'm creating this project to build the foundations for many games to come.
It is designed as an engine that *I* want to use - which doesn't necessarily
make it a good engine. It is designed with non-linear platform games in mind.

## Features
 - [x] Basic Rendering.
 - [x] Resource management.
 - [x] Code hot-reloading for game logic.
 - [x] Windows support.
 - [x] Physics.
 - [ ] Audio.
 - [x] In-game GUI.
 - [x] Debug IMGUI.
 - [x] Save-game system.
 - [x] Room system.
 - [x] Resource Packing.
 - [x] Tilemap system.
 - [ ] Advanced Rendering (Post processing fx, etc.).
 - [ ] Actually make a damn game.

## Building
Generate a Makefile using Premake. Requires a bunch of GCC extensions, because
I am bad at programming; Just use GCC.

Before you can run the game in release mode, build the `packer` project and run
it from the project root to create a file named `res.pck` which contains all of
the game's resources. The game reads from this file in release mode so that
shipping the entire resource folder isn't required.

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

## Levels
Levels are created using the Tiled level editor and exported using a custom binary
format. An extension for Tiled to add this format can be found in
`tiledext/mapformat.js`.
