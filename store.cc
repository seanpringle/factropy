#include "common.h"
#include "store.h"
#include "spec.h"
#include "sim.h"

void Store::reset() {
	all.clear();
}

void Store::tick() {

}

Store& Store::create(uint id) {
	Store& store = all.ref(id);
	store.id = id;
	store.activity = 0;
	return store;
}

Store& Store::get(uint id) {
	ensuref(all.has(id), "invalid store access %d", id);
	return all.ref(id);
}

void Store::destroy() {
	stacks.clear();
	all.drop(id);
}

Stack Store::insert(Stack istack) {
	for (auto it = stacks.begin(); it != stacks.end(); it++) {
		if (it->iid == istack.iid) {
			it->size += istack.size;
			istack.size = 0;
			break;
		}
	}
	if (istack.size > 0) {
		stacks.push_back(istack);
		istack.size = 0;
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
			return rstack;
		}
	}
	rstack.size = 0;
	return rstack;
}

Stack Store::removeAny(uint size) {
	for (auto it = stacks.begin(); it != stacks.end(); it++) {
		return remove({it->iid, size});
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

void Store::clearLevels() {
	levels.clear();
}

void Store::levelSet(uint iid, uint lower, uint upper) {
	Level *lvl = level(iid);
	if (lvl) {
		lvl->lower = lower;
		lvl->upper = upper;
		return;
	}
	levels.push_back({
		iid: iid,
		lower: lower,
		upper: upper,
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
	return false;
}

Mass Store::limit() {
	return 1000;
}

Mass Store::usage() {
	return 0;
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
	return lvl ? n - lvl->reserved + lvl->promised: n;
}

uint Store::countAvailable(uint iid) {
	uint n = count(iid);
	Level* lvl = level(iid);
	return lvl ? n - lvl->reserved: n;
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
	return {0,0};
}

// any provider or overflow to requester
Stack Store::supplyFrom(Store& src) {
	for (Level& dl: levels) {
		if (isRequesting(dl.iid) && src.isProviding(dl.iid)) {
			return {dl.iid, 1};
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

// active provider to default overflow
Stack Store::overflowDefaulTo(Store& dst) {
	for (Level& sl: levels) {
		if (dst.isOverflowDefault(sl.iid) && isActiveProviding(sl.iid)) {
			return {sl.iid, 1};
		}
	}
	return {0,0};
}

