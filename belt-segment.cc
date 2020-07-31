#include "common.h"
#include "belt-segment.h"
#include "sim.h"

void BeltSegment::tick() {
	for (BeltSegment* segment: all) {
		segment->offload();
	}
	for (BeltSegment* segment: all) {
		segment->advance();
	}
}

BeltSegment::BeltSegment() {
	all.insert(this);
	changed = true;
	pauseOffload = 0;
}

BeltSegment::~BeltSegment() {
	all.erase(this);
	for (Belt* belt: belts) {
		belt->segment = NULL;
	}
	belts.clear();
}

void BeltSegment::push(Belt* belt) {
	ensure(belt->segment == NULL);
	belt->segment = this;
	belt->offset = belts.size();
	belts.push_back(belt);
}

Belt* BeltSegment::pop() {
	Belt* belt = belts.back();
	belt->segment = NULL;
	belt->offset = 0;
	belts.pop_back();
	if (!belts.size()) {
		delete this;
	}
	return belt;
}

void BeltSegment::shove(Belt* belt) {
	ensure(belt->segment == NULL);
	belt->segment = this;
	belt->offset = 0;
	belts.push_front(belt);
	for (Belt* sib: belts) {
		sib->offset++;
	}
}

Belt* BeltSegment::shift() {
	Belt* belt = belts.front();
	belt->segment = NULL;
	belts.pop_front();
	for (Belt* sib: belts) {
		sib->offset--;
	}
	if (!belts.size()) {
		delete this;
	}
	return belt;
}

void BeltSegment::join(BeltSegment* other) {

	// The leading belt item on the other belt will need its
	// offset increased to include any trailing space on this belt
	int sumOffset = 0;
	for (auto it = items.begin(); it != items.end(); it++) {
		sumOffset += it->offset;
		sumOffset += slot;
	}

	int trailingGap = ((int)belts.size() * slot) - sumOffset;

	if (other->items.size()) {
		other->items.front().offset += trailingGap;
	}

	// transfer items across maintaining gaps
	for (auto it = other->items.begin(); it != other->items.end(); it++) {
		items.push_back(*it);
	}

	// transfer belts
	for (uint i = 0, l = other->belts.size(); i < l; i++) {
		push(other->shift());
	}

	changed = true;
	other->changed = true;
}

void BeltSegment::split(uint beltSlot) {

	removeAny(beltSlot);
	uint len = belts.size();

	if (len == 1) {
		pop();
		return;
	}

	// drop from front
	if (beltSlot == 0) {
		shift();
		if (items.size()) {
			items.front().offset -= slot;
		}
		return;
	}

	// drop from rear
	if (beltSlot+1 >= belts.size()) {
		pop();
		return;
	}

	BeltSegment *left = this;
	BeltSegment *right = new BeltSegment();

	// transfer trailing items onto the new right segemtn
	int absOffset = beltSlot * slot;
	int sumOffset = 0;
	for (auto it = items.begin(); it != items.end(); it++) {
		sumOffset += it->offset;
		if (sumOffset >= absOffset+slot) {
			right->items.push_back(*it);
		}
		sumOffset += slot;
	}

	for (uint i = 0; i < right->items.size(); i++) {
		left->items.pop_back();
	}

	// transfer trailing belts onto the new right segment
	std::vector<Belt*> tmp;

	for (uint i = beltSlot+1, l = left->belts.size(); i < l; i++) {
		tmp.push_back(left->pop());
	}

	pop();

	for (uint i = tmp.size(); i > 0; i--) {
		right->push(tmp[i-1]);
	}

	// The leading right segment item needs its offset decreased
	// to remove any trailing offset it had on the left segment

	sumOffset = 0;
	for (auto it = left->items.begin(); it != left->items.end(); it++) {
		sumOffset += it->offset;
		sumOffset += slot;
	}

	int trailingGap = ((int)left->belts.size() * slot) - sumOffset;

	if (right->items.size()) {
		right->items.front().offset -= (trailingGap+slot);
	}

	ensure(left->belts.size() == beltSlot);
	ensure(right->belts.size() == len-beltSlot-1);

	left->changed = true;
	right->changed = true;
}

bool BeltSegment::insert(int beltSlot, uint iid) {
	ensure(beltSlot >= 0 && beltSlot < (int)belts.size());
	int absOffset = beltSlot * slot;

	// belt is empty
	if (items.size() == 0) {
		items.push_back({
			iid: iid,
			offset: absOffset,
		});
		changed = true;
		return true;
	}

	int sumOffset = 0;

	for (auto it = items.begin(); it != items.end(); it++) {
		int relOffset = absOffset - sumOffset;

		if (relOffset < 0) {
			return false;
		}

		// allow insertion into an undersized gap for good belt compression.
		// the tick handler will smoothly even out the spacing
		int gap = slot/4;

		if (relOffset+gap < it->offset) {

			it->offset -= relOffset;
			it->offset -= slot;

			items.insert(it, {
				iid: iid,
				offset: relOffset,
			});
			changed = true;
			return true;
		}

		sumOffset += it->offset;
		sumOffset += slot;
	}

	// end of belt
	if (absOffset >= sumOffset) {
		int relOffset = absOffset - sumOffset;
		items.push_back({
			iid: iid,
			offset: relOffset,
		});
		changed = true;
		return true;
	}

	return false;
}

