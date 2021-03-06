#pragma once

// The core of the Entity Component System. All entities have an Entity struct
// with a unique uint ID. Component structs store the ID to associate with an
// entity.

struct Entity;

// Entities visible on screen are not exposed directly to the rendering thread,
// but loaded into a GuiEntity when a frame starts. This allows the rendering
// thread to mostly proceed without locking the Sim, decoupling UPS and FPS.

struct GuiEntity;
struct GuiFakeEntity;

#include <set>
#include <map>
#include <vector>
#include "slabmap.h"
#include "gridmap.h"
#include "spec.h"
#include "recipe.h"
#include "chunk.h"
#include "part.h"
#include "mat4.h"
#include "point.h"
#include "box.h"
#include "sphere.h"
#include "ghost.h"

// Components

#include "store.h"
#include "arm.h"
#include "conveyor.h"
#include "unveyor.h"
#include "loader.h"
#include "ropeway.h"
#include "pipe.h"
#include "crafter.h"
#include "projector.h"
#include "drone.h"
#include "missile.h"
#include "explosion.h"
#include "depot.h"
#include "vehicle.h"
#include "burner.h"
#include "generator.h"
#include "turret.h"
#include "computer.h"

struct Entity {
	uint id;
	uint32_t flags;
	Spec* spec;
	Point pos;
	Point dir;
	uint state;
	Health health;

	static const uint32_t GHOST = 1<<0;
	static const uint32_t CONSTRUCTION = 1<<1;
	static const uint32_t DECONSTRUCTION = 1<<2;
	static const uint32_t ENABLED = 1<<3;
	static const uint32_t GENERATING = 1<<4;

	static inline gridmap<32,uint> grid;
	static inline std::set<uint> removing;
	static inline std::set<uint> exploding;

	static inline float electricityLoad = 0.0f;
	static inline float electricitySatisfaction = 0.0f;
	static inline Energy electricityDemand = 0;
	static inline Energy electricitySupply = 0;
	static inline Energy electricityCapacity = 0;
	static inline Energy electricityCapacityReady = 0;
	static inline std::set<uint> electricityGenerators;
	static inline std::map<uint,Energy> energyConsumers;

	static inline slabmap<Entity,&Entity::id> all;
	static inline uint sequence = 0;
	static uint next();

	static inline std::map<uint,std::string> names;

	static Entity& create(uint id, Spec* spec);
	static bool exists(uint id);
	static Entity& get(uint id);

	static void saveAll(const char* name);
	static void loadAll(const char* name);
	static void reset();
	static void preTick();

	static bool fits(Spec *spec, Point pos, Point dir);

	static std::vector<uint> intersecting(Box box);
	static std::vector<uint> intersecting(Sphere sphere);
	static std::vector<uint> intersecting(Point pos, float radius);
	static uint at(Point p);

	static std::vector<uint> enemiesInRange(Point pos, float radius);

	bool isGhost();
	Entity& setGhost(bool state);
	bool isConstruction();
	Entity& setConstruction(bool state);
	bool isDeconstruction();
	Entity& setDeconstruction(bool state);
	bool isEnabled();
	Entity& setEnabled(bool state);
	bool isGenerating();
	Entity& setGenerating(bool state);

	std::string name();
	bool rename(std::string n);
	Box box();
	Sphere sphere();
	Box miningBox();
	Point ground();
	Entity& look(Point); // rel
	Entity& lookAt(Point); // abs
	bool lookAtPivot(Point);
	Entity& move(Point p);
	Entity& move(float x, float y, float z);
	Entity& floor(float level);
	Entity& rotate();
	Entity& toggle();
	void destroy();
	void remove();
	void explode();
	static void removeJunk(Box b);

	Entity& index();
	Entity& unindex();
	Entity& manage();
	Entity& unmanage();

	Entity& construct();
	Entity& deconstruct();
	Entity& materialize();

	Energy consume(Energy e);
	float consumeRate(Energy e);
	void generate();

	void damage(Health hits);

	Ghost& ghost();
	Store& store();
	std::vector<Store*> stores();
	Crafter& crafter();
	Projector& projector();
	Vehicle& vehicle();
	Arm& arm();
	Conveyor& conveyor();
	Unveyor& unveyor();
	Loader& loader();
	Ropeway& ropeway();
	RopewayBucket& ropewayBucket();
	Pipe& pipe();
	Drone& drone();
	Missile& missile();
	Explosion& explosion();
	Depot& depot();
	Burner& burner();
	Generator& generator();
	Turret& turret();
	Computer& computer();
};

struct GuiEntity {
	uint id;
	Spec* spec;
	Point pos;
	Point dir;
	uint state;
	Health health;
	bool ghost;
	Mat4 transform;
	Point aim;

	struct {
		Energy energy;
		Energy buffer;
	} burner;

	struct {
		Mass limit;
		Mass usage;
		minivec<Store::Level> levels;
	} store;

	struct {
		Recipe* recipe;
		float progress;
		float inputsProgress;
		int completed;
	} crafter;

	struct {
		uint fid;
		float level;
	} pipe;

	struct {
		float radius;
	} explosion;

	struct {
		uint iid;
		uint offset;
	} conveyor;

	struct {
		uint iid;
	} arm;

	struct {
		uint iid;
	} drone;

	struct {
		uint next;
		Point a;
		Point b;
		Point c;
		Point d;
	} ropeway;

	GuiEntity();
	GuiEntity(uint id);
	~GuiEntity();

	Box box();
	Box selectionBox();
	Box southBox();
	Box miningBox();
	Box supportBox();
	Point ground();
	Sphere sphere();
	void updateTransform();
	Mat4 partTransform(Part* part);

	bool connectable(GuiEntity* other);
};

struct GuiFakeEntity : GuiEntity {
	GuiFakeEntity(Spec* spec);
	~GuiFakeEntity();

	GuiFakeEntity* getConfig(Entity& en);
	GuiFakeEntity* setConfig(Entity& en);
	GuiFakeEntity* move(Point p);
	GuiFakeEntity* move(float x, float y, float z);
	GuiFakeEntity* floor(float level);
	GuiFakeEntity* rotate();
};
