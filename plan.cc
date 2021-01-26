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
		if (!Entity::fits(te->spec, te->pos, te->dir) && !entityFits(te->spec, te->pos, te->dir)) {
			return false;
		}
	}
	return true;
}

bool Plan::entityFits(Spec *spec, Point pos, Point dir) {
	Box bounds = spec->box(pos, dir).shrink(0.1);

	switch (spec->place) {
		case Spec::Land: {

			if (spec->supportPoints.size() > 0) {
				for (auto p: spec->relativePoints(spec->supportPoints, dir.rotation(), pos)) {
					bool supported = p.y < 0 && p.y > -1;

					if (!supported) {
						for (auto bid: Entity::intersecting(p.box().grow(0.1))) {
							Entity& eb = Entity::get(bid);
							if (eb.spec->block) {
								supported = true;
								break;
							}
						}
					}

					if (!supported) {
						for (auto eb: entities) {
							if (eb->spec->block && eb->box().intersects(p.box().grow(0.1))) {
								supported = true;
								break;
							}
						}
					}

					if (!supported) {
						return false;
					}
				}
			}

			if (!Chunk::isLand(bounds)) {
				return false;
			}
			break;
		}
		case Spec::Water: {
			if (!Chunk::isWater(bounds)) {
				return false;
			}
			break;
		}
		case Spec::Hill: {
			if (!Chunk::isHill(bounds)) {
				return false;
			}
			break;
		}
	}

	for (auto sid: Entity::intersecting(bounds)) {
		Entity& es = Entity::get(sid);
		if (es.spec != spec) return false;
		if (es.pos != pos) return false;
		if (es.dir != dir) return false;
	}

	return true;
}