bool BeltSegment::remove(int beltSlot, uint iid) {
	ensure(beltSlot >= 0 && beltSlot < (int)belts.size());
	int absOffset = beltSlot * slot;

	int sumOffset = 0;

	for (auto it = items.begin(); it != items.end(); it++) {
		sumOffset += it->offset;

		if (absOffset == sumOffset && it->iid == iid) {
			auto in = std::next(it, 1);

			if (in != items.end()) {
				in->offset += it->offset;
				in->offset += slot;
			}

			items.erase(it);
			changed = true;
			return true;
		}

		sumOffset += slot;
	}

	return false;
}

uint BeltSegment::removeAny(int beltSlot) {
	ensure(beltSlot >= 0 && beltSlot < (int)belts.size());
	int absOffset = beltSlot * slot;

	int sumOffset = 0;

	for (auto it = items.begin(); it != items.end(); it++) {
		sumOffset += it->offset;

		if (absOffset == sumOffset) {
			uint iid = it->iid;
			auto in = std::next(it, 1);

			if (in != items.end()) {
				in->offset += it->offset;
				in->offset += slot;
			}

			items.erase(it);
			changed = true;
			return iid;
		}

		sumOffset += slot;
	}

	return 0;
}

uint BeltSegment::itemAt(int beltSlot) {
	ensure(beltSlot >= 0 && beltSlot < (int)belts.size());
	int absOffset = beltSlot * slot;

	int sumOffset = 0;

	for (auto it = items.begin(); it != items.end(); it++) {
		sumOffset += it->offset;

		if (absOffset == sumOffset) {
			return it->iid;
		}

		sumOffset += slot;
	}

	return 0;
}

Box BeltSegment::box() {
	Box fb = Entity::get(belts.front()->id).box();

	if (belts.size() == 1) {
		return fb;
	}

	Box bb = Entity::get(belts.back()->id).box();

	Point dir = Entity::get(belts.front()->id).dir;

	if (dir == Point::South()) {
		float l = fb.z-bb.z;
		return (Box){
			fb.x, fb.y, fb.z - l/2.0f,
			fb.w, fb.h, l + fb.d,
		};
	}

	if (dir == Point::North()) {
		float l = bb.z-fb.z;
		return (Box){
			bb.x, bb.y, bb.z - l/2.0f,
			bb.w, bb.h, l + bb.d,
		};
	}

	if (dir == Point::East()) {
		float l = fb.x-bb.x;
		return (Box){
			fb.x - l/2.0f, fb.y, fb.z,
			l + fb.w, fb.h, fb.d,
		};
	}

	if (dir == Point::West()) {
		float l = bb.x-fb.x;
		return (Box){
			bb.x - l/2.0f, bb.y, bb.z,
			l + bb.w, bb.h, bb.d,
		};
	}

	fatalf("invalid segment direction");
}

Point BeltSegment::front() {
	Entity& en = Entity::get(belts.front()->id);
	return en.pos + (Point::Up()*0.5f) + (en.dir*0.5f);
}

Point BeltSegment::step() {
	Entity& en = Entity::get(belts.front()->id);
	return -en.dir * (1.0f/(float)slot);
}

void BeltSegment::offload() {
	if (pauseOffload > Sim::tick) return;

	if (items.size() > 0 && items.front().offset == 0) {
		Entity& en = Entity::get(belts.front()->id);

		uint tid = Entity::at(en.pos + en.dir);

		if (tid) {
			Entity& te = Entity::get(tid);

			if (te.spec->belt && te.belt().insert(items.front().iid)) {
				removeAny(0);
				return;
			}

			if (te.spec->store && te.store().insert({items.front().iid,1}).size == 0) {
				removeAny(0);
				return;
			}
		}

		pauseOffload = Sim::tick+15;
	}
}

void BeltSegment::advance() {
	if (changed) {

		bool shrinkFound = false;
		shrinkGap = items.begin();

		bool expandFound = false;
		expandGap = items.begin();

		for (uint i = 0, l = items.size(); i < l; i++) {
			shrinkFound = shrinkFound || shrinkGap->offset > 0;
			expandFound = expandFound || expandGap->offset < 0;
			if (!shrinkFound) shrinkGap++;
			if (!expandFound) expandGap++;
			if (shrinkFound && expandFound) break;
		}

		changed = false;
	}

	bool shrank = false;

	if (shrinkGap != items.end() && shrinkGap->offset > 0) {
		shrinkGap->offset--;
		shrank = true;
	}

	while (shrinkGap != items.end() && shrinkGap->offset <= 0) {
		shrinkGap++;
	}

	if (shrank && expandGap != items.end() && expandGap->offset < 0) {
		expandGap->offset++;

		for (auto shrinkAhead = expandGap; shrinkAhead != items.end(); shrinkAhead++) {
			if (shrinkAhead->offset > 0) {
				shrinkAhead->offset--;
				break;
			}
		}
	}

	while (expandGap != items.end() && expandGap->offset >= 0) {
		expandGap++;
	}
}
