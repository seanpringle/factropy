#ifndef _H_direction
#define _H_direction

enum Direction {
	South = 0,
	North,
	East,
	West,
};

namespace Directions {
	enum Direction rotate(enum Direction dir);
	float degrees(enum Direction dir);
}

#endif