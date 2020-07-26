#include "common.h"
#include "arm.h"

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

void Arm::update() {
	Entity& en = Entity::get(id);
	if (en.isGhost()) return;

	switch (stage) {
		case Input: {
			stage = ToOutput;
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
			stage = ToInput;
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