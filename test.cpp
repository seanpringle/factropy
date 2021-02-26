#include "common.h"
#include "grid.h"

#include "grid.cc"
#include "mat4.cc"
#include "box.cc"
#include "point.cc"
#include <iostream>

int main(int argc, char const *argv[]) {
	auto grid = Grid(32);

	grid.insert({0.5f,0.5f,0.5f,1.0f,1.0f,1.0f}, 1);
	grid.insert({0.5f,0.5f,32.5f,1.0f,1.0f,1.0f}, 2);

	for (auto& id: grid.search({0.5f,0.5f,31.5f,1.0f,1.0f,1.0f})) {
		std::cout << id << std::endl;
	}

	return 0;
}