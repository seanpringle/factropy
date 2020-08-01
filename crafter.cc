#include "common.h"
#include "crafter.h"

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
	crafter.recipe = NULL;
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
	return en.pos.floor(0.5f) + (en.dir * (en.spec->d/2.0f+0.5f));
}

void Crafter::update() {
	Entity& en = Entity::get(id);
	if (en.isGhost()) return;

	Store& store = en.store();

	if (!working && recipe) {

		bool itemsReady = true;

		for (auto [item,count]: recipe->inputItems) {
			itemsReady = itemsReady && store.count(item->id) >= count;
		}

		bool miningReady = !recipe->mining || Chunk::isHill(en.box());

		if (itemsReady && miningReady) {

			for (auto [item,count]: recipe->inputItems) {
				store.remove({item->id,count});
			}

			working = true;
			progress = 0.0f;
		}
	}

	if (working) {
		progress += 0.001f;
		progress = std::min(1.0f, progress);
	}

	if (working && progress > 0.999) {

		for (auto [item,count]: recipe->outputItems) {
			store.insert({item->id,count});
		}

		if (recipe->mining) {
			store.insert(Chunk::mine(en.box()));
		}

		working = false;
		progress = 0.0f;
	}

	if (!store.isEmpty()) {
		uint iid = store.stacks.begin()->iid;
		uint outputId = Entity::at(output());

		if (outputId) {
			Entity& eo = Entity::get(outputId);

			if (eo.spec->store) {
				if (eo.store().insert({iid,1}).size == 0) {
					store.remove({iid,1});
				}
			}

			if (eo.spec->belt) {
				if (eo.belt().insert(iid, Belt::Any)) {
					store.remove({iid,1});
				}
			}
		}
	}
}