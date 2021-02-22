#include "common.h"
#include "store.h"
#include "entity.h"
#include "sim.h"

void Store::reset() {
	all.clear();
}

void Store::tick() {
	for (Store& store: all) {
		store.update();
	}
	for (auto& burner: Burner::all) {
		burner.store.update();
	}
	for (auto& ghost: Ghost::all) {
		ghost.store.update();
	}
}

void Store::update() {
	for (Level& lvl: levels) {
		lvl.promised = 0;
		lvl.reserved = 0;
		lvl.upper = std::max(lvl.lower, lvl.upper);
	}

	std::vector<uint> remove;

	for (uint did: drones) {
		if (!Entity::exists(did)) {
			remove.push_back(did);
			continue;
		}

		Entity& de = Entity::get(did);

		if (de.drone().src == id) {
			reserve(de.drone().stack);
		}

		if (de.drone().dst == id) {
			promise(de.drone().stack);
		}
	}

	for (uint did: remove) {
		drones.erase(did);
	}

	remove.clear();

	for (uint aid: arms) {
		if (!Entity::exists(aid)) {
			remove.push_back(aid);
			continue;
		}

		Entity& ae = Entity::get(aid);
		Arm& arm = ae.arm();

		if (arm.inputId != id && arm.outputId != id) {
			remove.push_back(aid);
			continue;
		}

		if (arm.inputId == id) {
			reserve({arm.iid,1});
		}

		if (arm.outputId == id) {
			promise({arm.iid,1});
		}
	}

	for (uint aid: remove) {
		arms.erase(aid);
	}
}

Store& Store::create(uint id, uint sid, Mass cap) {
	Store& store = all[id];
	store.id = id;
	store.sid = sid;
	store.activity = 0;
	store.capacity = cap;
	store.fuel = false;
	return store;
}

void Store::ghostInit(uint gid, uint gsid) {
	id = gid;
	sid = gsid;
	activity = 0;
	capacity = Mass::Inf;
	fuel = false;
}

void Store::burnerInit(uint bid, uint bsid, Mass cap) {
	id = bid;
	sid = bsid;
	activity = 0;
	capacity = cap;
	fuel = true;
	fuelCategory = "chemical";
}

Store& Store::get(uint id) {
	ensuref(all.has(id), "invalid store access %d", id);
	return all[id];
}

void Store::destroy() {
	stacks.clear();
	all.erase(id);
}

void Store::ghostDestroy() {
	stacks.clear();
}

void Store::burnerDestroy() {
	stacks.clear();
}

Stack Store::insert(Stack istack) {
	Mass space = limit() - usage();
	uint count = std::min(istack.size, space.items(istack.iid));

	if (count > 0) {
		activity = Sim::tick;
	}

	for (auto it = stacks.begin(); it != stacks.end(); it++) {
		if (it->iid == istack.iid) {
			it->size += count;
			istack.size -= count;
			count = 0;
			break;
		}
	}
	if (count > 0) {
		stacks.push_back({istack.iid, count});
		istack.size -= count;
	}
	return istack;
}

Stack Store::remove(Stack rstack) {
	for (auto it = stacks.begin(); it != stacks.end(); it++) {
		if (it->iid == rstack.iid) {
			if (it->size <= rstack.size) {
				rstack.size = it->size;
				stacks.erase(it);
			} else {
				it->size -= rstack.size;
			}
			activity = Sim::tick;
			return rstack;
		}
	}
	rstack.size = 0;
	return rstack;
}

uint Store::wouldRemoveAny() {
	for (auto it = stacks.begin(); it != stacks.end(); it++) {
		if (isActiveProviding(it->iid)) {
			return it->iid;
		}
	}
	for (auto it = stacks.begin(); it != stacks.end(); it++) {
		if (isProviding(it->iid)) {
			return it->iid;
		}
	}
	for (auto it = stacks.begin(); it != stacks.end(); it++) {
		if (fuel && Item::get(it->iid)->fuel.category == fuelCategory) {
			continue;
		}
		Level *lvl = level(it->iid);
		if (!lvl) {
			return it->iid;
		}
	}
	return 0;
}

