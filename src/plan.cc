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
	position = (entities.size() == 1 && !entities[0]->spec->align)
		? p: Point(std::floor(p.x), std::floor(p.y), std::floor(p.z));
	for (uint i = 0; i < entities.size(); i++) {
		auto te = entities[i];
		te->move(position + offsets[i]);
	}
}

void Plan::rotate() {
	if (entities.size() == 1) {
		auto ge = entities[0];
		ge->rotate();
		return;
	}
	for (uint i = 0; i < entities.size(); i++) {
		auto te = entities[i];
		if (!te->spec->rotate) return;
	}
	Mat4 rot = Mat4::rotateY(90*DEG2RAD);
	for (uint i = 0; i < entities.size(); i++) {
		auto te = entities[i];
		offsets[i] = offsets[i].transform(rot);
		if (te->spec->align) {
			offsets[i].x = std::round(offsets[i].x*2.0f)/2.0f;
			offsets[i].z = std::round(offsets[i].z*2.0f)/2.0f;
		}
		te->dir = te->dir.rotateHorizontal();
		te->pos = position + offsets[i];
		te->updateTransform();
	}
}

void Plan::cycle() {
	if (entities.size() == 1) {
		auto ge = entities[0];

		if (ge->spec->cycle) {
			auto ce = new GuiFakeEntity(ge->spec->cycle);
			ce->dir = ge->dir;
			ce->move(ge->pos);
			entities[0] = ce;

			if (ge->spec->conveyor) {
				Point ginput = ge->spec->conveyorInput
					.transform(ge->dir.rotation())
					.transform(ge->pos.translation());
				Box gbox = ginput.box().grow(0.1f);
				for (int i = 0; i < 4; i++) {
					Point cinput = ce->spec->conveyorInput
						.transform(ce->dir.rotation())
						.transform(ce->pos.translation());
					if (gbox.contains(cinput)) break;
					ce->rotate();
				}
			}

			delete ge;
		}
	}
}

void Plan::floor(float level) {
	position.y = std::floor(level);
	//for (uint i = 0; i < entities.size(); i++) {
	//	auto te = entities[i];
	//	te->floor(position.y + offsets[i].y);
	//}
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
	Box bounds = spec->box(pos, dir, spec->collision).shrink(0.1);

	for (auto sid: Entity::intersecting(bounds)) {
		Entity& es = Entity::get(sid);
		if (es.spec != spec) return false;
		if (es.pos != pos) return false;
		if (es.dir != dir) return false;
	}

	if (spec->place != Spec::Footings) {
		for (auto [x,y]: Chunk::walkTiles(bounds)) {
			Chunk::Tile *tile = Chunk::tileTryGet(x, y);
			if (!tile) return false;
			bool ok = false;
			ok = ok || ((spec->place & Spec::Land) && tile->isLand());
			ok = ok || ((spec->place & Spec::Hill) && tile->isHill());
			ok = ok || ((spec->place & Spec::Water) && tile->isWater());
			if (!ok) return false;
		}
		return true;
	}

	for (auto& footing: spec->footings) {
		Point point = footing.point.transform(dir.rotation()) + pos;
		Chunk::Tile *tile = Chunk::tileTryGet(std::floor(point.x), std::floor(point.z));

		if (!tile) return false;
		bool ok = false;
		ok = ok || ((footing.place & Spec::Land) && tile->isLand());
		ok = ok || ((footing.place & Spec::Hill) && tile->isHill());
		ok = ok || ((footing.place & Spec::Water) && tile->isWater());
		if (!ok) return false;
	}

	return true;
}