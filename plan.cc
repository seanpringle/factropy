#include "common.h"
#include "plan.h"

Plan::Plan() {
	position = Point::Zero;
}

Plan::Plan(Point p) {
	position = Point(std::floor(p.x), 0.0f, std::floor(p.z));
}

Plan::~Plan() {
	for (auto te: entities) {
		delete te;
	}
	entities.clear();
}

void Plan::add(GuiFakeEntity* ge) {
	entities.push_back(ge);
	Point offset = ge->pos - position;
	offsets.push_back(offset);
}

void Plan::move(Point p) {
	position = Point(std::floor(p.x), std::floor(p.y), std::floor(p.z));
	for (uint i = 0; i < entities.size(); i++) {
		auto te = entities[i];
		te->move(position + offsets[i]);
	}
}

void Plan::rotate() {
	if (entities.size() == 1) {
		entities[0]->rotate();
		return;
	}
	Mat4 rot = Mat4::rotateY(90*DEG2RAD);
	for (uint i = 0; i < entities.size(); i++) {
		auto te = entities[i];
		offsets[i] = offsets[i].transform(rot);
		te->move(position + offsets[i]);
		offsets[i] = te->pos - position;
		te->rotate();
	}
}

void Plan::floor(float level) {
	position.y = std::floor(level);
}

bool Plan::fits() {
	for (uint i = 0; i < entities.size(); i++) {
		auto te = entities[i];
		if (!Entity::fits(te->spec, te->pos, te->dir)) {
			return false;
		}
	}
	return true;
}