#include "common.h"
#include "spec.h"
#include "entity.h"
#include "chunk.h"
#include "sim.h"
#include "time-series.h"
#include <cstdlib>

namespace Sim {

	TimeSeries statsElectricityDemand;
	TimeSeries statsElectricitySupply;
	TimeSeries statsEntity;
	TimeSeries statsGhost;
	TimeSeries statsStore;
	TimeSeries statsArm;
	TimeSeries statsCrafter;
	TimeSeries statsConveyor;
	TimeSeries statsUnveyor;
	TimeSeries statsLoader;
	TimeSeries statsRopeway;
	TimeSeries statsRopewayBucket;
	TimeSeries statsProjector;
	TimeSeries statsPath;
	TimeSeries statsVehicle;
	TimeSeries statsPipe;
	TimeSeries statsShunt;
	TimeSeries statsDepot;
	TimeSeries statsDrone;
	TimeSeries statsMissile;
	TimeSeries statsExplosion;
	TimeSeries statsTurret;
	TimeSeries statsComputer;

	OpenSimplex* opensimplex;
	std::mutex mutex;
	uint64_t tick;
	int64_t seed;

	void reseed(int64_t s) {
		seed = s;
		//std::srand((unsigned)s);
		opensimplex = OpenSimplexNew(s);
	}

	thread_local struct drand48_data threadRand;

	void reseedThread() {
		srand48_r(seed, &threadRand);
	}

	float random() {
		long int result = 0;
		lrand48_r(&threadRand, &result);
		return (float)(result%1000) / 1000.0f;
	}

	int choose(uint range) {
		long int result = 0;
		lrand48_r(&threadRand, &result);
		return result%(int)range;
	}

	void reset() {
		tick = 0;
		seed = 0;
		reseedThread();
		statsElectricityDemand.clear();
		statsElectricitySupply.clear();
		statsEntity.clear();
		statsGhost.clear();
		statsStore.clear();
		statsArm.clear();
		statsCrafter.clear();
		statsConveyor.clear();
		statsUnveyor.clear();
		statsLoader.clear();
		statsRopeway.clear();
		statsRopewayBucket.clear();
		statsProjector.clear();
		statsPath.clear();
		statsVehicle.clear();
		statsPipe.clear();
		statsShunt.clear();
		statsDepot.clear();
		statsDrone.clear();
		statsMissile.clear();
		statsExplosion.clear();
		statsTurret.clear();
		statsComputer.clear();
	}

	void locked(lockCallback cb) {
		mutex.lock();
		cb();
		mutex.unlock();
	}

	double noise2D(double x, double y, int layers, double persistence, double frequency) {
		double amp = 1.0;
		double ampSum = 0.0;
		double result = 0.0;

		for (int i = 0; i < layers; i++) {
			result += OpenSimplexNoise(opensimplex, x*frequency, y*frequency) * amp;
			ampSum += amp;
			amp *= persistence;
			frequency *= 2;
		}

		double noise = result / ampSum;
		noise -= 0.5;
		noise *= 1.5;
		noise += 0.5;
		return std::clamp(noise, 0.0, 1.0);
	}

	bool rayCast(Point a, Point b, float clearance, std::function<bool(uint)> collide) {

		Point n = (b-a).normalize();

		for (Point c = a; c.distance(b) > 1.0f; c += n) {
			for (int eid: Entity::intersecting(c.box().grow(clearance))) {
				if (collide(eid)) {
					return false;
				}
			}
		}

		return true;
	}

	void update() {
		statsElectricityDemand.set(tick, Entity::electricityDemand);
		statsElectricityDemand.update(tick);
		statsElectricitySupply.set(tick, Entity::electricitySupply);
		statsElectricitySupply.update(tick);

		tick++;
		statsEntity.track(tick, Entity::preTick);
		statsGhost.track(tick, Ghost::tick);
		statsPipe.track(tick, Pipe::tick);
		statsStore.track(tick, Store::tick);
		statsArm.track(tick, Arm::tick);
		statsCrafter.track(tick, Crafter::tick);
		statsProjector.track(tick, Projector::tick);
		statsPath.track(tick, Path::tick);
		statsVehicle.track(tick, Vehicle::tick);
		statsConveyor.track(tick, Conveyor::tick);
		statsUnveyor.track(tick, Unveyor::tick);
		statsLoader.track(tick, Loader::tick);
		statsRopeway.track(tick, Ropeway::tick);
		statsRopewayBucket.track(tick, RopewayBucket::tick);
		statsDepot.track(tick, Depot::tick);
		statsDrone.track(tick, Drone::tick);
		statsMissile.track(tick, Missile::tick);
		statsExplosion.track(tick, Explosion::tick);
		statsTurret.track(tick, Turret::tick);
		statsComputer.track(tick, Computer::tick);
	}
}

