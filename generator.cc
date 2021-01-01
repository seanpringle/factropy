#include "common.h"
#include "entity.h"
#include "generator.h"

void Generator::reset() {
	all.clear();
}

Generator& Generator::create(uint id, uint sid) {
	Generator& generator = all.ref(id);
	generator.id = id;
	generator.supplying = false;
	generator.monitor = Energy(0);
	return generator;
}

Generator& Generator::get(uint id) {
	ensuref(all.has(id), "invalid generator access %d", id);
	return all.ref(id);
}

void Generator::destroy() {
	all.drop(id);
}

Energy Generator::consume(Energy e) {
	Entity& en = Entity::get(id);

	supplying = false;
	if (monitor.value <= 0) monitor = en.spec->energyGenerate;

	if (en.spec->pipe) {
		Pipe& pipe = Pipe::get(id);

		if (pipe.network && pipe.network->fid && pipe.network->count(pipe.network->fid)) {
			Fluid* fluid = Fluid::get(pipe.network->fid);

			if (fluid->thermal) {
				supplying = true;

				int remove = std::ceil((float)e.value / (float)fluid->thermal.value);

				if (remove > 0) {
					Amount removed = pipe.network->extract({pipe.network->fid, (uint)remove});

					if (removed.size) {
						int actual = std::min((int)removed.size, remove);
						Energy transfer = fluid->thermal * (float)actual;
						monitor -= transfer;
						return transfer;
					}
				}
			}
		}
	}

	return 0;
}