uint Store::wouldRemoveAny(std::set<uint>& filter) {
	for (auto it = stacks.begin(); it != stacks.end(); it++) {
		if (filter.count(it->iid) && isActiveProviding(it->iid)) {
			return it->iid;
		}
	}
	for (auto it = stacks.begin(); it != stacks.end(); it++) {
		if (filter.count(it->iid) && isProviding(it->iid)) {
			return it->iid;
		}
	}
	for (auto it = stacks.begin(); it != stacks.end(); it++) {
		if (!filter.count(it->iid)) {
			continue;
		}
		if (fuel && Item::get(it->iid)->fuel.category == fuelCategory) {
			continue;
		}
		Level *lvl = level(it->iid);
		if (!lvl) {
			return it->iid;
		}
	}
	return 0;
}

Stack Store::removeAny(uint size) {
	uint iid = wouldRemoveAny();
	if (iid) {
		return remove({iid,size});
	}
	return {0,0};
}

Stack Store::removeFuel(std::string category, uint size) {
	for (auto it = stacks.begin(); it != stacks.end(); it++) {
		if (Item::get(it->iid)->fuel.category == category) {
			return remove({it->iid, std::min(size, it->size)});
		}
	}
	return {0,0};
}

Stack Store::overflowAny(uint size) {
	for (auto it = stacks.begin(); it != stacks.end(); it++) {
		if (isActiveProviding(it->iid)) {
			return remove({it->iid, size});
		}
	}
	return {0,0};
}

void Store::promise(Stack stack) {
	activity = Sim::tick;
	Level *lvl = level(stack.iid);
	if (lvl != NULL) {
		lvl->promised += stack.size;
	}
}

void Store::reserve(Stack stack) {
	activity = Sim::tick;
	Level *lvl = level(stack.iid);
	if (lvl != NULL) {
		lvl->reserved += stack.size;
	}
}

void Store::levelSet(uint iid, uint lower, uint upper) {
	Level *lvl = level(iid);
	if (lvl) {
		lvl->lower = lower;
		lvl->upper = upper;
		return;
	}
	levels.push_back({
		.iid = iid,
		.lower = lower,
		.upper = upper,
	});
}

void Store::levelClear(uint iid) {
	for (auto it = levels.begin(); it != levels.end(); it++) {
		if (it->iid == iid) {
			levels.erase(it);
			break;
		}
	}
}

Store::Level* Store::level(uint iid) {
	for (auto it = levels.begin(); it != levels.end(); it++) {
		if (it->iid == iid) {
			return &*it;
		}
	}
	return NULL;
}

bool Store::isEmpty() {
	return stacks.size() == 0;
}

bool Store::isFull() {
	return usage() >= limit();
}

Mass Store::limit() {
	return capacity;
}

Mass Store::usage() {
	Mass m = 0;
	for (Stack stack: stacks) {
		m += Item::get(stack.iid)->mass * stack.size;
	}
	return m;
}

uint Store::count(uint iid) {
	for (auto it = stacks.begin(); it != stacks.end(); it++) {
		if (it->iid == iid) {
			return it->size;
		}
	}
	return 0;
}

uint Store::countNet(uint iid) {
	uint n = count(iid);
	Level* lvl = level(iid);
	return lvl ? n - std::min(n, lvl->reserved) + lvl->promised: n;
}

uint Store::countAvailable(uint iid) {
	uint n = count(iid);
	Level* lvl = level(iid);
	return lvl ? n - std::min(n, lvl->reserved): n;
}

uint Store::countExpected(uint iid) {
	uint n = count(iid);
	Level* lvl = level(iid);
	return lvl ? n + lvl->promised: n;
}

//                lower                upper
// |----------------|--------------------|-------------------------
//     requester           provider            active provider
//     overflow            overflow              no overflow

// store has all lower limits met
bool Store::isRequesterSatisfied() {
	for (Level& lvl: levels) {
		if (lvl.lower > 0 && count(lvl.iid) < lvl.lower) {
			return false;
		}
	}
	return true;
}

// store has a lower limit for item not met
bool Store::isRequesting(uint iid) {
	Level* lvl = level(iid);
	return lvl != NULL && lvl->lower > 0 && countExpected(iid) < lvl->lower;
}

// store has a lower limit for item exceeded
bool Store::isProviding(uint iid) {
	Level *lvl = level(iid);
	return lvl != NULL && lvl->upper >= lvl->lower && countAvailable(lvl->iid) > lvl->lower;
}

