#ifndef _H_entity
#define _H_entity

#define MaxEntity 1000000

struct Entity;
struct GuiEntity;
struct GuiFakeEntity;

#include "sparse.h"
#include "spec.h"
#include "part.h"
#include "chunk.h"
#include "point.h"
#include "box.h"
#include "store.h"
#include "vehicle.h"
#include <unordered_set>
#include <vector>

struct Store;

struct Entity {

	static const uint32_t GHOST = 1<<0;
	static inline std::map<Chunk::XY,std::set<int>> grid;

	static inline SparseArray<Entity> all = (MaxEntity);
	static int next();

	static Entity& create(int id, Spec* spec);
	static bool exists(int id);
	static Entity& get(int id);

	static void saveAll(const char* name);
	static void loadAll(const char* name);
	static void reset();

	static std::unordered_set<int> intersecting(Box box);

	int id;
	uint32_t flags;
	Spec* spec;
	Point pos;
	Point dir;

	bool isGhost();
	Entity& setGhost(bool state);

	Box box();
	Entity& look(Point); // rel
	Entity& lookAt(Point); // abs
	bool lookAtPivot(Point);
	Entity& move(Point p);
	Entity& move(float x, float y, float z);
	Entity& floor(float level);
	Entity& rotate();
	void destroy();

	Entity& index();
	Entity& unindex();

	Store& store();
	Vehicle& vehicle();
};

struct GuiEntity {
	int id;
	Spec* spec;
	Point pos;
	Point dir;
	bool ghost;

	GuiEntity();
	GuiEntity(int id);
	~GuiEntity();

	Box box();
	Matrix transform();
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