#ifndef _H_spec
#define _H_spec

#include <string>
#include <map>
#include "raylib.h"
#include "raymath.h"
#include "point.h"
#include "direction.h"

struct Spec {

	struct Animation {
		float w;
		float h;
		float d;
	};

	std::string name;
	std::string obj;
	Model model;
	Color color;
	bool align;
	bool rotate;
	bool rotateGhost;

	Animation animations[4];

	Spec(std::string name);
	Point aligned(Point p, enum Direction dir);
	bool hasDirection();

	inline static std::map<std::string,Spec*> all;
	static Spec* byName(std::string name);
};

#endif
