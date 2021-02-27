# test9

A hobby gamedev project heavily influenced by Factorio.

The only thing more fun than playing a base-building simulation game is making your own!

# tech stack

* [raylib](https://github.com/raysan5/raylib)
* [imgui](https://github.com/ocornut/imgui)
* [c++17](https://en.wikipedia.org/wiki/C%2B%2B17)
* [openscad](http://www.openscad.org/)

# design aims

*No player character*. Free 3D camera view with construction vehicles and drones, RTS style.

*Emphasize vehicles*. Make AAI-style vehicles and Logistic Carts first class citizens of the engine and see if it works. Satisfactory trucks are cool but could be smarter.

*De-emphasize robots, and call them drones*. Make construction and logistic ~~robots~~ drones only operate within range of their own ~~roboport~~ possibly-mobile depot. No massive networks covering the map so one waits for ages for a distant construction robot to arrive.

*Inserters are cool, but they're arms!* So, have ~~inserters~~ arms, but otherwise the same idea.

*Single-lane conveyor belts*. But reltively larger other entities. Wonder if part of the reason Factorio has dual-lane belts is to allow enough resources to get close to an Assembler so inserters can reach. Make the asssmblers larger instead for more adjacent space.

*Use an entity-component system*. There was discussion in the Factorio forums about the game not using an ECS. Curious what it would look like with an ECS.
