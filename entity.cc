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

	for (auto id: exploding) {
		Entity& en = Entity::get(id);
		ensuref(en.spec->explodes, "%s cannot explode", en.spec->name);

		Entity& ex = Entity::create(Entity::next(), Spec::byName(en.spec->explosionSpec));
		ex.explosion().define(ex.spec->explosionDamage, ex.spec->explosionRadius, ex.spec->explosionRate);
		ex.move(en.pos);
		ex.materialize();

		removing.insert(id);
	}
	exploding.clear();

	for (auto id: removing) {
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
	en.health = spec->health;

	en.flags = GHOST;
	Ghost::create(id, Entity::next());

	if (spec->store) {
		Store::create(id, Entity::next(), spec->capacity);
	}

	if (spec->crafter) {
		Crafter::create(id);
	}

	if (spec->projector) {
		Projector::create(id);
	}

	if (spec->drone) {
		Drone::create(id);
	}

	if (spec->missile) {
		Missile::create(id);
	}

	if (spec->explosion) {
		Explosion::create(id);
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

	if (spec->conveyor) {
		Conveyor::create(id);
	}

	if (spec->lift) {
		Lift::create(id);
	}

	if (spec->pipe) {
		Pipe::create(id);
	}

	if (spec->turret) {
		Turret::create(id);
	}

	if (spec->computer) {
		Computer::create(id);
	}

	if (spec->consumeChemical) {
		Burner::create(id, Entity::next());
	}

	if (spec->consumeThermalFluid) {
		Generator::create(id, Entity::next());
	}

	if (spec->consumeElectricity) {
		electricityConsumers.insert(id);
	}

	if (spec->generateElectricity) {
		electricityGenerators.insert(id);
	}

	if (spec->named) {
		names[id] = fmt("%s %u", spec->name, id);
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

	if (spec->projector) {
		projector().destroy();
	}

	if (spec->vehicle) {
		vehicle().destroy();
	}

	if (spec->drone) {
		drone().destroy();
	}

	if (spec->missile) {
		missile().destroy();
	}

	if (spec->explosion) {
		explosion().destroy();
	}

	if (spec->depot) {
		depot().destroy();
	}

	if (spec->arm) {
		arm().destroy();
	}

	if (spec->conveyor) {
		conveyor().destroy();
	}

	if (spec->lift) {
		lift().destroy();
	}

	if (spec->pipe) {
		pipe().destroy();
	}

	if (spec->turret) {
		turret().destroy();
	}

	if (spec->computer) {
		computer().destroy();
	}

	if (spec->consumeElectricity) {
		electricityConsumers.erase(id);
	}

	if (spec->consumeChemical) {
		burner().destroy();
	}

	if (spec->consumeThermalFluid) {
		generator().destroy();
	}

	if (spec->generateElectricity) {
		electricityGenerators.erase(id);
	}

	names.erase(id);
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

	if (intersecting(bounds).size()) return false;

	switch (spec->place) {
		case Spec::Land: {

			if (spec->supportPoints.size() > 0) {
				for (auto p: spec->relativePoints(spec->supportPoints, dir.rotation(), pos)) {
					bool supported = p.y < 0 && p.y > -1;

					if (!supported) {
						for (auto bid: intersecting(p.box().grow(0.1))) {
							Entity& eb = get(bid);
							if (eb.spec->block) {
								supported = true;
								break;
							}
						}
					}

					if (!supported) {
						return false;
					}
				}
			}

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

	return true;
}

std::vector<uint> Entity::intersecting(Box box) {
	std::vector<uint> hits;
	for (auto xy: Chunk::walk(box)) {
		for (uint id: grid[xy]) {
			if (get(id).box().intersects(box)) {
				hits.push_back(id);
			}
		}
	}
	deduplicate(hits);
	return hits;
}

std::vector<uint> Entity::intersecting(Point pos, float radius) {
	std::vector<uint> hits;
	for (auto id: intersecting(pos.box().grow(radius))) {
		Entity& en = get(id);
		if (en.pos.distance(pos) < radius) {
			hits.push_back(id);
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

std::vector<uint> Entity::enemiesInRange(Point pos, float radius) {
	std::vector<uint>hits;
	for (auto& pair: Missile::all) {
		Missile& missile = pair.second;
		Entity& me = Entity::get(missile.id);
		float de = me.pos.distance(pos);

		if (me.spec->name == "bullet") continue;

		if (de < radius) hits.push_back(me.id);
	}
	return hits;
}

void Entity::remove() {
	removing.insert(id);
}

void Entity::explode() {
	exploding.insert(id);
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

std::string Entity::name() {
	return spec->named ? names[id]: fmt("%s %u", spec->name, id);
}

bool Entity::rename(std::string name) {
	if (spec->named) {
		names[id] = name;
		return true;
	}
	return false;
}

Box Entity::box() {
	return spec->box(pos, dir);
}

Box Entity::miningBox() {
	return box().grow(0.5f);
}

Entity& Entity::look(Point p) {
	unmanage();
	unindex();
	dir = p.normalize();
	index();
	manage();
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
	//unindex();
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
		if (spec->conveyor) {
			conveyor().manage();
		}
		if (spec->pipe) {
			pipe().manage();
		}
	}
	return *this;
}

Entity& Entity::unmanage() {
	if (!isGhost()) {
		if (spec->conveyor) {
			conveyor().unmanage();
		}
		if (spec->pipe) {
			pipe().unmanage();
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
	if (spec->consumeThermalFluid) {
		return generator().consume(e);
	}
	return 0;
}

float Entity::consumeRate(Energy e) {
	return consume(e).portion(e);
}

void Entity::generate() {
	if (spec->generateElectricity && spec->consumeChemical) {

		electricitySupply += burner().consume(spec->energyGenerate * electricityLoad);
		if (burner().energy) electricityCapacityReady += spec->energyGenerate;
		electricityCapacity += spec->energyGenerate;

		state = std::floor((float)burner().energy.value/(float)burner().buffer.value * (float)spec->states.size());
		state = std::min((uint)spec->states.size(), std::max((uint)1, state)) - 1;
		return;
	}

	if (spec->generateElectricity && spec->consumeThermalFluid) {

		Energy supplied = generator().consume(spec->energyGenerate * electricityLoad);
		electricitySupply += supplied;

		if (generator().supplying) electricityCapacityReady += spec->energyGenerate;
		electricityCapacity += spec->energyGenerate;

		if (supplied) {
			state += supplied > (spec->energyGenerate * 0.5f) ? 2: 1;
			if (state >= spec->states.size()) state -= spec->states.size();
		}

		return;
	}
}

void Entity::damage(Health hits) {
	if (!spec->health) return;

	health -= hits;
	if (health < 0) {
		unmanage();
		if (spec->explodes) explode(); else remove();
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

Projector& Entity::projector() {
	return Projector::get(id);
}

Vehicle& Entity::vehicle() {
	return Vehicle::get(id);
}

Arm& Entity::arm() {
	return Arm::get(id);
}

Conveyor& Entity::conveyor() {
	return Conveyor::get(id);
}

Lift& Entity::lift() {
	return Lift::get(id);
}

Pipe& Entity::pipe() {
	return Pipe::get(id);
}

Drone& Entity::drone() {
	return Drone::get(id);
}

Missile& Entity::missile() {
	return Missile::get(id);
}

Explosion& Entity::explosion() {
	return Explosion::get(id);
}

Depot& Entity::depot() {
	return Depot::get(id);
}

Burner& Entity::burner() {
	return Burner::get(id);
}

Generator& Entity::generator() {
	return Generator::get(id);
}

Turret& Entity::turret() {
	return Turret::get(id);
}

Computer& Entity::computer() {
	return Computer::get(id);
}

