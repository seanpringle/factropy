#include "common.h"
#include "belt.h"

void Belt::reset() {
	all.clear();
}

void Belt::tick() {
	BeltSegment::tick();
}

Belt& Belt::create(uint id) {
	Belt& belt = all.ref(id);
	belt.id = id;
	belt.segment = NULL;
	belt.offset = 0;
	return belt;
}

Belt& Belt::get(uint id) {
	ensuref(all.has(id), "invalid belt access %d", id);
	return all.ref(id);
}

void Belt::destroy() {
	if (segment) {
		unmanage();
	}
	all.drop(id);
}

Belt& Belt::manage() {
	ensure(segment == NULL);
	Entity& en = Entity::get(id);

	uint aid = Entity::at(en.pos + en.dir);
	uint bid = Entity::at(en.pos - en.dir);

	Entity* aen = aid ? &Entity::get(aid): NULL;
	Entity* ben = bid ? &Entity::get(bid): NULL;

	Belt* aheadBelt = aen && aen->spec->belt ? &aen->belt(): NULL;
	Belt* behindBelt = ben && ben->spec->belt ? &ben->belt(): NULL;

	bool aheadSameDir = aen && aen->dir == en.dir;
	bool behindSameDir = ben && ben->dir == en.dir;

	bool aheadJoin = aheadBelt && aheadSameDir && aheadBelt->segment;
	bool behindJoin = behindBelt && behindSameDir && behindBelt->segment;

	for (;;) {
		if (aheadJoin && behindJoin) {
			aheadBelt->segment->push(this);
			aheadBelt->segment->join(behindBelt->segment);
			break;
		}

		if (aheadJoin) {
			aheadBelt->segment->push(this);
			break;
		}

		(new BeltSegment())->push(this);

		if (behindJoin) {
			segment->join(behindBelt->segment);
			break;
		}

		break;
	}

	ensure(segment);
	return *this;
}

Belt& Belt::unmanage() {
	ensure(segment != NULL);
	segment->split(offset);
	ensure(segment == NULL);
	return *this;
}

bool Belt::insert(uint iid, uint area) {
	return segment->insert(offset, iid, area);
}

bool Belt::remove(uint iid, uint area) {
	return segment->remove(offset, iid, area);
}

uint Belt::removeAny(uint area) {
	return segment->removeAny(offset, area);
}

uint Belt::itemAt(uint area) {
	return segment->itemAt(offset, area);
}