#include "common.h"
#include "arm.h"
#include "sim.h"

void Arm::reset() {
	all.clear();
}

void Arm::tick() {
	for (auto& pair: all) {
		pair.second.update();
	}
}

Arm& Arm::create(uint id) {
	ensuref(!all.count(id), "double-create arm %d", id);
	Arm& arm = all[id];
	arm.id = id;
	arm.iid = 0;
	arm.inputId = 0;
	arm.outputId = 0;
	arm.stage = Input;
	arm.orientation = 0.0f;
	return arm;
}

Arm& Arm::get(uint id) {
	ensuref(all.count(id) > 0, "invalid arm access %d", id);
	return all[id];
}

void Arm::destroy() {
	all.erase(id);
}

Point Arm::input() {
	Entity& en = Entity::get(id);
	return en.ground() + (Point::Up*0.5f) - (en.dir * en.spec->armOffset);
}

Point Arm::output() {
	Entity& en = Entity::get(id);
	return en.ground() + (Point::Up*0.5f) + (en.dir * en.spec->armOffset);
}

void Arm::updateProximity() {
	if (inputId && !Entity::exists(inputId)) {
		inputId = 0;
	}

	if (inputId) {
		Entity& en = Entity::get(inputId);
		if (!en.box().contains(input())) {
			inputId = 0;
		}
	}

	if (outputId && !Entity::exists(outputId)) {
		outputId = 0;
	}

	if (outputId) {
		Entity& en = Entity::get(outputId);
		if (!en.box().contains(output())) {
			outputId = 0;
		}
	}

	if (!inputId) {
		inputId = Entity::at(input());
	}

	if (!outputId) {
		outputId = Entity::at(output());
	}
}

Stack Arm::transferStoreToStore(Store& dst, Store& src) {
	Entity& de = Entity::get(dst.id);
	Entity& se = Entity::get(src.id);

	if (de.isGhost() || se.isGhost()) {
		return {0,0};
	}

	if (filter.size()) {
		Stack stack = src.overflowTo(dst, filter);
		if (!stack.iid) {
			stack = dst.supplyFrom(src, filter);
		}
		if (!stack.iid) {
			if (de.spec->loadPriority && !se.spec->loadPriority) {
				stack = dst.forceSupplyFrom(src, filter);
			}
		}
		if (!stack.iid) {
			for (Stack& ss: src.stacks) {
				bool allow = filter.count(ss.iid) > 0;
				bool dstOk = dst.isAccepting(ss.iid) || de.spec->loadAnything;
				bool srcOk = !src.fuel && (src.isProviding(ss.iid) || se.spec->unloadAnything || src.level(ss.iid) == NULL);
				if (allow && dstOk && srcOk) {
					stack = {ss.iid,1};
					break;
				}
			}
		}
		return stack;
	}

	Stack stack = src.overflowTo(dst);
	if (!stack.iid) {
		stack = dst.supplyFrom(src);
	}
	if (!stack.iid) {
		if (de.spec->loadPriority && !se.spec->loadPriority) {
			stack = dst.forceSupplyFrom(src);
		}
	}
	if (!stack.iid) {
		for (Stack& ss: src.stacks) {
			bool dstOk = dst.isAccepting(ss.iid) || de.spec->loadAnything;
			bool srcOk = !src.fuel && (src.isProviding(ss.iid) || se.spec->unloadAnything || src.level(ss.iid) == NULL);
			if (dstOk && srcOk) {
				stack = {ss.iid,1};
				break;
			}
		}
	}
	return stack;
}

