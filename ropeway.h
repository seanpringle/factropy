
#pragma once

struct Ropeway;
struct RopewayBucket;

#include "entity.h"
#include <list>

struct Ropeway {
	static void reset();
	static void tick();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline std::map<uint,Ropeway> all;
	static Ropeway& create(uint id);
	static Ropeway& get(uint id);

	uint id;
	uint prev;
	uint next;
	uint cycle;
	bool check;
	Point aim;
	std::vector<Point> steps;
	std::list<uint> buckets;

	void destroy();

	bool first();
	bool last();

	void connect(uint tid);
	void disconnect(uint tid);

	Point arrive();
	Point depart();

	uint leader();
	uint deputy();
	void reorient();
	void rebucket();
	bool complete();
	float length();

	void update();
};

struct RopewayBucket {
	static void reset();
	static void tick();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline std::map<uint,RopewayBucket> all;
	static RopewayBucket& create(uint id);
	static RopewayBucket& get(uint id);

	uint id;
	uint rid;
	uint step;

	void destroy();
	void update();
};