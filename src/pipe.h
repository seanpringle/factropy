#pragma once

// Pipe components transport fluid. They automatically link together with adjacent
// pipes to form a network holding a single fluid type.

// Fluid modelling is fairly simplistic so far; the network tracks the fluid level
// as a percentage of $totalNetworkCapacity and each Pipe just assumes it has
// ($localCapacity * $levelPercentage) fluid available.

struct Pipe;
struct PipeNetwork;

#include "fluid.h"
#include "entity.h"
#include "slabmap.h"
#include <set>

struct Pipe {
	uint id;
	static void reset();
	static void tick();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline slabmap<Pipe,&Pipe::id> all;
	static Pipe& create(uint id);
	static Pipe& get(uint id);

	static std::vector<uint> servicing(Box box);

	PipeNetwork* network;
	uint cacheFid;
	int cacheTally;
	uint partner;
	bool underground;

	void destroy();
	void manage();
	void unmanage();
	std::vector<Point> pipeConnections();
	Amount contents();
	Box undergroundRange();
	void flush();
};

struct PipeNetwork {
	static void tick();
	static inline bool rebuild = true;
	static inline std::set<PipeNetwork*> all;

	std::set<uint> pipes;
	uint fid;
	int tally;
	Liquid limit;

	PipeNetwork();
	~PipeNetwork();
	void propagateLevels();

	void cacheState();
	Amount inject(Amount amount);
	Amount extract(Amount amount);
	uint count(uint fid);
	uint space(uint fid);
	float level();
	void flush();
};
