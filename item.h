#ifndef _H_item
#define _H_item

#include "raylib.h"
#include <map>
#include <set>

struct Item {
	static void reset();

	static inline uint sequence;
	static uint next();

	static inline std::map<std::string,Item*> names;
	static inline std::map<uint,Item*> ids;
	static Item* byName(std::string name);
	static Item* get(uint id);

	static inline std::set<Item*> mining;

	uint id;
	std::string name;
	Image image;

	Item(uint id, std::string name);
	~Item();
};

struct Stack {
	uint iid;
	size_t size;

	Stack(std::initializer_list<size_t>);
};

#endif
