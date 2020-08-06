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
	Arm& arm = all[id];
	arm.id = id;
	arm.iid = 0;
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
	return en.pos.floor(0.5f) - en.dir;
}

Point Arm::output() {
	Entity& en = Entity::get(id);
	return en.pos.floor(0.5f) + en.dir;
}

void Arm::updateInput() {
	uint inputId = Entity::at(input());
	uint outputId = Entity::at(output());

	if (inputId && outputId) {

		Entity& ei = Entity::get(inputId);
		Entity& eo = Entity::get(outputId);

		for (Store* si: ei.stores()) {
			for (Store* so: eo.stores()) {
				Stack stack = so->supplyFrom(*si);
				if (!stack.iid || !stack.size) {
					stack = si->overflowTo(*so);
				}
				if (stack.iid && stack.size) {
					si->remove({stack.iid,1});
					iid = stack.iid;
					stage = ToOutput;
					return;
				}
			}
		}

		if (eo.spec->belt) {
			for (Store* si: ei.stores()) {
				Stack stack = si->removeAny(1);
				if (stack.iid && stack.size) {
					si->remove({stack.iid,1});
					iid = stack.iid;
					stage = ToOutput;
					return;
				}
			}
		}

		if (ei.spec->belt) {
			uint biid = ei.belt().itemAt(BeltAny);
			if (biid) {
				for (Store* so: eo.stores()) {
					if (so->isAccepting(biid)) {
						ei.belt().remove(biid, BeltAny);
						iid = biid;
						stage = ToOutput;
						return;
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
				return;
			}
		}
	}

	pause = Sim::tick+10;
}

void Arm::updateOutput() {
	uint outputId = Entity::at(output());

	if (outputId) {
		Entity& eo = Entity::get(outputId);

		for (Store* so: eo.stores()) {
			if (so->insert({iid,1}).size == 0) {
				iid = 0;
				stage = ToInput;
				return;
			}
		}

		if (eo.spec->belt) {
			if (eo.belt().insert(iid, BeltAny)) {
				iid = 0;
				stage = ToInput;
				return;
			}
		}
	}

	pause = Sim::tick+10;
}

void Arm::update() {
	Entity& en = Entity::get(id);
	if (en.isGhost()) return;
	if (pause > Sim::tick) return;

	switch (stage) {
		case Input: {
			updateInput();
			break;
		}

		case ToInput: {
			en.consume(en.spec->energyConsume);
			orientation = std::min(1.0f, orientation+0.01f);
			if (std::abs(orientation-1.0f) < 0.01f) {
				orientation = 0.0f;
				stage = Input;
			}
			break;
		}

		case Output: {
			updateOutput();
			break;
		}

		case ToOutput: {
			en.consume(en.spec->energyConsume);
			orientation = std::min(0.5f, orientation+0.01f);
			if (std::abs(orientation-0.5f) < 0.01) {
				orientation = 0.5f;
				stage = Output;
			}
			break;
		}
	}

	en.state = (uint)std::floor(orientation*360.f);
}
