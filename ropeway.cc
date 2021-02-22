#include "common.h"
#include "sim.h"
#include "ropeway.h"

void Ropeway::reset() {
	all.clear();
}

void Ropeway::tick() {
	for (auto& pair: all) {
		auto& ropeway = pair.second;
		if (!ropeway.next && ropeway.check) {
			ropeway.reorient();
			ropeway.rebucket();
			get(ropeway.deputy()).rebucket();
			ropeway.check = false;
		}
	}
	for (auto& pair: all) {
		auto& ropeway = pair.second;
		if (ropeway.next && ropeway.prev) continue;
		ropeway.update();
	}
}

Ropeway& Ropeway::create(uint id) {
	Ropeway& ropeway = all[id];
	ropeway.id = id;
	ropeway.next = 0;
	ropeway.prev = 0;
	ropeway.cycle = 0;
	ropeway.check = false;
	ropeway.aim = Point::Zero;
	ropeway.handover = 0;
	return ropeway;
}

Ropeway& Ropeway::get(uint id) {
	ensuref(all.count(id), "invalid ropeway access %d", id);
	return all[id];
}

void Ropeway::destroy() {
	if (prev) disconnect(prev);
	if (next) disconnect(next);
	all.erase(id);
}

uint Ropeway::leader() {
	return next ? get(next).leader(): id;
}

uint Ropeway::deputy() {
	return prev ? get(prev).deputy(): id;
}

bool Ropeway::complete() {
	return leader() != deputy() && Entity::get(leader()).spec->ropewayTerminus && Entity::get(deputy()).spec->ropewayTerminus;
}

void Ropeway::reorient() {
	Entity& en = Entity::get(id);
	Point pe = en.pos.floor(0);
	aim = en.dir;

	if (prev && next) {
		// rotate to face between prev and next
		Entity& ea = Entity::get(prev);
		Entity& eb = Entity::get(next);
		Point pa = ea.pos.floor(0);
		Point pb = eb.pos.floor(0);
		aim = (pe.nearestPointOnLine(pa, pb) - pe).normalize();
		aim = aim.transform(Mat4::rotateY(90.0f*DEG2RAD));
	}

	if (prev && !next) {
		// rotate to face prev
		Entity& eo = Entity::get(prev);
		Point po = eo.pos.floor(0);
		aim = (po-pe).normalize();
	}

	if (!prev && next) {
		// rotate to face away from next
		Entity& eo = Entity::get(next);
		Point po = eo.pos.floor(0);
		aim = (po-pe).normalize();
	}

	if (next && Point::linesCrossOnGround(get(next).arrive(), arrive(), get(next).depart(), depart())) {
		aim = aim.transform(Mat4::rotateY(180.0f*DEG2RAD));
	}

	if (prev) {
		get(prev).reorient();
	}
}

void Ropeway::rebucket() {
	ensuref(!next || !prev, "ropeway rebucketing needs a terminus");

	steps.clear();
	float step = 1.0f/30.0f;

	if (!next && !prev) {
		for (auto bid: buckets) {
			Entity::get(bid).remove();
		}
		return;
	}

	if (!next) {

		uint towerA = id;
		uint towerB = prev;

		Point pos = get(towerA).depart();
		Point end = get(towerB).depart();
		Point dir = (end - pos).normalize();
		Point inc = dir * step;

		for (;;) {
			steps.push_back(pos);

			if (pos.distance(end) < step) {
				towerA = towerB;
				towerB = get(towerB).prev;
				if (!towerB) break;

				pos = end;
				end = get(towerB).depart();
				dir = (end - pos).normalize();
				inc = dir * step;
				continue;
			}

			pos += inc;
		}
	}

	if (!prev) {

		uint towerA = id;
		uint towerB = next;

		Point pos = get(towerA).arrive();
		Point end = get(towerB).arrive();
		Point dir = (end - pos).normalize();
		Point inc = dir * step;

		for (;;) {
			steps.push_back(pos);

			if (pos.distance(end) < step) {
				towerA = towerB;
				towerB = get(towerB).next;
				if (!towerB) break;

				pos = end;
				end = get(towerB).arrive();
				dir = (end - pos).normalize();
				inc = dir * step;
				continue;
			}

			pos += inc;
		}
	}

	// remove buckets no longer on valid position
	for (auto it = buckets.begin(); it != buckets.end(); ) {

		int bid = *it;
		bool ok = false;
		auto& bucket = RopewayBucket::get(bid);

		for (uint i = 0; i < steps.size(); i++) {
			Point step = steps[i];
			if (Entity::get(bid).pos.floor(0) == step.floor(0)) {
				bucket.step = i;
				ok = true;
				break;
			}
		}

		if (!ok) {
			Entity::get(bid).remove();
		}

		it++;
	}
}

