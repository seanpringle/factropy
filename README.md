# factropy

A hobby gamedev project heavily influenced by Factorio.

The only thing more fun than playing a base-building simulation game is making your own!

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://github.com/seanpringle/factropy/blob/master/LICENSE) [![C/C++ CI](https://github.com/seanpringle/factropy/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/seanpringle/factropy/actions/workflows/c-cpp.yml)

# tech stack

* [raylib](https://github.com/raysan5/raylib)
* [imgui](https://github.com/ocornut/imgui)
* [wren](https://github.com/wren-lang/wren)
* [c++17](https://en.wikipedia.org/wiki/C%2B%2B17)
* [openscad](http://www.openscad.org/)

![screenshot](https://github.com/seanpringle/factropy/wiki/images/factropy.png)

# building

Make sure to clone recursively for the raylib and wren submodules.

## Ubuntu 20.04

Install the [raylib dependencies](https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux) but don't build it separately.

```bash
sudo apt install libasound2-dev mesa-common-dev libx11-dev libxrandr-dev libxi-dev xorg-dev libgl1-mesa-dev libglu1-mesa-dev
```

Build factropy (requires GCC 9.x):

```bash
make
```

# running

```bash
./factropy
```

# saving

Press `F5`. Save data will be placed in `autosave` in the current directory and automatically loaded on restart. To force a new game either remove `autosave` or:

```bash
./factropy --new
```

# crashing

Probably going to happen sooner or later. A stack trace would be useful. Easy way is to run it in `gdb`:

```bash
gdb -ex run -args ./factropy
```

If it crashes collect a stack trace and drop it into a ticket:

```bash
thread apply all bt
```

Exit gdb with CTRL-D.

# antialiasing

Nothing in-game yet. Consider enabling FSAA or similar at the video driver level, such as via the Nvidia settings app.

# controls

| control | behaviour |
|---|---|
| W A S D | Normal camera movement |
| Right Click + Drag | Rotate camera |
| O | Orientate camera top-down |
| E | Toggle build menu popup |
| Q | Copy entity under mouse pointer or selection as a ghost (pipette) |
| R | Rotate ghost or constructed entity if possible |
| C | Cycle through similar ghost types (belt orienations, pipe configurations etc)
| G | Toggle the build grid |
| Escape | Stop selecting / stop pipetting / close popup window |
| Delete | Deconstruct entity under mouse or selected entities |
| Left Click on entity | Open entity popup window | 
| Left Click on ground with ghost(s) | place ghost(s) for construction | 
| Left Click + Drag on ground | Select multiple entities |
| Control + Left Click on vehicle | Select vehicle for movement |
| Control + Right Click | Direct vehicle to drive to waypoint |
| Shift + Left Click | Force ghost(s) to be placed despite collisions |
| F1 | Energy consumption stats popup |
| F2 | Vehicle waypoints and patrol popup |
| F3 | Technology licensing popup |
| F5 | Quick save game |
