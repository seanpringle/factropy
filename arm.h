#pragma once

// Arm components move items between Stores and Conveyors.

struct Arm;

#include "slabmap.h"
#include "item.h"
#include "entity.h"
#include <map>

struct Arm {
	uint id;
	static void reset();
	static void tick();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline slabmap<Arm,&Arm::id> all;
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

	uint iid;
	uint inputId;
	uint inputStoreId;
	uint outputId;
	uint outputStoreId;
	float orientation;
	enum Stage stage;
	uint64_t pause;
	std::set<uint> filter;

	void destroy();
	void update();
	void updateProximity();
	bool updateInput();
	void updateOutput();
	bool updateReady();
	Point input();
	Point output();
	uint source();
	uint target();
	Stack transferStoreToStore(Store& dst, Store& src);
	Stack transferStoreToBelt(Store& src);
	Stack transferBeltToStore(Store& dst, Stack stack);
};
