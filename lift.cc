#include "common.h"
#include "lift.h"
#include "sim.h"

void Lift::reset() {
	all.clear();
}

void Lift::tick() {
	for (auto& pair: all) {
		pair.second.update();
	}
}

Lift& Lift::create(uint id) {
	Lift& lift = all[id];
	lift.id = id;
	lift.iid = 0;
	lift.mode = Raise;
	lift.stage = Lowering;
	lift.steps = Entity::get(id).spec->states.size();
	lift.ascent = lift.steps/2;
	return lift;
}

Lift& Lift::get(uint id) {
	ensuref(all.count(id) > 0, "invalid lift access %d", id);
	return all[id];
}

void Lift::destroy() {
	all.erase(id);
}

void Lift::toggle() {
	mode = mode == Raise ? Lower: Raise;
}

bool Lift::insert(uint iiid, float level) {
	Entity& en = Entity::get(id);
	bool high = level > en.pos.y;

	if (mode == Raise && high) {
		return false;
	}
	if (mode == Lower && !high) {
		return false;
	}
	if (mode == Raise && stage != Lowered) {
		return false;
	}
	if (mode == Lower && stage != Raised) {
		return false;
	}
	if (!iid) {
		iid = iiid;
		return true;
	}
	return false;
}

bool Lift::remove(uint riid, float level) {
	Entity& en = Entity::get(id);
	bool high = level > en.pos.y;

	if (mode == Raise && !high) {
		return false;
	}
	if (mode == Lower && high) {
		return false;
	}
	if (mode == Raise && stage != Raised) {
		return false;
	}
	if (mode == Lower && stage != Lowered) {
		return false;
	}
	if (iid == riid) {
		iid = 0;
		return true;
	}
	return false;
}

uint Lift::removeAny(float level) {
	Entity& en = Entity::get(id);
	bool high = level > en.pos.y;

	if (mode == Raise && !high) {
		return false;
	}
	if (mode == Lower && high) {
		return false;
	}
	if (mode == Raise && stage != Raised) {
		return 0;
	}
	if (mode == Lower && stage != Lowered) {
		return 0;
	}
	uint riid = iid;
	iid = 0;
	return riid;
}

void Lift::updateRaise() {
	Entity& en = Entity::get(id);

	switch (stage) {
		case Lowered: { // input
			if (iid) {
				stage = Raising;
			}
			break;
		}

		case Lowering: {
			en.consume(en.spec->energyConsume);
			ascent--;
			if (ascent == 0) {
				stage = Lowered;
			}
			break;
		}

		case Raised: { // output
			if (!iid) {
				stage = Lowering;
			}
			break;
		}

		case Raising: {
			en.consume(en.spec->energyConsume);
			ascent++;
			if (ascent == steps-1) {
				stage = Raised;
			}
			break;
		}
	}
}

void Lift::updateLower() {
	Entity& en = Entity::get(id);

	switch (stage) {
		case Raised: { // input
			if (iid) {
				stage = Lowering;
			}
			break;
		}

		case Raising: {
			en.consume(en.spec->energyConsume);
			ascent++;
			if (ascent == steps-1) {
				stage = Raised;
			}
			break;
		}

		case Lowered: { // output
			if (!iid) {
				stage = Raising;
			}
			break;
		}

		case Lowering: {
			en.consume(en.spec->energyConsume);
			ascent--;
			if (ascent == 0) {
				stage = Lowered;
			}
			break;
		}
	}
}

void Lift::update() {
	Entity& en = Entity::get(id);
	if (en.isGhost()) return;
	if (pause > Sim::tick) return;

	switch (mode) {
		case Raise: updateRaise(); break;
		case Lower: updateLower(); break;
	}

	en.state = std::min(ascent, steps-1);
}

