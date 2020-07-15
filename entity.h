#ifndef _H_entity
#define _H_entity

#include "sparse.h"
#include "spec.h"
#include "point.h"
#include "box.h"

#define MaxEntity 1000000

struct Entity {
	int id;
	uint32_t flags;
	Spec* spec;
	Point pos;

	bool isGhost();
	Entity& setGhost(bool state);

	Box box();
	enum Direction dir();
	Entity& face(enum Direction);
	Entity& move(Point p);
	Entity& floor(float level);
	Entity& rotate();
	void destroy();

	static const uint32_t GHOST = 1<<0;

	static inline SparseArray<Entity> all = (MaxEntity);
	static inline SparseArray<enum Direction> dirs = (MaxEntity);
	static int next();

	static Entity& create(int id, Spec* spec);
	static Entity& load(int id);
};

struct GuiEntity {
	int id;
	Spec* spec;
	Point pos;
	enum Direction dir;
	bool ghost;

	GuiEntity(int id);
	~GuiEntity();

	Box box();
};

struct GuiFakeEntity {
	Spec* spec;
	Point pos;
	enum Direction dir;
	bool ghost;

	GuiFakeEntity(Spec* spec);
	~GuiFakeEntity();

	Box box();
	GuiFakeEntity* face(enum Direction);
	GuiFakeEntity* move(Point p);
	GuiFakeEntity* floor(float level);
	GuiFakeEntity* rotate();
};

#endif