#ifndef _H_belt_segment
#define _H_belt_segment

struct BeltSegment;

#include "sparse.h"
#include "entity.h"
#include "belt.h"
#include <set>
#include <list>

struct BeltSegment {
	static inline std::set<BeltSegment*> all;
	static const int slot = 50;
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static void tick();

	struct BeltItem {
		uint iid;
		int offset;
	};

	std::list<Belt*> belts;
	std::list<BeltItem> items;

	bool changed;
	std::list<BeltItem>::iterator expandGap;
	std::list<BeltItem>::iterator shrinkGap;

	uint64_t pauseOffload;
	uint64_t pauseLoad;

	BeltSegment();
	~BeltSegment();
	void push(Belt* belt);
	Belt* pop();
	void shove(Belt* belt);
	Belt* shift();
	void join(BeltSegment* other);
	void split(uint pos);

	void offload();
	void advance();
	void load();
	Box box();
	Point front();
	Point step();

	bool insert(int beltSlot, uint iid, uint area);
	bool remove(int beltSlot, uint iid, uint area);
	uint removeAny(int beltSlot, uint area);
	uint itemAt(int beltSlot, uint area);
};

#endif