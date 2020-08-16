#include "common.h"
#include "item.h"
#include "entity.h"
#include "chunk.h"
#include "sparse.h"

#include <map>
#include <stdio.h>
#include <algorithm>

void Entity::reset() {
	for (Entity& en: all) {
		en.destroy();
	}
	all.clear();
	grid.clear();
	Store::reset();
}

void Entity::preTick() {
	for (int id: removing) {
		Entity::get(id).destroy();
	}
	removing.clear();

	electricitySupply = 0;
	electricityCapacity = 0;
	electricityCapacityReady = 0;

	for (uint eid: electricityGenerators) {
		Entity& en = Entity::get(eid);
		if (en.isGhost()) continue;
		en.generate();
	}

	electricityLoad = electricityDemand.portion(electricityCapacityReady);
	electricitySatisfaction = electricitySupply.portion(electricityDemand);
	electricityDemand = 0;
}

uint Entity::next() {
	for (uint i = 0; i < MaxEntity; i++) {
		if (sequence == MaxEntity) {
			sequence = 0;
		}
		sequence++;
		if (!all.has(sequence)) {
			return sequence;
		}
	}
	fatalf("max entities reached");
}

Entity& Entity::create(uint id, Spec *spec) {
	Entity& en = all.ref(id);
	en.id = id;
	en.spec = spec;
	en.dir = Point::South;
	en.state = 0;

	en.flags = GHOST;
	Ghost::create(id, Entity::next());

	if (spec->store) {
		Store::create(id, Entity::next(), spec->capacity);
	}

	if (spec->crafter) {
		Crafter::create(id);
	}

	if (spec->drone) {
		Drone::create(id);
	}

	if (spec->depot) {
		Depot::create(id);
	}

	if (spec->vehicle) {
		Vehicle::create(id);
	}

	if (spec->arm) {
		Arm::create(id);
	}

	if (spec->belt) {
		Belt::create(id);
	}

	if (spec->lift) {
		Lift::create(id);
	}

	if (spec->consumeChemical) {
		Burner::create(id, Entity::next());
	}

	if (spec->consumeElectricity) {
		electricityConsumers.insert(id);
	}

	if (spec->generateElectricity) {
		electricityGenerators.insert(id);
	}

	en.pos = {0,0,0};
	en.move((Point){0,0,0});

	return en;
}

void Entity::destroy() {
	unindex();

	if (isGhost()) {
		ghost().destroy();
	}

	if (spec->store) {
		store().destroy();
	}

	if (spec->crafter) {
		crafter().destroy();
	}

	if (spec->vehicle) {
		vehicle().destroy();
	}

	if (spec->drone) {
		drone().destroy();
	}

	if (spec->depot) {
		depot().destroy();
	}

	if (spec->arm) {
		arm().destroy();
	}

	if (spec->belt) {
		belt().destroy();
	}

	if (spec->lift) {
		lift().destroy();
	}

	if (spec->consumeElectricity) {
		electricityConsumers.erase(id);
	}

	if (spec->consumeChemical) {
		burner().destroy();
	}

	if (spec->generateElectricity) {
		electricityGenerators.erase(id);
	}

	all.drop(id);
}

bool Entity::exists(uint id) {
	return id > 0 && all.has(id);
}

Entity& Entity::get(uint id) {
	ensuref(id > 0 && all.has(id), "invalid id %d", id);
	return all.ref(id);
}

bool Entity::fits(Spec *spec, Point pos, Point dir) {
	Box bounds = spec->box(pos, dir).shrink(0.1);
	switch (spec->place) {
		case Spec::Land: {
			if (!Chunk::isLand(bounds)) {
				return false;
			}
			break;
		}
		case Spec::Water: {
			if (!Chunk::isWater(bounds)) {
				return false;
			}
			break;
		}
		case Spec::Hill: {
			if (!Chunk::isHill(bounds)) {
				return false;
			}
			break;
		}
	}
	if (intersecting(bounds).size() > 0) {
		return false;
	}
	return true;
}

std::unordered_set<uint> Entity::intersecting(Box box) {
	std::unordered_set<uint> hits;
	for (auto xy: Chunk::walk(box)) {
		for (uint id: grid[xy]) {
			if (get(id).box().intersects(box)) {
				hits.insert(id);
			}
		}
	}
	return hits;
}

uint Entity::at(Point p) {
	Box box = p.box();
	for (auto xy: Chunk::walk(box)) {
		for (uint id: grid[xy]) {
			if (get(id).box().intersects(box)) {
				return id;
			}
		}
	}
	return 0;
}

void Entity::remove() {
	removing.insert(id);
}

void Entity::removeJunk(Box b) {
	for (uint eid: intersecting(b)) {
		Entity& ke = Entity::get(eid);
		if (ke.spec->junk) {
			ke.remove();
		}
	}
}

bool Entity::isGhost() {
	return (flags & GHOST) != 0;
}

Entity& Entity::setGhost(bool state) {
	flags = state ? (flags | GHOST) : (flags & ~GHOST);
	return *this;
}

bool Entity::isConstruction() {
	return (flags & CONSTRUCTION) != 0;
}

