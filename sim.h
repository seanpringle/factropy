#ifndef _H_sim
#define _H_sim

#include "opensimplex.h"
#include "time-series.h"
#include <mutex>
#include <functional>

namespace Sim {
	extern OpenSimplex* opensimplex;
	extern uint64_t tick;

	extern TimeSeries statsEntity;
	extern TimeSeries statsArm;
	extern TimeSeries statsCrafter;
	extern TimeSeries statsPath;
	extern TimeSeries statsVehicle;

	void reset();

	typedef std::function<void(void)> lockCallback;
	void locked(lockCallback cb);

	void seed(int64_t seed);
	float random();
	int choose(uint range);

	// increase frequency to make lakes smaller
	// decrease persistence to make coastline smoother
	double noise2D(double x, double y, int layers, double persistence, double frequency);

	void save(const char *path);
	void load(const char *path);

	void update();
}

#endif
