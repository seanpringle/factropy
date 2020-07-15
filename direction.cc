#include "common.h"
#include "direction.h"

namespace Directions {

	enum Direction rotate(enum Direction dir) {
		switch (dir) {
			case South: return West;
			case North: return East;
			case East: return South;
			case West: return North;
		}
		fatalf("impossible");
	}

	float degrees(enum Direction dir) {
		switch (dir) {
			case South: return 0.0f;
			case North: return 180.0f;
			case East: return 270.0f;
			case West: return 90.0f;
		}
		fatalf("impossible");
	}

}