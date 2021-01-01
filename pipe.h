#ifndef _H_pipe
#define _H_pipe

struct Pipe;
struct PipeNetwork;

#include "sparse.h"
#include "fluid.h"
#include "entity.h"
#include <set>
#include <unordered_set>

struct Pipe {
	static void reset();
	static void tick();
	static void saveAll(const char* name);
	static void loadAll(const char* name);

	static inline SparseArray<Pipe> all = (MaxEntity);
	static Pipe& create(uint id);
	static Pipe& get(uint id);

	static std::unordered_set<uint> servicing(Box box);

	uint id;
	PipeNetwork* network;
	uint cacheFid;
	uint cacheTally;

	void destroy();
	void manage();
	void unmanage();
	std::vector<Point> pipeConnections();
	Amount contents();
};

struct PipeNetwork {
	static void tick();
	static inline bool rebuild = true;
	static inline std::set<PipeNetwork*> all;

	std::set<uint> pipes;
	uint fid;
	uint tally;
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
};

#endif