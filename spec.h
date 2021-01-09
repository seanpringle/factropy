#ifndef _H_spec
#define _H_spec

struct Spec;
typedef int Health;

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
	Health health;

	Image image;
	RenderTexture texture;
	bool align;
	bool pivot;
	bool belt;
	bool loader;
	bool lift;
	bool shunt;
	bool drone;
	bool rotate;
	bool junk;
	bool named;

	Volume collision;
	EnergyUser electrical;

	enum Place place;

	float costGreedy;
	float clearance;

	// the entity explosing
	bool explodes;
	std::string explosionSpec;

	// the resulting explosion
	bool explosion;
	float explosionRate;
	float explosionRadius;
	Health explosionDamage;

	// store
	bool store;
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

	// tanks
	bool tank;
	Mass tankCapacity;

	bool crafter;
	bool crafterProgress;
	std::set<std::string> recipeTags;

	bool projector;

	bool teleporter;

	bool arm;
	float armOffset;
	float armSpeed;

	bool depot;
	uint drones;

	std::vector<Stack> materials;

	bool consumeChemical;
	bool consumeElectricity;
	bool consumeThermalFluid;
	Energy energyConsume;
	Energy energyDrain;
	bool generateElectricity;
	Energy energyGenerate;

	bool vehicle;
	bool vehicleStop;
	Energy vehicleEnergy;
	bool vehicleWaitActivity;

	bool pipe;
	std::vector<Point> pipeConnections;
	std::vector<Point> pipeInputConnections;
	std::vector<Point> pipeOutputConnections;
	Liquid pipeCapacity;

	bool turret;
	float turretRange;
	int turretCooldown;
	std::string turretBulletSpec;

	bool missile;
	float missileSpeed;
	bool missileBallistic;

	bool processor;
	uint processorSpeed;
	uint processorMarkStack;
	uint processorDataStack;
	uint processorReturnStack;
	uint processorMemory;

	Spec(std::string name);
	~Spec();
	Point aligned(Point p, Point dir);
	Box box(Point pos, Point dir);
	bool hasStore();

	static std::vector<Point> relativePoints(const std::vector<Point> points, const Matrix rotation, const Point position);
};

#endif
