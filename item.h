#ifndef _H_item
#define _H_item

struct Item;
struct Fuel;
struct Stack;

#include "raylib.h"
#include "part.h"
#include "mass.h"
#include "energy.h"
#include <map>
#include <set>
#include <vector>

struct Fuel {
	std::string category;
	Energy energy;

	Fuel();
	Fuel(std::string, Energy);
};

struct Stack {
	uint iid;
	uint size;

	Stack();
	Stack(std::initializer_list<uint>);
};

struct Item {
	static void reset();

	static inline uint sequence;
	static uint next();

	static inline std::map<std::string,Item*> names;
	static inline std::map<uint,Item*> ids;
	static Item* byName(std::string name);
	static Item* get(uint id);

	static inline std::set<uint> mining;

	uint id;
	std::string name;
	Image image;
	Mass mass;
	Fuel fuel;
	std::vector<Part*> parts;
	RenderTexture texture;
	float beltV;
	float armV;

	Item(uint id, std::string name);
	~Item();
};

#endif
