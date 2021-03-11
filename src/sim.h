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
	extern TimeSeries statsEntity;
	extern TimeSeries statsGhost;
	extern TimeSeries statsStore;
	extern TimeSeries statsArm;
	extern TimeSeries statsCrafter;
	extern TimeSeries statsConveyor;
	extern TimeSeries statsUnveyor;
	extern TimeSeries statsLoader;
	extern TimeSeries statsRopeway;
	extern TimeSeries statsRopewayBucket;
	extern TimeSeries statsProjector;
	extern TimeSeries statsPath;
	extern TimeSeries statsVehicle;
	extern TimeSeries statsPipe;
	extern TimeSeries statsShunt;
	extern TimeSeries statsDepot;
	extern TimeSeries statsDrone;
	extern TimeSeries statsMissile;
	extern TimeSeries statsExplosion;
	extern TimeSeries statsTurret;
	extern TimeSeries statsComputer;

	void reset();
	void save();
	void load();

	typedef std::function<void(void)> lockCallback;
	void locked(lockCallback cb);

	void reseed(int64_t seed);
	float random();
	int choose(uint range);

	// decrease persistence to make coastline smoother
	// increase frequency to make lakes smaller
	double noise2D(double x, double y, int layers, double persistence, double frequency);

	bool rayCast(Point a, Point b, float clearance, std::function<bool(uint)> collide);

	void save(const char *path);
	void load(const char *path);

	void update();

}

#endif
