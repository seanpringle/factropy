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

void Arm::update() {
	Entity& en = Entity::get(id);
	if (en.isGhost()) return;
	if (pause > Sim::tick) return;

	switch (stage) {
		case Input: {

			uint inputId = Entity::at(input());

			if (inputId) {
				Entity& ei = Entity::get(inputId);

				if (ei.spec->store) {
					Stack stack = ei.store().removeAny(1);
					iid = stack.iid;
					stage = iid > 0 ? ToOutput: Input;
					break;
				}

				if (ei.spec->belt) {
					iid = ei.belt().removeAny();
					stage = iid > 0 ? ToOutput: Input;
					break;
				}
			}

			pause = Sim::tick+30;
			break;
		}

		case ToInput: {
			orientation = std::min(1.0f, orientation+0.01f);
			if (std::abs(orientation-1.0f) < 0.01f) {
				orientation = 0.0f;
				stage = Input;
			}
			break;
		}

		case Output: {

			uint outputId = Entity::at(output());

			if (outputId) {
				Entity& eo = Entity::get(outputId);

				if (eo.spec->store) {
					if (eo.store().insert({iid,1}).size == 0) {
						iid = 0;
						stage = ToInput;
					}
					break;
				}

				if (eo.spec->belt) {
					if (eo.belt().insert(iid)) {
						iid = 0;
						stage = ToInput;
					}
					break;
				}
			}

			pause = Sim::tick+30;
			break;
		}

		case ToOutput: {
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