Stack Arm::transferStoreToBelt(Store& src) {
	Entity& se = Entity::get(src.id);

	if (se.isGhost()) {
		return {0,0};
	}

	if (filter.size()) {
		Stack stack = {src.wouldRemoveAny(filter),1};
		if (!stack.iid) {
			for (Stack& ss: src.stacks) {
				if (filter.count(ss.iid) && src.isActiveProviding(ss.iid)) {
					return {ss.iid, 1};
				}
			}
			for (Stack& ss: src.stacks) {
				if (filter.count(ss.iid) && src.isProviding(ss.iid)) {
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

bool Arm::updateReady() {
	updateProximity();

	if (inputId && outputId) {

		Entity& ei = Entity::get(inputId);
		Entity& eo = Entity::get(outputId);

		for (Store* si: ei.stores()) {
			for (Store* so: eo.stores()) {
				Stack stack = transferStoreToStore(*so, *si);
				if (stack.iid && stack.size) {
					return true;
				}
			}
		}

		if (eo.spec->belt) {
			for (Store* si: ei.stores()) {
				Stack stack = transferStoreToBelt(*si);
				if (stack.iid && stack.size) {
					return true;
				}
			}
		}

		if (ei.spec->belt) {
			uint biid = ei.belt().itemAt(BeltAny);
			if (biid) {
				for (Store* so: eo.stores()) {
					if (so->isAccepting(biid)) {
						return true;
					}
				}
			}
		}

		if (ei.spec->lift) {
			uint liid = ei.lift().wouldRemoveAny(input().y);
			if (liid) {
				for (Store* so: eo.stores()) {
					if (so->isAccepting(liid)) {
						return true;
					}
				}
			}
		}

		if (ei.spec->belt && eo.spec->belt) {
			uint biid = ei.belt().itemAt(BeltAny);
			if (biid) {
				return true;
			}
		}
	}

	return false;
}

bool Arm::updateInput() {
	updateProximity();

	if (inputId && outputId) {

		Entity& ei = Entity::get(inputId);
		Entity& eo = Entity::get(outputId);
		inputStoreId = 0;
		outputStoreId = 0;

		for (Store* si: ei.stores()) {
			for (Store* so: eo.stores()) {
				Stack stack = transferStoreToStore(*so, *si);
				if (stack.iid && stack.size) {
					outputStoreId = so->sid;
					so->promise({stack.iid,1});
					so->arms.insert(id);
					si->remove({stack.iid,1});
					iid = stack.iid;
					stage = ToOutput;
					return true;
				}
			}
		}

		if (eo.spec->belt) {
			for (Store* si: ei.stores()) {
				Stack stack = transferStoreToBelt(*si);
				if (stack.iid && stack.size) {
					si->remove({stack.iid,1});
					iid = stack.iid;
					stage = ToOutput;
					return true;
				}
			}
		}

		if (ei.spec->belt) {
			uint biid = ei.belt().itemAt(BeltAny);
			if (biid) {
				for (Store* so: eo.stores()) {
					if (so->isAccepting(biid)) {
						outputStoreId = so->sid;
						so->promise({biid,1});
						so->arms.insert(id);
						ei.belt().remove(biid, BeltAny);
						iid = biid;
						stage = ToOutput;
						return true;
					}
				}
			}
		}

		if (ei.spec->lift) {
			uint liid = ei.lift().wouldRemoveAny(input().y);
			if (liid) {
				for (Store* so: eo.stores()) {
					if (so->isAccepting(liid)) {
						outputStoreId = so->sid;
						so->promise({liid,1});
						so->arms.insert(id);
						ei.lift().removeAny(input().y);
						iid = liid;
						stage = ToOutput;
						return true;
					}
				}
			}
		}

		if (ei.spec->belt && eo.spec->belt) {
			uint biid = ei.belt().itemAt(BeltAny);
			if (biid) {
				ei.belt().remove(biid, BeltAny);
				iid = biid;
				stage = ToOutput;
				return true;
			}
		}
	}

	return false;
}

void Arm::updateOutput() {
	updateProximity();

	if (outputId) {
		Entity& eo = Entity::get(outputId);

		if (eo.spec->store || eo.spec->consumeChemical) {
			for (Store* so: eo.stores()) {
				if (so->sid == outputStoreId || eo.spec->loadAnything) {
					if (so->insert({iid,1}).size == 0) {
						so->arms.erase(id);
						break;
					}
				}
			}
			outputStoreId = 0;
			iid = 0;
			stage = ToInput;
			return;
		}

		if (eo.spec->belt) {
			if (eo.belt().insert(iid, BeltAny)) {
				iid = 0;
				stage = ToInput;
				return;
			}
		}
	}

	pause = Sim::tick+5;
}

// Expected states:
// 0-359: rotation
// 360-?: parking

void Arm::update() {
	Entity& en = Entity::get(id);
	if (en.isGhost()) return;
	if (pause > Sim::tick) return;

	uint maxState = en.spec->states.size()-1;
	ensure(maxState > 360);

	switch (stage) {
		case Parked: {
			en.state = maxState;
			if (updateReady()) {
				stage = Unparking;
			} else {
				pause = Sim::tick+10;
			}
			break;
		}

		case Parking: {
			if (en.state >= maxState) {
				en.state = maxState;
				stage = Parked;
			} else {
				en.state++;
			}
			break;
		}

		case Unparking: {
			if (en.state <= 360) {
				en.state = 0;
				stage = Input;
			} else {
				en.state--;
			}
			break;
		}

		case Input: {
			if (!updateInput()) {
				stage = Parking;
				en.state = 360;
			}
			break;
		}

		case ToInput: {
			float speed = std::max(en.consumeRate(en.spec->energyConsume) * en.spec->armSpeed, 0.001f);

			orientation = std::min(1.0f, orientation+speed);
			if (std::abs(orientation-1.0f) < en.spec->armSpeed) {
				orientation = 0.0f;
				stage = Input;
			}
			en.state = (uint)std::floor(orientation*360.f);
			break;
		}

		case Output: {
			updateOutput();
			en.state = (uint)std::floor(orientation*360.f);
			break;
		}

		case ToOutput: {
			float speed = std::max(en.consumeRate(en.spec->energyConsume) * en.spec->armSpeed, 0.001f);

			orientation = std::min(0.5f, orientation+speed);
			if (std::abs(orientation-0.5f) < en.spec->armSpeed) {
				orientation = 0.5f;
				stage = Output;
			}
			en.state = (uint)std::floor(orientation*360.f);
			break;
		}
	}
}
