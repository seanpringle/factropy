#ifndef _H_fluid
#define _H_fluid

struct Fluid;
struct Amount;

#include "item.h"

struct Fluid {
	static void reset();

	static inline uint sequence;
	static uint next();

	static inline std::map<std::string,Fluid*> names;
	static inline std::map<uint,Fluid*> ids;
	static Fluid* byName(std::string name);
	static Fluid* get(uint id);

	uint id;
	std::string name;
	Image image;
	RenderTexture texture;
	Color color;
	Liquid liquid;
	Part* droplet;
	Energy thermal;

	Fluid(uint id, std::string name);
	~Fluid();

	bool manufacturable();
};

struct Amount {
	uint fid;
	uint size;

	Amount();
	Amount(std::initializer_list<uint>);
};

#endif
