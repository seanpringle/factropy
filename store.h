#ifndef _H_store
#define _H_store

struct Store;

#include "sparse.h"
#include "item.h"
#include "mass.h"
#include <vector>
#include <set>

struct Store {
	static void reset();
	static void tick();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline SparseArray<Store> all = (MaxEntity);
	static Store& create(uint id, uint sid, Mass cap);
	static Store& get(uint id);

	struct Level {
		uint iid;
		uint lower;
		uint upper;
		uint promised;
		uint reserved;
	};

	uint id;
	uint sid;
	uint64_t activity;
	Mass capacity;
	std::vector<Stack> stacks;
	std::vector<Level> levels;
	std::set<uint> drones;
	std::set<uint> arms;
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
	Stack removeFuel(std::string chemical, uint size);
	Stack overflowAny(uint size);
	void promise(Stack stack);
	void reserve(Stack stack);
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
	Stack overflowDefaultTo(Store& dst);
};

#endif