// store has an upper limit for item exceeded
bool Store::isActiveProviding(uint iid) {
	Level *lvl = level(iid);
	return lvl != NULL && lvl->upper >= lvl->lower && countAvailable(lvl->iid) > lvl->upper;
}

// store has an upper limit for an item not exceeded
bool Store::isAccepting(uint iid) {
	if (isFull()) {
		return false;
	}
	if (isRequesting(iid)) {
		return true;
	}
	Level *lvl = level(iid);
	if (lvl != NULL && countExpected(iid) < lvl->upper) {
		return true;
	}
	if (fuel && Item::get(iid)->fuel.category == fuelCategory) {
		return true;
	}
	return false;
}

// store is a logistic overflow accepting anything
bool Store::isOverflowDefault(uint iid) {
	Spec* spec = Entity::get(id).spec;
	return spec->logistic && spec->defaultOverflow && !isFull();
}

// requester to another requester
Stack Store::forceSupplyFrom(Store& src) {
	for (Level& dl: levels) {
		if (isRequesting(dl.iid) && src.countAvailable(dl.iid) > 0) {
			return {dl.iid, 1};
		}
	}
	if (fuel && !isFull()) {
		for (Stack& ss: src.stacks) {
			if (Item::get(ss.iid)->fuel.category == fuelCategory) {
				return {ss.iid, 1};
			}
		}
	}
	return {0,0};
}

Stack Store::forceSupplyFrom(Store& src, std::set<uint>& filter) {
	for (Level& dl: levels) {
		if (filter.count(dl.iid) && isRequesting(dl.iid) && src.countAvailable(dl.iid) > 0) {
			return {dl.iid, 1};
		}
	}
	if (fuel && !isFull()) {
		for (Stack& ss: src.stacks) {
			if (filter.count(ss.iid) && Item::get(ss.iid)->fuel.category == fuelCategory) {
				return {ss.iid, 1};
			}
		}
	}
	return {0,0};
}

// any provider or overflow to requester
Stack Store::supplyFrom(Store& src) {
	for (Level& dl: levels) {
		if (isRequesting(dl.iid) && src.isProviding(dl.iid)) {
			return {dl.iid, 1};
		}
	}
	if (fuel && !isFull()) {
		for (Stack& ss: src.stacks) {
			if (Item::get(ss.iid)->fuel.category == fuelCategory && src.isProviding(ss.iid)) {
				return {ss.iid, 1};
			}
		}
	}
	return {0,0};
}

Stack Store::supplyFrom(Store& src, std::set<uint>& filter) {
	for (Level& dl: levels) {
		if (filter.count(dl.iid) && isRequesting(dl.iid) && src.isProviding(dl.iid)) {
			return {dl.iid, 1};
		}
	}
	if (fuel && !isFull()) {
		for (Stack& ss: src.stacks) {
			if (filter.count(ss.iid) && Item::get(ss.iid)->fuel.category == fuelCategory && src.isProviding(ss.iid)) {
				return {ss.iid, 1};
			}
		}
	}
	return {0,0};
}

// active provider to anything
Stack Store::forceOverflowTo(Store& dst) {
	if (!dst.isFull()) {
		for (Level& sl: levels) {
			if (isActiveProviding(sl.iid)) {
				return {sl.iid, 1};
			}
		}
	}
	return {0,0};
}

// active provider to overflow with space
Stack Store::overflowTo(Store& dst) {
	for (Level& sl: levels) {
		if (dst.isAccepting(sl.iid) && isActiveProviding(sl.iid)) {
			return {sl.iid, 1};
		}
	}
	return {0,0};
}

Stack Store::overflowTo(Store& dst, std::set<uint>& filter) {
	for (Level& sl: levels) {
		if (filter.count(sl.iid) && dst.isAccepting(sl.iid) && isActiveProviding(sl.iid)) {
			return {sl.iid, 1};
		}
	}
	return {0,0};
}

// active provider to default overflow
Stack Store::overflowDefaultTo(Store& dst) {
	for (Level& sl: levels) {
		if (dst.isOverflowDefault(sl.iid) && isActiveProviding(sl.iid)) {
			return {sl.iid, 1};
		}
	}
	return {0,0};
}
