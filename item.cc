#include "common.h"
#include "item.h"

void Item::reset() {
	for (auto pair: ids) {
		delete pair.second;
	}
	names.clear();
	ids.clear();
}

uint Item::next() {
	return sequence++;
}

Item::Item(uint id, std::string name) {
	ensuref(names.count(name) == 0, "duplicate item %s", name.c_str());
	this->name = name;
	this->id = id;
	names[name] = this;
	ids[id] = this;
}

Item::~Item() {
	UnloadImage(image);
}

Item* Item::byName(std::string name) {
	ensuref(names.count(name) == 1, "unknown item %s", name.c_str());
	return names[name];
}

Item* Item::get(uint id) {
	ensuref(ids.count(id) == 1, "unknown item %d", id);
	return ids[id];
}

Stack::Stack(std::initializer_list<size_t> l) {
	auto i = l.begin();
	iid = (uint)*i++;
	size = *i++;
}