void Ropeway::connect(uint tid) {
	if (prev) return;
	Ropeway& other = get(tid);
	if (other.next) return;
	if (other.complete()) return;
	other.next = id;
	prev = other.id;

	get(leader()).check = true;
}

void Ropeway::disconnect(uint tid) {
	Ropeway& other = get(tid);
	if (prev == tid) {
		other.next = 0;
		prev = 0;
	}
	if (next == tid) {
		other.prev = 0;
		next = 0;
	}

	get(leader()).check = true;
}

Point Ropeway::arrive() {
	Entity& en = Entity::get(id);
	return en.spec->ropewayCableEast
		.transform(aim.rotation())
		.transform(en.pos.translation())
	;
}

Point Ropeway::depart() {
	Entity& en = Entity::get(id);
	return en.spec->ropewayCableEast
		.transform(Mat4::rotateY(180.0f*DEG2RAD))
		.transform(aim.rotation())
		.transform(en.pos.translation())
	;
}

float Ropeway::length() {
	float length = 0.0f;

	uint towerA = leader();
	uint towerB = 0;

	while ((towerB = get(towerA).prev) && towerB) {

		auto& ta = get(towerA);
		auto& tb = get(towerB);

		Point a = ta.arrive();
		Point b = tb.arrive();
		float df = a.distance(b);

		Point c = ta.depart();
		Point d = tb.depart();
		float db = c.distance(d);

		length += df+db;
		towerA = towerB;
	}

	return length;
}

void Ropeway::update() {
	if (next && prev) return;
	if (!steps.size()) return;
	if (!complete()) return;

	uint gap = 30*10;
	bool newBucket = cycle == 0;

	cycle++;
	if (cycle == gap) cycle = 0;

	// leader
	if (prev) {

		if (buckets.size() > 0) {
			uint bid = buckets.back();
			auto& bucket = RopewayBucket::get(bid);
			if (bucket.step == steps.size()-1) {
				auto& partner = get(deputy());
				ensure(!partner.handover);
				partner.handover = bid;
				buckets.pop_back();
			}
		}

		if (handover) {
			Entity::get(handover).remove();
			handover = 0;
		}

		if (newBucket) {
			auto& en = Entity::get(id);
			auto& eb = Entity::create(Entity::next(), en.spec->ropewayBucketSpec);
			eb.move(depart()).materialize();
			RopewayBucket::get(eb.id).rid = id;
			buckets.push_front(eb.id);
		}
	}

	// deputy
	if (next) {

		if (buckets.size() > 0) {
			uint bid = buckets.back();
			auto& bucket = RopewayBucket::get(bid);
			if (bucket.step == steps.size()-1) {
				auto& partner = get(leader());
				ensure(!partner.handover);
				partner.handover = bid;
				buckets.pop_back();
			}
		}

		if (handover) {
			buckets.push_front(handover);
			auto& bucket = RopewayBucket::get(handover);
			bucket.rid = id;
			bucket.step = 0;
			handover = 0;
		}
	}
}

void RopewayBucket::reset() {
	all.clear();
}

void RopewayBucket::tick() {
	for (auto& pair: all) {
		auto& bucket = pair.second;
		bucket.update();
	}
}

RopewayBucket& RopewayBucket::create(uint id) {
	RopewayBucket& bucket = all[id];
	bucket.id = id;
	bucket.rid = 0;
	bucket.step = 0;
	return bucket;
}

RopewayBucket& RopewayBucket::get(uint id) {
	ensuref(all.count(id), "invalid ropeway bucket access %d", id);
	return all[id];
}

void RopewayBucket::destroy() {
	if (Ropeway::all.count(rid)) {
		auto& ropeway = Ropeway::get(rid);
		ropeway.buckets.remove(id);
	}
	all.erase(id);
}

void RopewayBucket::update() {
	if (!rid || !Entity::exists(rid)) {
		Entity::get(id).remove();
		return;
	}

	auto& en = Entity::get(id);
	auto& ropeway = Ropeway::get(rid);

	step = std::min((uint)(ropeway.steps.size()-1), step+1);
	en.move(ropeway.steps[step] + (Point::Down*2.0f));
}