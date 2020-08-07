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
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline std::map<uint,Arm> all;
	static Arm& create(uint id);
	static Arm& get(uint id);

	enum Stage {
		Input = 1,
		ToInput,
		Output,
		ToOutput,
		Parked,
		Parking,
		Unparking,
	};

	uint id;
	uint iid;
	float orientation;
	enum Stage stage;
	uint64_t pause;

	void destroy();
	void update();
	bool updateInput();
	void updateOutput();
	bool updateReady();
	Point input();
	Point output();
	uint source();
	uint target();
};

#endif
