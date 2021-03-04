#include "common.h"
#include "sim.h"
#include "loader.h"

void Loader::reset() {
	all.clear();
}

void Loader::tick() {
	for (auto& loader: all) {
		loader.update();
	}
}

Loader& Loader::create(uint id) {
	Entity& en = Entity::get(id);
	Loader& loader = all[id];
	loader.id = id;
	loader.pause = 0;
	loader.loading = !en.spec->loaderUnload;
	return loader;
}

Loader& Loader::get(uint id) {
	return all.refer(id);
}

void Loader::destroy() {
	all.erase(id);
}

Point Loader::point() {
	Entity& en = Entity::get(id);
	return en.spec->loaderPoint
		.transform(en.dir.rotation())
		.transform(en.pos.translation())
	;
}

Stack Loader::transferBeltToStore(Store& dst, Stack stack) {
	Entity& de = Entity::get(dst.id);

	if (de.isGhost()) {
		return {0,0};
	}

	if (filters.size() && filters.count(stack.iid) && dst.isAccepting(stack.iid)) {
		return {stack.iid, std::min(stack.size, dst.countAcceptable(stack.iid))};
	}

	if (dst.isAccepting(stack.iid)) {
		return {stack.iid, std::min(stack.size, dst.countAcceptable(stack.iid))};
	}

	return {0,0};
}

Stack Loader::transferStoreToBelt(Store& src) {
	Entity& se = Entity::get(src.id);

	if (se.isGhost()) {
		return {0,0};
	}

	if (filters.size()) {
		Stack stack = {src.wouldRemoveAny(filters),1};
		if (!stack.iid) {
			for (Stack& ss: src.stacks) {
				if (filters.count(ss.iid) && src.isActiveProviding(ss.iid)) {
					return {ss.iid, 1};
				}
			}
			for (Stack& ss: src.stacks) {
				if (filters.count(ss.iid) && src.isProviding(ss.iid)) {
					return {ss.iid, 1};
				}
			}
		}
		return stack;
	}

	Stack stack = {src.wouldRemoveAny(),1};
	if (!stack.iid) {
		for (Stack& ss: src.stacks) {
			if (src.isActiveProviding(ss.iid)) {
				return {ss.iid, 1};
			}
		}
		for (Stack& ss: src.stacks) {
			if (src.isProviding(ss.iid)) {
				return {ss.iid, 1};
			}
		}
	}
	return stack;
}

void Loader::update() {
	Entity& en = Entity::get(id);
	if (en.isGhost()) return;
	if (!en.isEnabled()) return;
	if (pause > Sim::tick) return;

	auto& conveyor = Conveyor::get(id);

	if (loading && conveyor.iid && conveyor.offset != conveyor.steps/2) {
		pause = Sim::tick + (conveyor.offset - conveyor.steps/2) - 1;
		return;
	}

	if (loading && !conveyor.iid) {
		pause = Sim::tick + conveyor.steps/2;
		return;
	}

	if (!loading && conveyor.iid) {
		pause = Sim::tick + conveyor.offset;
		return;
	}

	Box targetArea = point().box().grow(0.1f);

	if (!storeId || !Entity::exists(storeId) || !Entity::get(storeId).box().intersects(targetArea)) {
		storeId = 0;
		for (auto sid: Entity::intersecting(targetArea)) {
			auto& es = Entity::get(sid);
			if (es.spec->store) {
				storeId = sid;
				break;
			}
		}
	}

	if (!storeId) {
		pause = Sim::tick + 30;
		return;
	}

	auto& store = Store::get(storeId);

	if (loading && conveyor.iid && conveyor.offset == conveyor.steps/2) {
		Stack stack = transferBeltToStore(store, {conveyor.iid,1});
		if (stack.iid == conveyor.iid && stack.size && store.insert({stack.iid,1}).size == 0) {
			conveyor.remove(stack.iid);
		}
		return;
	}

	if (!loading && !conveyor.iid) {
		Stack stack = transferStoreToBelt(store);
		if (stack.iid && stack.size && conveyor.deliver(stack.iid)) {
			store.remove(stack);
		}
		return;
	}
}
