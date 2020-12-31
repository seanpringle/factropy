#include "common.h"
#include "spec.h"
#include "entity.h"
#include "chunk.h"
#include "sim.h"
#include "time-series.h"
#include <cstdlib>

namespace Sim {
	OpenSimplex* opensimplex;
	std::mutex mutex;
	uint64_t tick;
	int64_t seed;

	TimeSeries statsElectricityDemand;
	TimeSeries statsElectricitySupply;
	TimeSeries statsEntity;
	TimeSeries statsStore;
	TimeSeries statsArm;
	TimeSeries statsCrafter;
	TimeSeries statsProjector;
	TimeSeries statsPath;
	TimeSeries statsVehicle;
	TimeSeries statsBelt;
	TimeSeries statsLift;
	TimeSeries statsPipe;
	TimeSeries statsShunt;
	TimeSeries statsDepot;
	TimeSeries statsDrone;

	void reset() {
		statsElectricityDemand.clear();
		statsElectricitySupply.clear();
		statsEntity.clear();
		statsStore.clear();
		statsArm.clear();
		statsCrafter.clear();
		statsProjector.clear();
		statsPath.clear();
		statsVehicle.clear();
		statsBelt.clear();
		statsLift.clear();
		statsPipe.clear();
		statsShunt.clear();
		statsDepot.clear();
		statsDrone.clear();
	}

	void locked(lockCallback cb) {
		mutex.lock();
		cb();
		mutex.unlock();
	}

	void reseed(int64_t s) {
		seed = s;
		std::srand((unsigned)s);
		opensimplex = OpenSimplexNew(s);
	}

	float random() {
		return (float)(std::rand()%1000) / 1000.0f;
	}

	int choose(uint range) {
		return std::rand()%(int)range;
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
		return noise;
	}

	void update() {
		statsElectricityDemand.set(tick, Entity::electricityDemand.value);
		statsElectricityDemand.update(tick);
		statsElectricitySupply.set(tick, Entity::electricitySupply.value);
		statsElectricitySupply.update(tick);

		tick++;
		Entity::preTick();
		Ghost::tick();
		statsPipe.track(tick, Pipe::tick);
		statsStore.track(tick, Store::tick);
		statsArm.track(tick, Arm::tick);
		statsCrafter.track(tick, Crafter::tick);
		statsProjector.track(tick, Projector::tick);
		statsPath.track(tick, Path::tick);
		statsVehicle.track(tick, Vehicle::tick);
		statsBelt.track(tick, Belt::tick);
		statsLift.track(tick, Lift::tick);
		//statsShunt.track(tick, Shunt::tick);
		statsDepot.track(tick, Depot::tick);
		statsDrone.track(tick, Drone::tick);
	}
}