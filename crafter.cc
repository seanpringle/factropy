#include "common.h"
#include "crafter.h"
#include "ledger.h"

void Crafter::reset() {
	all.clear();
}

void Crafter::tick() {
	for (auto& pair: all) {
		pair.second.update();
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
	ensuref(all.count(id) > 0, "invalid crafter access %d", id);
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
	Store& store = en.store();
	float avg = 0.0f;
	float agg = 0.0f;
	if (recipe) {
		for (auto [iid,count]: recipe->inputItems) {
			agg += std::min(1.0f, (float)store.count(iid)/float(count));
		}
		avg = agg/(float)recipe->inputItems.size();
	}
	return std::max(0.0f, std::min(1.0f, avg));
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

	Store& store = en.store();

	if (nextRecipe) {
		if (nextRecipe != recipe) {

			recipe = nextRecipe;
			nextRecipe = NULL;

			ensure(recipe->energyUsage > Energy(0));

			store.levels.clear();
			store.stacks.clear();

			for (auto [iid,count]: recipe->inputItems) {
				store.levelSet(iid, count*2, count*2);
			}

			for ([[maybe_unused]] auto [iid,count]: recipe->outputItems) {
				store.levelSet(iid, 0, 0);
			}

			if (recipe->mine) {
				store.levelSet(recipe->mine, 0, 1);
			}

			working = false;
			energyUsed = 0;
			progress = 0.0f;
			efficiency = 0.0f;
		}
	}

	if (!working && recipe) {

		bool itemsReady = true;

		for (auto [iid,count]: recipe->inputItems) {
			itemsReady = itemsReady && store.count(iid) >= count;
		}

		bool outputReady = true;

		for (auto [iid,count]: recipe->outputItems) {
			outputReady = outputReady && store.count(iid) <= count;
		}

		bool miningReady = !recipe->mine || Chunk::canMine(en.miningBox(), recipe->mine);

		if (!store.isFull() && itemsReady && miningReady && outputReady) {

			for (auto [iid,count]: recipe->inputItems) {
				store.remove({iid,count});
			}

			working = true;
			energyUsed = 0;
			progress = 0.0f;
			efficiency = 0.0f;
		}
	}

	if (working) {
		energyUsed += en.consume(en.spec->energyConsume);
		efficiency = energyUsed.portion(en.spec->energyConsume);
		progress = energyUsed.portion(recipe->energyUsage);
	}

	if (!working && en.spec->energyDrain) {
		en.consume(en.spec->energyDrain);
	}

	if (working && progress > 0.999) {

		for (auto [iid,count]: recipe->outputItems) {
			store.insert({iid,count});
		}

		if (recipe->mine) {
			Stack stack = Chunk::mine(en.miningBox(), recipe->mine);
			if (stack.iid) store.insert(stack);
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