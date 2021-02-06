#pragma once

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
};
