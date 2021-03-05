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
| Scroll wheel | Zoom camera |
| E | Toggle build menu popup |
| Q | Copy entity under mouse pointer or selection as a ghost (pipette) |
| R | Rotate ghost or constructed entity if possible |
| C | Cycle through similar ghost types (belt orienations, pipe configurations etc)
| G | Toggle the build grid |
| Escape | Stop selecting / stop pipetting / close popup window |
| Delete | Deconstruct entity under mouse or selected entities |
| Left Click on entity | Open entity popup window |
| Left Click on ground with ghost(s) | Place ghost(s) for construction |
| Left Click + Drag on ground | Select multiple entities |
| Control + Left Click on vehicle | Select vehicle for movement |
| Control + Right Click | Direct vehicle to drive to waypoint |
| Shift + Left Click | Force ghost(s) to be placed despite collisions |
| F1 | Energy consumption stats popup |
| F2 | Vehicle waypoints and patrol popup |
| F3 | Technology licensing popup |
| F5 | Quick save game |
| F9 | Move the secondary camera to the main camera's current view |

# interesting questions

## Fac-what-now?

Entropy, in the colloquial sense, the _measure of disorder in a system_. Fighting with entropy seems to describe my Factorio experience rather well. Factropy.

Or if you hate that etymology maybe it's just that Factorio is such a successful genre-defining game that implementing anything remotely like it will be considered nothing but a [collection of tropes](https://tvtropes.org/pmwiki/pmwiki.php/Videogame/Factorio). Factropy.

## Hasn't Factorio in 3D been done?

IMHO Factorio's top-down 2D view is more comfortable than Satisfactory's first-person or the Dyson Sphere Program's third-person view, and the Factorio build grid is simpler to use than other approaches too. So Factropy is still largely about building a base that spreads over flat ground on a fixed grid. The use of 3D is in specific areas where it seems worthwhile:

* **Hills**. Instead of ore patches we have hills and mountains which contain minable resources. Instead of laying out a grid of Factorio _mining drills_ and belts to cover every bit of an ore patch, which gets boring, a few well-placed Factropy _miners_ can eventually tunnel throughout their local hill and extract all available resources. Hills also get in the way of base expansion in a non-arbitrary way (looking at you, Factorio cliffs!) requiring careful thought to build around or ultimately flatten out.

* **Lakes**. Technically Factropy lakes have underwater topography too. Nothing done with it so far but some vague ideas about oil rigs and underwater resource extraction; dredging, drilling etc.

* **Plains**. Deliberately not 3D and all suspiciously level and easy to build on. You see all the imaginary silt is washed down by the imaginary rain from the hills and...

* **Entity heights**. Collision boxes are 3D, and a furnace is obviously taller than an arm (inserter) which must be able to extend over a conveyor (belt). Grouping and layout of Factropy entities must make sense in three dimensions, and introduces limitations and challenges around access and line of sight that Factorio can ignore.

* **Ropeways**. Like these https://en.wikipedia.org/wiki/Material_ropeway . They're a long-distance logistics alternative to trains for transporting ore from remote outposts, that can be built over hills and across water depending on cable span. The buckets travel at around 10 metres up in the air, well above other entities.

* **Drone flight paths**. The equivalent of construction/logistic robots, these guys have slightly more realistic flight paths to contend with 3D terrain and entities, and as a result have certain limitations that would be seen as nerfing Factorio's robots.

* **Combat**. Gun turrets can fire in any direction including upward so some sort of aerial enemy seems logical.

Basically I'm interested in features that use 3D to enhance the basic 2D factory experience, not just saying _hey, let's go 3D so we can build vertically and frustrate ourselves trying to control the camera!_

## Is 3D rendering practical at Factorio's scale?

A few times Factorio devs have said that 3D wouldn't be practical. Eg from Kovarex: ...[show me a game that can render 30 000 3D objects (instead of sprites) on the same screen](https://forums.factorio.com/viewtopic.php?p=538365#p538365). Sounds like a challenge!

Anecdata: so far Factropy has rendered a base with 15000 objects on screen at 60FPS on a not-that-new machine with a not-that-new graphics card. That includes terrain meshes, entities with moving parts, items on belts, ore on the ground, particle emitters for smoke, lights etc. 30000 or more doesn't seem unrealistic.

Factropy does this by only permitting 3D models that can be fully instanced into render batches with minimal shader switching. Entities consist of one or more component meshes with associated materials: wheels, hulls, arms, gears, pistons, fans, wires, metal, wood etc. A shared library of components are composed into entities, rather than entities coming with dedicated graphical assets. All the instances of a single component on screen are one mesh rendered many times with a single draw call. Animation of components is done using OpenGL VBO arrays of transformation matrices to control mesh rotation/translation/scale, allowing one assmbler to be idling while another is running at full speed. The same approach is used for items on belts: one draw call per type of item on screen.

All this boils down to the rendering thread being largely bottlenecked on CPU doing math and updating dynamic VBO arrays each frame before the relatively few draw calls execute. There are ways to reduce the load. Some math is deferred and queued to be done by the graphics card, particularly where a component transforms deterministically. Some components use pre-computed sets of transforms so the work is done at startup rather than per-entity per-frame, effectively a lookup table in VRAM. Particle emitters are instanced too and just tweaked in their shader to appear a bit random.

However, these are the hurdles that might yet bring it all crashing down in a smoking heap of burnt-out GPUs:

* No shadow mapping is done yet. _Not too scary_ based on some test runs though, and works fine with instancing
* Only a small number of dynamic lights are supported. _Somewhat scary_ as dynamic lighting affects how instance batches are handled. Needs thought...
* The use of textures is limited so far, instead relying on customisation of materials and shaders for effects. _Not too scary either_ as texture switching is already restricted to material and shader switch points (ie, rare). Alternatives like the physically-based rendering coming to Raylib are also potentially more interesting than messing with Blender anyway.

None of this is rocket science; plenty of game engines do the same stuff and much more. I guess I'm as-yet-unconvinced that Factorio is so complex or so large that it can only be done with 2D sprites.
