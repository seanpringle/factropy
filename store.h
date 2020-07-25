#ifndef _H_store
#define _H_store

#include "sparse.h"
#include "item.h"
#include "entity.h"
#include <vector>

struct Store {
	static void reset();
	static void tick();

	static inline SparseArray<Store> all = (MaxEntity);
	static Store& create(uint id);
	static Store& get(uint id);

	uint id;
	std::vector<Stack> stacks;

	void destroy();
	Stack insert(Stack stack);
	Stack remove(Stack stack);
	size_t count(uint iid);
};

#endif
