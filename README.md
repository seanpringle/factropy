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
