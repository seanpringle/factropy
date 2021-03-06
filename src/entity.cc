#include "common.h"
#include "item.h"
#include "entity.h"
#include "chunk.h"

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

	for (auto& pair: Spec::all) {
		auto& spec = pair.second;
		spec->statsGroup->energyConsumption.set(Sim::tick, 0);
	}

	for (auto [eid,consumption]: energyConsumers) {
		Entity& en = Entity::get(eid);
		if (en.isEnabled() && en.spec->energyDrain && !consumption) {
			consumption = en.consume(en.spec->energyDrain);
		}
		en.spec->statsGroup->energyConsumption.add(Sim::tick, consumption);
		energyConsumers[eid] = 0;
	}

	for (auto& pair: Spec::all) {
		auto& spec = pair.second;
		spec->statsGroup->energyConsumption.update(Sim::tick);
	}
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
	Entity& en = all[id];
	en.id = id;
	en.spec = spec;
	en.dir = Point::South;
	en.state = 0;
	en.health = spec->health;

	en.flags = GHOST | ENABLED;
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

	if (spec->unveyor) {
		Unveyor::create(id);
	}

	if (spec->loader) {
		Loader::create(id);
	}

	if (spec->ropeway) {
		Ropeway::create(id);
	}

	if (spec->ropewayBucket) {
		RopewayBucket::create(id);
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

	if (spec->energyConsume || spec->energyDrain) {
		energyConsumers[id] = 0;
	}

	if (spec->consumeChemical) {
		Burner::create(id, Entity::next());
	}

	if (spec->consumeThermalFluid) {
		Generator::create(id, Entity::next());
	}

	if (spec->generateElectricity) {
		electricityGenerators.insert(id);
		en.setGenerating(true);
	}

	if (spec->named) {
		names[id] = fmt("%s %u", spec->name, id);
	}

	en.pos = {0,0,0};
	en.move((Point){0,0,0});

	return en;
}

void Entity::destroy() {
	unmanage();
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

	if (spec->unveyor) {
		unveyor().destroy();
	}

	if (spec->loader) {
		loader().destroy();
	}

	if (spec->ropeway) {
		ropeway().destroy();
	}

	if (spec->ropewayBucket) {
		ropewayBucket().destroy();
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

	if (spec->energyConsume || spec->energyDrain) {
		energyConsumers.erase(id);
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
	all.erase(id);
}

bool Entity::exists(uint id) {
	return id > 0 && all.has(id);
}

Entity& Entity::get(uint id) {
	return all.refer(id);
}

bool Entity::fits(Spec *spec, Point pos, Point dir) {
	Box bounds = spec->box(pos, dir, spec->collision).shrink(0.1);

	for (auto cid: intersecting(bounds)) {
		if (get(cid).spec->collideBuild) return false;
	}

	if (spec->place != Spec::Footings) {
		for (auto [x,y]: Chunk::walkTiles(bounds)) {
			Chunk::Tile *tile = Chunk::tileTryGet(x, y);
			if (!tile) return false;
			bool ok = false;
			ok = ok || ((spec->place & Spec::Land) && tile->isLand());
			ok = ok || ((spec->place & Spec::Hill) && tile->isHill());
			ok = ok || ((spec->place & Spec::Water) && tile->isWater());
			if (!ok) return false;
		}
		return true;
	}

	for (auto& footing: spec->footings) {
		Point point = footing.point.transform(dir.rotation()) + pos;
		Chunk::Tile *tile = Chunk::tileTryGet(std::floor(point.x), std::floor(point.z));

		if (!tile) return false;
		bool ok = false;
		ok = ok || ((footing.place & Spec::Land) && tile->isLand());
		ok = ok || ((footing.place & Spec::Hill) && tile->isHill());
		ok = ok || ((footing.place & Spec::Water) && tile->isWater());
		if (!ok) return false;
	}

	return true;
}

std::vector<uint> Entity::intersecting(Box box) {
	auto hits = grid.search(box);
	discard_if(hits, [&](uint id) { return !get(id).box().intersects(box); });
	return hits;
}

std::vector<uint> Entity::intersecting(Sphere sphere) {
	auto hits = grid.search(sphere);
	discard_if(hits, [&](uint id) { return !get(id).sphere().intersects(sphere); });
	return hits;
}

std::vector<uint> Entity::intersecting(Point pos, float radius) {
	auto hits = intersecting(pos.box().grow(radius));
	discard_if(hits, [&](uint id) { return !(get(id).pos.distance(pos) < radius); });
	return hits;
}

uint Entity::at(Point p) {
	auto hits = intersecting(p.box());
	return hits.size() ? hits.front(): 0;
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

bool Entity::isEnabled() {
	return (flags & ENABLED) != 0;
}

Entity& Entity::setEnabled(bool state) {
	flags = state ? (flags | ENABLED) : (flags & ~ENABLED);
	return *this;
}

bool Entity::isGenerating() {
	return (flags & GENERATING) != 0;
}

Entity& Entity::setGenerating(bool state) {
	flags = state ? (flags | GENERATING) : (flags & ~GENERATING);
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
	return spec->box(pos, dir, spec->collision);
}

Sphere Entity::sphere() {
	return box().sphere();
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
	grid.insert(box(), id);
	return *this;
}

Entity& Entity::unindex() {
	grid.remove(box(), id);
	return *this;
}

Entity& Entity::manage() {
	if (!isGhost()) {
		if (spec->conveyor) {
			conveyor().manage();
		}
		if (spec->unveyor) {
			unveyor().manage();
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
		if (spec->unveyor) {
			unveyor().unmanage();
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
	if (spec->toggle) {
		unmanage();
		//if (spec->lift) {
		//	lift().toggle();
		//}
		manage();
	}
	return *this;
}

Energy Entity::consume(Energy e) {
	Energy c = 0;
	if (!isEnabled()) return c;

	if (spec->consumeElectricity) {
		electricityDemand += e;
		c = e * electricitySatisfaction;
	}
	if (spec->consumeChemical) {
		c = burner().consume(e);
	}
	if (spec->consumeThermalFluid) {
		c = generator().consume(e);
	}
	if (energyConsumers.count(id)) {
		energyConsumers[id] += c;
	}
	return c;
}

float Entity::consumeRate(Energy e) {
	return consume(e).portion(e);
}

void Entity::generate() {
	if (!isEnabled() || !isGenerating() || !electricityGenerators.count(id)) return;

	// At the start of the game when a single generator exists, or when the electricity network
	// fuel supply crashes and generators need to restart, load must always be slightly > 0.
	Energy energy = std::max(Energy::J(1), spec->energyGenerate * electricityLoad);

	if (spec->generateElectricity && spec->consumeChemical) {

		electricitySupply += burner().consume(energy);
		if (burner().energy) electricityCapacityReady += spec->energyGenerate;
		electricityCapacity += spec->energyGenerate;

		state = std::floor((float)burner().energy.value/(float)burner().buffer.value * (float)spec->states.size());
		state = std::min((uint)spec->states.size(), std::max((uint)1, state)) - 1;
		return;
	}

	if (spec->generateElectricity && spec->consumeThermalFluid) {

		Energy supplied = generator().consume(energy);
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

Unveyor& Entity::unveyor() {
	return Unveyor::get(id);
}

Loader& Entity::loader() {
	return Loader::get(id);
}

Ropeway& Entity::ropeway() {
	return Ropeway::get(id);
}

RopewayBucket& Entity::ropewayBucket() {
	return RopewayBucket::get(id);
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

