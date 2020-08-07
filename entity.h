#ifndef _H_entity
#define _H_entity

struct Entity;
struct GuiEntity;
struct GuiFakeEntity;

#include "sparse.h"
#include "spec.h"
#include "chunk.h"
#include "part.h"
#include "point.h"
#include "box.h"
#include "ghost.h"
#include "store.h"
#include "arm.h"
#include "belt.h"
#include "lift.h"
#include "crafter.h"
#include "drone.h"
#include "depot.h"
#include "vehicle.h"
#include "burner.h"
#include <unordered_set>
#include <vector>

struct Entity {

	static const uint32_t GHOST = 1<<0;
	static const uint32_t CONSTRUCTION = 1<<1;
	static const uint32_t DECONSTRUCTION = 1<<2;

	static inline std::map<Chunk::XY,std::set<uint>> grid;
	static inline std::unordered_set<uint> removing;

	static inline float electricityLoad = 0.0f;
	static inline float electricitySatisfaction = 0.0f;
	static inline Energy electricityDemand = 0;
	static inline Energy electricitySupply = 0;
	static inline Energy electricityCapacity = 0;
	static inline std::set<uint> electricityConsumers;
	static inline std::set<uint> electricityGenerators;

	static inline SparseArray<Entity> all = (MaxEntity);
	static inline uint sequence = 0;
	static uint next();

	static Entity& create(uint id, Spec* spec);
	static bool exists(uint id);
	static Entity& get(uint id);

	static void saveAll(const char* name);
	static void loadAll(const char* name);
	static void reset();
	static void preTick();

	static bool fits(Spec *spec, Point pos, Point dir);

	static std::unordered_set<uint> intersecting(Box box);
	static uint at(Point p);

	uint id;
	uint32_t flags;
	Spec* spec;
	Point pos;
	Point dir;
	uint state;

	bool isGhost();
	Entity& setGhost(bool state);
	bool isConstruction();
	Entity& setConstruction(bool state);
	bool isDeconstruction();
	Entity& setDeconstruction(bool state);

	Box box();
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

	Ghost& ghost();
	Store& store();
	std::vector<Store*> stores();
	Crafter& crafter();
	Vehicle& vehicle();
	Arm& arm();
	Belt& belt();
	Lift& lift();
	Drone& drone();
	Depot& depot();
	Burner& burner();
};

struct GuiEntity {
	uint id;
	Spec* spec;
	Point pos;
	Point dir;
	uint state;
	bool ghost;
	Matrix transform;

	GuiEntity();
	GuiEntity(uint id);
	~GuiEntity();

	Box box();
	Point ground();
	void updateTransform();
};

struct GuiFakeEntity : GuiEntity {
	GuiFakeEntity(Spec* spec);
	~GuiFakeEntity();

	GuiFakeEntity* move(Point p);
	GuiFakeEntity* move(float x, float y, float z);
	GuiFakeEntity* floor(float level);
	GuiFakeEntity* rotate();
};

#endif