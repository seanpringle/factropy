#include "common.h"
#include "entity.h"
#include "generator.h"

void Generator::reset() {
	all.clear();
}

Generator& Generator::create(uint id, uint sid) {
	Generator& generator = all[id];
	generator.id = id;
	generator.supplying = false;
	generator.energy = Energy(0);
	return generator;
}

Generator& Generator::get(uint id) {
	ensuref(all.count(id), "invalid generator access %d", id);
	return all[id];
}

void Generator::destroy() {
	all.erase(id);
}

Energy Generator::consume(Energy e) {
	Entity& en = Entity::get(id);

	if (energy < e) {
		if (en.spec->pipe) {
			Pipe& pipe = Pipe::get(id);

			if (pipe.network && pipe.network->fid && pipe.network->count(pipe.network->fid)) {
				Fluid* fluid = Fluid::get(pipe.network->fid);

				if (fluid->thermal) {

					int remove = std::ceil((float)e.value / (float)fluid->thermal.value);

					if (remove > 0) {
						Amount removed = pipe.network->extract({pipe.network->fid, (uint)remove});

						if (removed.size) {
							int actual = std::min((int)removed.size, remove);
							energy += fluid->thermal * (float)actual;
						}
					}
				}
			}
		}
	}

	e = std::min(e, energy);
	energy -= e;
	supplying = e;

	return e;
}
