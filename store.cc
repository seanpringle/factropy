#include "common.h"
#include "store.h"

void Store::reset() {
	all.clear();
}

Store& Store::create(int id) {
	Store& store = all.ref(id);
	store.id = id;
	return store;
}

Store& Store::get(int id) {
	ensuref(all.has(id), "invalid store access %d", id);
	return all.ref(id);
}

void Store::destroy() {
	stacks.clear();
	all.drop(id);
}

Stack Store::insert(Stack istack) {
	for (auto it = stacks.begin(); it != stacks.end(); it++) {
		if (it->iid == istack.iid) {
			it->size += istack.size;
			istack.size = 0;
			break;
		}
	}
	if (istack.size > 0) {
		stacks.push_back(istack);
		istack.size = 0;
	}
	return istack;
}

Stack Store::remove(Stack rstack) {
	for (auto it = stacks.begin(); it != stacks.end(); it++) {
		if (it->iid == rstack.iid) {
			if (it->size <= rstack.size) {
				rstack.size = it->size;
				stacks.erase(it);
			} else {
				it->size -= rstack.size;
			}
			return rstack;
		}
	}
	rstack.size = 0;
	return rstack;
}