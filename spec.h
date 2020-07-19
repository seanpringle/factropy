#ifndef _H_spec
#define _H_spec

#include <string>
#include <vector>
#include <map>
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "point.h"
#include "direction.h"
#include "part.h"

struct Spec {

	static inline std::map<std::string,Spec*> all;
	static Spec* byName(std::string name);

	static void saveAll(const char *path);
	static void loadAll(const char *path);
	static void reset();

	struct Animation {
		float w;
		float h;
		float d;
	};

	std::string name;
	std::vector<Part*> parts;
	bool align;
	bool rotate;
	bool rotateGhost;
	bool vehicle;
	Image image;

	Animation animations[4];

	Spec(std::string name);
	Point aligned(Point p, enum Direction dir);
	bool hasDirection();
	bool hasOrientation();
};

#endif
