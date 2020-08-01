#ifndef _H_spec
#define _H_spec

struct Spec;

#include <string>
#include <vector>
#include <map>
#include <set>
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "point.h"
#include "part.h"

struct Spec {

	enum Place {
		Land = 1,
		Water,
		Hill,
	};

	static inline std::map<std::string,Spec*> all;
	static Spec* byName(std::string name);

	static void saveAll(const char *path);
	static void loadAll(const char *path);
	static void reset();

	std::string name;
	std::vector<Part*> parts;
	std::vector<std::vector<Matrix>> states;
	Image image;
	RenderTexture texture;
	bool align;
	bool pivot;
	bool arm;
	bool belt;
	bool lift;
	bool vehicle;
	bool drone;
	bool store;
	bool rotate;

	float w;
	float h;
	float d;

	enum Place place;

	float costGreedy;
	float clearance;

	// store
	bool magic;
	bool enableSetLower;
	bool enableSetUpper;
	// loaders
	bool loadAnything;
	bool unloadAnything;
	// drones
	bool logistic;
	bool loadPriority;
	bool supplyPriority;
	bool defaultOverflow;

	bool crafter;
	std::set<std::string> recipeTags;

	Spec(std::string name);
	~Spec();
	Point aligned(Point p, Point dir);
	Box box(Point pos, Point dir);
	bool hasStore();
};

#endif
