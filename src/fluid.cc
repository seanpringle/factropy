#include "common.h"
#include "fluid.h"

void Fluid::reset() {
	for (auto pair: ids) {
		delete pair.second;
	}
	names.clear();
	ids.clear();
}

uint Fluid::next() {
	return ++sequence;
}

Fluid::Fluid(uint id, std::string name) {
	ensuref(names.count(name) == 0, "duplicate fluid %s", name.c_str());
	this->name = name;
	this->id = id;
	names[name] = this;
	ids[id] = this;
	color = BLUE;
	liquid = Liquid::l(1);
	droplet = nullptr;
	thermal = 0;

	ZERO(image);
	ZERO(texture);
}

Fluid::~Fluid() {
	UnloadImage(image);
	UnloadRenderTexture(texture);
}

Fluid* Fluid::byName(std::string name) {
	ensuref(names.count(name) == 1, "unknown fluid %s", name.c_str());
	return names[name];
}

Fluid* Fluid::get(uint id) {
	ensuref(ids.count(id) == 1, "unknown fluid %d", id);
	return ids[id];
}

bool Fluid::manufacturable() {
	for (auto& [_,recipe]: Recipe::names) {
		if (!recipe->licensed) continue;
		for (auto [fid,_]: recipe->outputFluids) {
			if (fid == id) {
				return true;
			}
		}
	}
	return false;
}

Amount::Amount() {
	fid = 0;
	size = 0;
}

Amount::Amount(std::initializer_list<uint> l) {
	auto i = l.begin();
	fid = (uint)*i++;
	size = *i++;
}

