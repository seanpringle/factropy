#ifndef _H_arm
#define _H_arm

struct Arm;

#include "sparse.h"
#include "item.h"
#include "entity.h"
#include <map>

struct Arm {
	static void reset();
	static void tick();

	static inline std::map<uint,Arm> all;
	static Arm& create(uint id);
	static Arm& get(uint id);

	enum Stage {
		Input = 1,
		ToInput,
		Output,
		ToOutput,
	};

	uint id;
	float orientation;
	enum Stage stage;

	void destroy();
	void update();
};

#endif