#include "common.h"
#include "sim.h"
#include "spec.h"
#include "json.hpp"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;
using json = nlohmann::json;

void Spec::saveAll(const char* name) {
	auto path = std::string(name);
	auto out = std::ofstream(path + "/specs.json");

	for (auto& pair: all) {
		Spec *spec = pair.second;
		json state;
		state["name"] = spec->name;
		out << state << "\n";
	}

	out.close();
}

void Spec::loadAll(const char* name) {
}

void Spec::reset() {
	for (auto pair: all) {
		for (auto part: pair.second->parts) {
			delete part;
		}
		delete pair.second;
	}
	all.clear();
}

Spec::Spec(std::string name) {
	ensuref(all.count(name) == 0, "duplicate spec name %s", name.c_str());
	notef("Spec: %s", name.c_str());
	this->name = name;
	all[name] = this;

	align = false;
	pivot = false;
	vehicle = false;
	drone = false;
	store = false;

	w = 1.0;
	d = 1.0;
	h = 1.0;
	costGreedy = 1.0;
}

Spec* Spec::byName(std::string name) {
	ensuref(all.count(name) == 1, "unknown spec name %s", name.c_str());
	return all[name];
}

Point Spec::aligned(Point p, Point dir) {
	if (align) {

		float ww = w;
		//float hh = h;
		float dd = d;

		if (dir == Point::West() || dir == Point::East()) {
			ww = d;
			dd = w;
		}

		p.x = std::floor(p.x);
		if ((int)ceil(ww)%2 != 0) {
			p.x += 0.5;
		}

		//p.y = std::floor(p.y);
		//if ((int)ceil(h)%2 != 0) {
		//	p.y += 0.5;
		//}

		p.z = std::floor(p.z);
		if ((int)ceil(dd)%2 != 0) {
			p.z += 0.5;
		}
	}
	return p;
}

Box Spec::box(Point pos, Point dir) {

	float ww = w;
	//float hh = h;
	float dd = d;

	if (dir == Point::West() || dir == Point::East()) {
		ww = d;
		dd = w;
	}

	return {pos.x, pos.y, pos.z, ww, h, dd};
}

