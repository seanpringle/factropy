#ifndef _H_sim
#define _H_sim

#include "opensimplex.h"
#include "time-series.h"
#include "point.h"
#include <mutex>
#include <functional>

namespace Sim {
	extern OpenSimplex* opensimplex;
	extern uint64_t tick;
	extern int64_t seed;

	extern TimeSeries statsElectricityDemand;
	extern TimeSeries statsElectricitySupply;
	extern TimeSeries statsStore;
	extern TimeSeries statsEntity;
	extern TimeSeries statsArm;
	extern TimeSeries statsCrafter;
	extern TimeSeries statsProjector;
	extern TimeSeries statsPath;
	extern TimeSeries statsVehicle;
	extern TimeSeries statsBelt;
	extern TimeSeries statsLift;
	extern TimeSeries statsPipe;
	extern TimeSeries statsShunt;
	extern TimeSeries statsDepot;
	extern TimeSeries statsDrone;

	void reset();
	void save();
	void load();

	typedef std::function<void(void)> lockCallback;
	void locked(lockCallback cb);

	void reseed(int64_t seed);
	float random();
	int choose(uint range);

	// increase frequency to make lakes smaller
	// decrease persistence to make coastline smoother
	double noise2D(double x, double y, int layers, double persistence, double frequency);

	bool rayCast(Point a, Point b, float clearance, std::function<bool(uint)> collide);

	void save(const char *path);
	void load(const char *path);

	void update();

}

#endif
