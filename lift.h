#ifndef _H_lift
#define _H_lift

struct Lift;

#include "sparse.h"
#include "item.h"
#include "entity.h"
#include <map>

struct Lift {
	static void reset();
	static void tick();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline std::map<uint,Lift> all;
	static Lift& create(uint id);
	static Lift& get(uint id);

	enum Mode {
		Raise = 1,
		Lower,
	};

	enum Stage {
		Lowered = 1,
		Lowering,
		Raised,
		Raising,
	};

	uint id;
	uint iid;
	uint steps;
	uint ascent;
	enum Mode mode;
	enum Stage stage;
	uint64_t pause;

	void destroy();
	void toggle();

	bool insert(uint iid, float level);
	bool remove(uint iid, float level);
	uint removeAny(float level);

	void updateLower();
	void updateRaise();
	void update();
};

#endif
