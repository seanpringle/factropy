#ifndef _H_spec
#define _H_spec

struct Spec;

#include <string>
#include <vector>
#include <map>
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "point.h"
#include "part.h"

struct Spec {

	static inline std::map<std::string,Spec*> all;
	static Spec* byName(std::string name);

	static void saveAll(const char *path);
	static void loadAll(const char *path);
	static void reset();

	std::string name;
	std::vector<Part*> parts;
	bool align;
	bool pivot;
	bool vehicle;
	bool drone;
	bool store;
	Image image;

	float w;
	float h;
	float d;

	Spec(std::string name);
	Point aligned(Point p, Point axis);
	bool hasStore();
};

#endif
