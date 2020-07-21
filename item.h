#ifndef _H_item
#define _H_item

#include "raylib.h"
#include <map>

struct Item {
	static void reset();

	static inline int sequence;
	static int next();

	static inline std::map<std::string,Item*> names;
	static inline std::map<int,Item*> ids;
	static Item* byName(std::string name);
	static Item* get(int id);

	int id;
	std::string name;
	Image image;

	Item(int id, std::string name);
};

struct Stack {
	int iid;
	size_t size;
};

#endif
