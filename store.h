#ifndef _H_store
#define _H_store

#include "sparse.h"
#include "item.h"
#include "entity.h"
#include "mass.h"
#include <vector>

struct Store {
	static void reset();
	static void tick();

	static inline SparseArray<Store> all = (MaxEntity);
	static Store& create(uint id);
	static Store& get(uint id);

	struct Level {
		uint iid;
		uint lower;
		uint upper;
		uint promised;
		uint reserved;
	};

	uint id;
	uint64_t activity;
	std::vector<Stack> stacks;
	std::vector<Level> levels;

	void destroy();
	Stack insert(Stack stack);
	Stack remove(Stack stack);
	void promise(Stack stack);
	void reserve(Stack stack);
	void clearLevels();
	void levelSet(uint iid, uint lower, uint upper);
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
	bool isRequesterSatisfied();
	bool isRequesting(uint iid);
	bool isProviding(uint iid);
	bool isActiveProviding(uint iid);
	bool isAccepting(uint iid);
	bool isOverflowDefault(uint iid);
	Stack forceSupplyFrom(Store& src);
	Stack supplyFrom(Store& src);
	Stack forceOverflowTo(Store& dst);
	Stack overflowTo(Store& dst);
	Stack overflowDefaulTo(Store& dst);

};

#endif
