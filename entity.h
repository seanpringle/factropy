#ifndef _H_entity
#define _H_entity

#define MaxEntity 1000000

struct Entity;
struct GuiEntity;
struct GuiFakeEntity;

#include "sparse.h"
#include "spec.h"
#include "chunk.h"
#include "part.h"
#include "point.h"
#include "box.h"
#include "store.h"
#include "arm.h"
#include "belt.h"
#include "crafter.h"
#include "vehicle.h"
#include <unordered_set>
#include <vector>

struct Store;

struct Entity {

	static const uint32_t GHOST = 1<<0;
	static const uint32_t CONSTRUCTION = 1<<1;
	static const uint32_t DECONSTRUCTION = 1<<2;

	static inline std::map<Chunk::XY,std::set<uint>> grid;
	static inline std::unordered_set<uint> removing;

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
	Entity& look(Point); // rel
	Entity& lookAt(Point); // abs
	bool lookAtPivot(Point);
	Entity& move(Point p);
	Entity& move(float x, float y, float z);
	Entity& floor(float level);
	Entity& rotate();
	void destroy();
	void remove();

	Entity& index();
	Entity& unindex();

	Entity& construct();
	Entity& deconstruct();
	Entity& materialize();

	Store& store();
	Crafter& crafter();
	Vehicle& vehicle();
	Arm& arm();
	Belt& belt();
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