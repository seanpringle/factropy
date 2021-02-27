# test9

A hobby gamedev project heavily influenced by Factorio.

The only thing more fun than playing a base-building simulation game is making your own!

Yes, there were eight earlier versions made while messing with ideas.

[![C/C++ CI](https://github.com/seanpringle/test9/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/seanpringle/test9/actions/workflows/c-cpp.yml)

# tech stack

* [raylib](https://github.com/raysan5/raylib)
* [imgui](https://github.com/ocornut/imgui)
* [wren](https://github.com/wren-lang/wren)
* [c++17](https://en.wikipedia.org/wiki/C%2B%2B17)
* [openscad](http://www.openscad.org/)

# design aims

**No player character**. Free 3D camera view, RTS style.

**Emphasize vehicles**. Make AAI-style vehicles and Logistic Carts first class citizens of the engine and see if it works. Satisfactory trucks are cool but could be smarter.

**De-emphasize robots, and call them drones**. Make construction and logistic ~~robots~~ drones only operate within range of their own ~~roboport~~ depot. No massive networks covering the map so one waits for ages for a distant robot to arrive. More use of construction vehicles and supply chains.

**Inserters are cool, but those are "arms"!** So, definitely keep ~~inserters~~ arms. They lead to such interesting emergent gameplay. Satisfactory's belts and entities are kind of boring connect-the-dots puzzles in comparison.

**Single-lane conveyor belts**. But with relatively larger other entities. Wonder if part of the reason Factorio has slightly absurd dual-lane belts is to allow enough resources to get close to an assembler so inserters can reach and recipes can be complex. Try making the asssmblers larger instead for more adjacent space, and see if it works out.

**Use an entity-component system**. There was discussion in the Factorio forums about the game not using an ECS. Curious what it would look like with an ECS under the hood, and whether modding could be more flexible (or possibly more broken -- who knows).
