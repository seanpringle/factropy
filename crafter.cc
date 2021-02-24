#include "common.h"
#include "crafter.h"
#include "ledger.h"

void Crafter::reset() {
	all.clear();
}

void Crafter::tick() {
	for (auto& crafter: all) {
		crafter.update();
	}
}

Crafter& Crafter::create(uint id) {
	Crafter& crafter = all[id];
	crafter.id = id;
	crafter.working = false;
	crafter.progress = 0.0f;
	crafter.efficiency = 0.0f;
	crafter.recipe = NULL;
	crafter.nextRecipe = NULL;
	crafter.energyUsed = 0;
	crafter.completed = 0;
	return crafter;
}

Crafter& Crafter::get(uint id) {
	ensuref(all.has(id) > 0, "invalid crafter access %d", id);
	return all[id];
}

void Crafter::destroy() {
	all.erase(id);
}

Point Crafter::output() {
	Entity& en = Entity::get(id);
	return en.pos.floor(0.5f) + (en.dir * (en.spec->collision.d/2.0f+0.5f));
}

float Crafter::inputsProgress() {
	Entity& en = Entity::get(id);
	float avg = 0.0f;
	float agg = 0.0f;
	if (en.spec->store && recipe) {
		for (auto [iid,count]: recipe->inputItems) {
			agg += std::min(1.0f, (float)en.store().count(iid)/float(count));
		}
		avg = agg/(float)recipe->inputItems.size();
	}
	return std::max(0.0f, std::min(1.0f, avg));
}

std::vector<Point> Crafter::pipeConnections() {
	Entity& en = Entity::get(id);
	return en.spec->relativePoints(en.spec->pipeConnections, en.dir.rotation(), en.pos);
}

std::vector<Point> Crafter::pipeInputConnections() {
	Entity& en = Entity::get(id);
	return en.spec->relativePoints(en.spec->pipeInputConnections, en.dir.rotation(), en.pos);
}

std::vector<Point> Crafter::pipeOutputConnections() {
	Entity& en = Entity::get(id);
	return en.spec->relativePoints(en.spec->pipeOutputConnections, en.dir.rotation(), en.pos);
}

