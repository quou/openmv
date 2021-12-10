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
 - [ ] Physics.
 - [ ] Audio.
 - [x] In-game GUI.
 - [x] Debug IMGUI.
 - [ ] Save-game system.
 - [x] Room system.
 - [x] Tilemap system.
 - [ ] Advanced Rendering (Post processing fx, etc.).
 - [ ] Actually make a damn game.

## Building
Generate a Makefile using Premake. Requires a bunch of GCC extensions, because
I am bad at programming.

Dependencies on Linux:
 - `libdl`
 - `libGL`
 - `libm`
 - `libX11`

Dependencies on Windows:
 - `opengl32`
 - `gdi32`
 - `user32`
 - `kernel32`
 - `winmm`
