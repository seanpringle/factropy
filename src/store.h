#pragma once

// Store components are general-purpose configurable item inventories. They
// allow setting lower and upper limits per item and can function as dumb
// containers, logistic requester/suppliers, burner fuel tanks, crafter
// input/output buffers etc. Arms and Drones work together with Stores to
// move items around co-operatively.

//                lower                upper                    capacity
// |----------------|--------------------|-------------------------|
//     requester           provider            active provider
//     overflow            overflow              no overflow

struct Store;

#include "slabmap.h"
#include "minivec.h"
#include "miniset.h"
#include "item.h"
#include "mass.h"
#include <vector>
#include <set>

struct Store {
	uint id;
	static void reset();
	static void tick();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline slabmap<Store,&Store::id> all;
	static Store& create(uint id, uint sid, Mass cap);
	static Store& get(uint id);

	struct Level {
		uint iid = 0;
		uint lower = 0;
		uint upper = 0;
		uint promised = 0;
		uint reserved = 0;
		bool craft = false;
	};

	uint sid;
	uint64_t activity;
	Mass capacity;
	minivec<Stack> stacks;
	minivec<Level> levels;
	miniset<uint> drones;
	miniset<uint> arms;
	bool fuel;
	std::string fuelCategory;

	void destroy();
	void update();
	void ghostInit(uint id, uint sid);
	void ghostDestroy();
	void burnerInit(uint id, uint sid, Mass cap);
	void burnerDestroy();
	Stack insert(Stack stack);
	Stack remove(Stack stack);
	Stack removeAny(uint size);
	uint wouldRemoveAny();
	uint wouldRemoveAny(miniset<uint>& filter);
	Stack removeFuel(std::string chemical, uint size);
	Stack overflowAny(uint size);
	void promise(Stack stack);
	void reserve(Stack stack);
	void levelSet(uint iid, uint lower, uint upper, bool craft = false);
	void levelClear(uint iid);
	Level* level(uint iid);
	bool isEmpty();
	bool isFull();
	Mass limit();
	Mass usage();
	uint count(uint iid);
	uint countNet(uint iid);
	uint countAvailable(uint iid);
	uint countExpected(uint iid);
	uint countAcceptable(uint iid);
	bool isRequesterSatisfied();
	bool isRequesting(uint iid);
	bool isProviding(uint iid);
	bool isActiveProviding(uint iid);
	bool isAccepting(uint iid);
	bool isOverflowDefault(uint iid);
	Stack forceSupplyFrom(Store& src);
	Stack forceSupplyFrom(Store& src, miniset<uint>& filter);
	Stack supplyFrom(Store& src);
	Stack supplyFrom(Store& src, miniset<uint>& filter);
	Stack forceOverflowTo(Store& dst);
	Stack overflowTo(Store& dst);
	Stack overflowTo(Store& dst, miniset<uint>& filter);
	Stack overflowDefaultTo(Store& dst);
};
