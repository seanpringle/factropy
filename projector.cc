#include "common.h"
#include "projector.h"
#include "sim.h"

void Projector::reset() {
	all.clear();
}

void Projector::tick() {
	for (auto& projector: all) {
		projector.update();
	}
}

Projector& Projector::create(uint id) {
	Projector& projector = all[id];
	projector.id = id;
	projector.iid = 0;
	return projector;
}

Projector& Projector::get(uint id) {
	ensuref(all.has(id) > 0, "invalid projector access %d", id);
	return all[id];
}

void Projector::destroy() {
	all.erase(id);
}

void Projector::update() {
	Entity& en = Entity::get(id);
	if (en.isGhost()) return;
	if (Sim::tick%10) return;

	iid = 0;
	fid = 0;

	for (auto eid: Entity::intersecting(en.box().grow(0.5f))) {
		Entity& ae = Entity::get(eid);
		if (ae.isGhost()) continue;
		if (ae.spec->crafter) {
			Crafter& ac = ae.crafter();
			if (!ac.recipe) continue;
			if (ac.recipe->outputItems.size()) {
				iid = ac.recipe->outputItems.begin()->first;
				break;
			}
			if (ac.recipe->outputFluids.size()) {
				fid = ac.recipe->outputFluids.begin()->first;
				break;
			}
		}
		if (ae.spec->pipe) {
			Pipe& pe = ae.pipe();
			if (pe.network && pe.network->fid) {
				fid = pe.network->fid;
				break;
			}
		}
	}
}