void Crafter::update() {
	Entity& en = Entity::get(id);
	if (en.isGhost()) return;

	if (en.spec->crafterProgress) {
		en.state = (uint)std::floor(progress * (float)en.spec->states.size());
	} else {
		if (working) {
			en.state = en.state + (efficiency > 0.5 ? 2: 1);
			if (en.state >= en.spec->states.size()) en.state = 0;
		}
	}

	if (nextRecipe) {
		if (nextRecipe != recipe) {

			recipe = nextRecipe;
			nextRecipe = NULL;

			ensure(recipe->energyUsage > Energy(0));

			if (en.spec->store) {
				en.store().levels.clear();
				en.store().stacks.clear();
			}

			for (auto [iid,count]: recipe->inputItems) {
				en.store().levelSet(iid, count*2, count*2);
			}

			for ([[maybe_unused]] auto [iid,count]: recipe->outputItems) {
				en.store().levelSet(iid, 0, 0);
			}

			if (recipe->mine) {
				en.store().levelSet(recipe->mine, 0, 1);
			}

			working = false;
			energyUsed = 0;
			progress = 0.0f;
			efficiency = 0.0f;
		}
	}

	if (!working && recipe) {

		bool itemsReady = true;

		if (recipe->inputItems.size()) {
			itemsReady = !en.store().isEmpty();

			for (auto [iid,count]: recipe->inputItems) {
				itemsReady = itemsReady && en.store().count(iid) >= count;
			}
		}

		bool fluidsReady = true;
		std::vector<uint> inputPipes;
		std::vector<uint> outputPipes;

		if (recipe->inputFluids.size()) {
			fluidsReady = false;

			for (auto point: pipeConnections()) {
				for (auto pid: Pipe::servicing(point.box())) {
					inputPipes.push_back(pid);
				}
			}

			for (auto point: pipeInputConnections()) {
				for (auto pid: Pipe::servicing(point.box())) {
					inputPipes.push_back(pid);
				}
			}

			deduplicate(inputPipes);

			for (auto [fid,count]: recipe->inputFluids) {
				for (uint pid: inputPipes) {
					Entity& pe = Entity::get(pid);
					if (pe.isGhost()) continue;
					Pipe& pipe = Pipe::get(pid);
					if (!pipe.network) continue;
					if (pipe.network->count(fid) >= count) {
						fluidsReady = true;
					}
				}
			}
		}

		bool outputReady = true;

		if (outputReady && recipe->outputItems.size()) {
			outputReady = !en.store().isFull();

			for (auto [iid,count]: recipe->outputItems) {
				outputReady = outputReady && en.store().count(iid) <= count;
			}
		}

		if (outputReady && recipe->outputFluids.size()) {
			outputReady = false;

			for (auto point: pipeOutputConnections()) {
				for (auto pid: Pipe::servicing(point.box())) {
					outputPipes.push_back(pid);
				}
			}

			deduplicate(outputPipes);

			for (auto [fid,count]: recipe->outputFluids) {
				for (uint pid: outputPipes) {
					Entity& pe = Entity::get(pid);
					if (pe.isGhost()) continue;
					Pipe& pipe = Pipe::get(pid);
					if (!pipe.network) continue;
					if (pipe.network->space(fid) > 0) {
						outputReady = true;
					}
				}
			}
		}

		if (outputReady && recipe->mine) {
			outputReady = !en.store().isFull();
		}

		bool miningReady = !recipe->mine || Chunk::canMine(en.miningBox(), recipe->mine);

		if (itemsReady && fluidsReady && miningReady && outputReady) {

			for (auto [iid,count]: recipe->inputItems) {
				en.store().remove({iid,count});
			}

			if (recipe->inputFluids.size()) {
				for (auto [fid,count]: recipe->inputFluids) {
					for (uint pid: inputPipes) {
						Entity& pe = Entity::get(pid);
						if (pe.isGhost()) continue;
						Pipe& pipe = Pipe::get(pid);
						if (!pipe.network) continue;
						pipe.network->extract({fid,count});
					}
				}
			}

			working = true;
			energyUsed = 0;
			progress = 0.0f;
			efficiency = 0.0f;
		}
	}

	if (working) {
		energyUsed += en.consume(en.spec->energyConsume * recipe->rate(en.spec));
		efficiency = energyUsed.portion(en.spec->energyConsume);
		progress = energyUsed.portion(recipe->energyUsage);
	}

	if (!working && en.spec->energyDrain) {
		en.consume(en.spec->energyDrain);
	}

	if (working && progress > 0.999) {

		for (auto [iid,count]: recipe->outputItems) {
			en.store().insert({iid,count});
		}

		if (recipe->outputFluids.size()) {

			std::vector<uint> outputPipes;
			for (auto point: pipeOutputConnections()) {
				for (auto pid: Pipe::servicing(point.box())) {
					outputPipes.push_back(pid);
				}
			}

			deduplicate(outputPipes);

			for (auto [fid,count]: recipe->outputFluids) {
				for (uint pid: outputPipes) {
					Entity& pe = Entity::get(pid);
					if (pe.isGhost()) continue;
					Pipe& pipe = Pipe::get(pid);
					if (!pipe.network) continue;
					pipe.network->inject({fid,count});
				}
			}
		}

		if (recipe->mine) {
			Stack stack = Chunk::mine(en.miningBox(), recipe->mine);
			if (stack.iid) en.store().insert(stack);
		}

		if (recipe->outputCurrency) {
			Ledger::transact(recipe->outputCurrency, recipe->name);
		}

		working = false;
		energyUsed = 0;
		progress = 0.0f;
		efficiency = 0.0f;
		completed++;
	}
}