Entity& Entity::setConstruction(bool state) {
	flags = state ? (flags | CONSTRUCTION) : (flags & ~CONSTRUCTION);
	return *this;
}

bool Entity::isDeconstruction() {
	return (flags & DECONSTRUCTION) != 0;
}

Entity& Entity::setDeconstruction(bool state) {
	flags = state ? (flags | DECONSTRUCTION) : (flags & ~DECONSTRUCTION);
	return *this;
}

Box Entity::box() {
	return spec->box(pos, dir);
}

Box Entity::miningBox() {
	return box().grow(0.5f);
}

Entity& Entity::look(Point p) {
	unindex();
	dir = p.normalize();
	index();
	return *this;
}

Entity& Entity::lookAt(Point p) {
	look(p-pos);
	return *this;
}

bool Entity::lookAtPivot(Point o) {
	o = (o-pos).normalize();
	look(dir.pivot(o, 0.01));
	return dir == o;
}

Entity& Entity::index() {
	unindex();
	for (auto xy: Chunk::walk(box())) {
		grid[xy].insert(id);
	}
	return *this;
}

Entity& Entity::unindex() {
	for (auto xy: Chunk::walk(box())) {
		grid[xy].erase(id);
	}
	return *this;
}

Entity& Entity::manage() {
	if (!isGhost()) {
		if (spec->belt) {
			belt().manage();
		}
	}
	return *this;
}

Entity& Entity::unmanage() {
	if (!isGhost()) {
		if (spec->belt) {
			belt().unmanage();
		}
	}
	return *this;
}

Entity& Entity::construct() {
	bool wasGhost = isGhost();

	if (!wasGhost) {
		Ghost::create(id, Entity::next());
	}

	unmanage();
	setGhost(true);
	setConstruction(true);
	setDeconstruction(false);

	for (Stack stack: spec->materials) {
		ghost().store.levelSet(stack.iid, stack.size, stack.size);
	}

	return *this;
}

Entity& Entity::deconstruct() {
	bool wasGhost = isGhost();

	if (!wasGhost) {
		Ghost::create(id, Entity::next());
	}

	unmanage();
	setGhost(true);
	setConstruction(false);
	setDeconstruction(true);

	Store& gstore = ghost().store;
	for (Stack stack: spec->materials) {
		if (!wasGhost) {
			gstore.insert(stack);
		}
		gstore.levelSet(stack.iid, 0, 0);
	}

	return *this;
}

Entity& Entity::materialize() {
	ghost().destroy();

	setGhost(false);
	setConstruction(false);
	setDeconstruction(false);
	manage();
	return *this;
}

Entity& Entity::move(Point p) {
	unmanage();
	unindex();
	pos = spec->aligned(p, dir);
	index();
	manage();
	return *this;
}

Entity& Entity::move(float x, float y, float z) {
	return move(Point(x, y, z));
}

Entity& Entity::floor(float level) {
	unmanage();
	unindex();
	pos.y = level + spec->collision.h/2.0f;
	index();
	manage();
	return *this;
}

Point Entity::ground() {
	return {pos.x, pos.y - spec->collision.h/2.0f, pos.z};
}

Entity& Entity::rotate() {
	if (spec->rotate) {
		unmanage();
		unindex();
		dir = dir.rotateHorizontal();
		pos = spec->aligned(pos, dir);
		index();
		manage();
	}
	return *this;
}

Entity& Entity::toggle() {
	if (spec->lift) {
		lift().toggle();
	}
	return *this;
}

Energy Entity::consume(Energy e) {
	if (spec->consumeElectricity) {
		electricityDemand += e;
		return e * electricitySatisfaction;
	}
	if (spec->consumeChemical) {
		return burner().consume(e);
	}
	return 0;
}

float Entity::consumeRate(Energy e) {
	return consume(e).portion(e);
}

void Entity::generate() {
	if (spec->generateElectricity && spec->consumeChemical) {

		electricitySupply += burner().consume(spec->energyGenerate * electricityLoad);
		if (!burner().store.isEmpty()) electricityCapacityReady += spec->energyGenerate;
		electricityCapacity += spec->energyGenerate;

		state = std::floor((float)burner().energy.value/(float)burner().buffer.value * (float)spec->states.size());
	}
}

Ghost& Entity::ghost() {
	return Ghost::get(id);
}

Store& Entity::store() {
	return Store::get(id);
}

std::vector<Store*> Entity::stores() {
	std::vector<Store*> stores;
	if (isGhost()) {
		stores.push_back(&ghost().store);
	}
	if (spec->consumeChemical) {
		stores.push_back(&burner().store);
	}
	if (spec->store) {
		stores.push_back(&store());
	}
	return stores;
}

Crafter& Entity::crafter() {
	return Crafter::get(id);
}

Vehicle& Entity::vehicle() {
	return Vehicle::get(id);
}

Arm& Entity::arm() {
	return Arm::get(id);
}

Belt& Entity::belt() {
	return Belt::get(id);
}

Lift& Entity::lift() {
	return Lift::get(id);
}

Drone& Entity::drone() {
	return Drone::get(id);
}

Depot& Entity::depot() {
	return Depot::get(id);
}

Burner& Entity::burner() {
	return Burner::get(id);
}
