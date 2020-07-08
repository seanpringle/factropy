#include "common.h"
#include "entity.h"
#include "sparse.h"

#include <map>
#include <stdio.h>

namespace Entities {

	SparseArray<Entity> all = (MaxEntity);

	int next() {
		int i = all.next(false);
		ensuref(i > 0, "max entities reached");
		return i;
	}

	Entity& create(int id, Spec *spec) {
		notef("Entity: %d %s", id, spec->name.c_str());
		Entity& en = all.ref(id);
		en.id = id;
		en.spec = spec;
		en.pos = (Point){0,0,0};
		return en;
	}

	void destroy(Entity& en) {
		all.drop(en.id);
	}
}

using namespace Entities;

Box Entity::box() {
	return (Box){pos.x, pos.y, pos.z, spec->w, spec->h, spec->d};
}

Entity& Entity::move(Point p) {
	pos = p;
	return *this;
}