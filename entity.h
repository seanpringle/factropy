#ifndef _H_entity
#define _H_entity

#include "sparse.h"
#include "spec.h"
#include "point.h"
#include "box.h"

#define MaxEntity 1000000

struct Entity {
	int id;
	Spec* spec;
	Point pos;

	Box box();
	Entity& move(Point p);
};

namespace Entities {
	int next();
	extern SparseArray<Entity> all;

	Entity& create(int id, Spec* spec);
	void destroy(Entity& en);
}

#endif