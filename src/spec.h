#pragma once

// Spec(ifications) describe everything about a type of entity. They are
// complex and huge so that entity components can be simple and small.

struct Spec;
typedef int Health;

#include "raylib-ex.h"
#include <string>
#include <vector>
#include <map>
#include <set>
#include "point.h"
#include "mat4.h"
#include "item.h"
#include "part.h"
#include "area.h"
#include "volume.h"
#include "mass.h"
#include "energy.h"
#include "time-series.h"

struct Spec {

	enum Place {
		Footings = 0,
		Land = 1<<1,
		Water = 1<<2,
		Hill = 1<<3,
	};

	struct Footing {
		Place place;
		Point point;
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
	bool loader;
	bool shunt;
	bool rotate;
	bool toggle;
	bool enable;
	bool junk;
	bool named;
	bool block;

	bool forceDelete;
	bool collideBuild;

	Box collision;
	Box selection;
	EnergyUser electrical;

	int place;
	bool placeOnHill;
	std::vector<Footing> footings;

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

	bool drone;
	float droneSpeed;

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
	bool pipeUnderground;
	float pipeUndergroundRange;

	bool turret;
	float turretRange;
	float turretPivot;
	int turretCooldown;
	std::string turretBulletSpec;

	bool missile;
	float missileSpeed;
	bool missileBallistic;

	bool conveyor;
	Point conveyorInput;
	Point conveyorOutput;
	std::vector<Mat4> conveyorTransforms;

	bool unveyor;
	bool unveyorEntry;
	float unveyorRange;

	bool ropeway;
	bool ropewayTerminus;
	bool ropewayTower;
	bool ropewayBucket;
	Spec* ropewayBucketSpec;
	Point ropewayCableEast;

	bool computer;
	bool networker;

	Spec* pipette;
	Spec* cycle;

	bool build;
	bool select;

	Spec* statsGroup;
	TimeSeries energyConsumption;

	Spec(std::string name);
	~Spec();
	Point aligned(Point p, Point dir);
	Box box(Point pos, Point dir, Box vol);
	Box southBox(Point pos);
	bool hasStore();
	void setCornerSupports();

	static std::vector<Point> relativePoints(const std::vector<Point> points, const Matrix rotation, const Point position);
};

