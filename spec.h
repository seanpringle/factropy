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
#include "mat4.h"
#include "item.h"
#include "part.h"
#include "area.h"
#include "volume.h"
#include "mass.h"
#include "energy.h"

struct Spec {

	enum Place {
		Land = 1,
		Water,
		Hill,
	};

	struct EnergyUser {
		Area area;
		Energy rate;
	};

	static inline std::map<std::string,Spec*> all;
	static Spec* byName(std::string name);

	static void saveAll(const char *path);
	static void loadAll(const char *path);
	static void reset();

	std::string name;
	std::vector<Part*> parts;
	std::vector<std::vector<Mat4>> states;
	Image image;
	RenderTexture texture;
	bool align;
	bool pivot;
	bool belt;
	bool lift;
	bool shunt;
	bool drone;
	bool store;
	bool rotate;
	bool junk;

	Volume collision;
	EnergyUser electrical;

	enum Place place;

	float costGreedy;
	float clearance;

	// store
	Mass capacity;
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
	bool crafterProgress;
	std::set<std::string> recipeTags;

	bool teleporter;

	bool arm;
	float armOffset;
	float armSpeed;

	bool depot;
	uint drones;

	std::vector<Stack> materials;

	bool consumeChemical;
	bool consumeElectricity;
	Energy energyConsume;
	bool generateElectricity;
	Energy energyGenerate;

	bool vehicle;
	Energy vehicleEnergy;
	bool vehicleWaitActivity;

	Spec(std::string name);
	~Spec();
	Point aligned(Point p, Point dir);
	Box box(Point pos, Point dir);
	bool hasStore();
};

#endif
