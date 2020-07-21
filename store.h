#ifndef _H_store
#define _H_store

#include "sparse.h"
#include "item.h"
#include "entity.h"
#include <vector>

struct Store {
	static void reset();

	static inline SparseArray<Store> all = (MaxEntity);
	static Store& create(int id);
	static Store& get(int id);

	int id;
	std::vector<Stack> stacks;

	void destroy();
	Stack insert(Stack stack);
	Stack remove(Stack stack);
};

#endif
