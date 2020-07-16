#ifndef _H_sim
#define _H_sim

#include "opensimplex.h"
#include <mutex>
#include <functional>

namespace Sim {
	extern OpenSimplex* opensimplex;
	extern uint64_t tick;

	typedef std::function<void(void)> lockCallback;
	void locked(lockCallback cb);

	void seed(int64_t seed);
	double noise2D(int x, int y, int layers, double persistence, double frequency);

	void save(const char *path);
	void load(const char *path);

	void update();
}

#endif
