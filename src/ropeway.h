#pragma once

// Ropeway components are long-range two-way aerial "conveyors" for transporting raw
// materials over long distances including over water and moutainous terrain. They
// are a line of towers linked by cables that move buckets of items at a fixed rate.

// https://duckduckgo.com/?q=ropeway+mining&iax=images&ia=images

struct Ropeway;
struct RopewayBucket;

#include "entity.h"
#include "minivec.h"
#include "miniset.h"
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
	minivec<Point> steps;
	std::list<uint> buckets;
	uint handover;

	miniset<uint> inputFilters;
	miniset<uint> outputFilters;

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