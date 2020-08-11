#include "common.h"
#include "item.h"

Fuel::Fuel() {
	energy = 0;
}

Fuel::Fuel(std::string cat, Energy e) {
	category = cat;
	energy = e;
}

Stack::Stack() {
	iid = 0;
	size = 0;
}

Stack::Stack(std::initializer_list<uint> l) {
	auto i = l.begin();
	iid = (uint)*i++;
	size = *i++;
}

void Item::reset() {
	for (auto pair: ids) {
		delete pair.second;
	}
	names.clear();
	ids.clear();
}

uint Item::next() {
	return ++sequence;
}

Item::Item(uint id, std::string name) {
	ensuref(names.count(name) == 0, "duplicate item %s", name.c_str());
	this->name = name;
	this->id = id;
	names[name] = this;
	ids[id] = this;
	mass = Mass::kg(1);
	beltV = 0;
	armV = 0;

	ZERO(image);
	ZERO(texture);
}

Item::~Item() {
	UnloadImage(image);
	UnloadRenderTexture(texture);
}

Item* Item::byName(std::string name) {
	ensuref(names.count(name) == 1, "unknown item %s", name.c_str());
	return names[name];
}

Item* Item::get(uint id) {
	ensuref(ids.count(id) == 1, "unknown item %d", id);
	return ids[id];
}

