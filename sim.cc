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

	TimeSeries statsEntity;
	TimeSeries statsArm;
	TimeSeries statsCrafter;
	TimeSeries statsPath;
	TimeSeries statsVehicle;
	TimeSeries statsBelt;
	TimeSeries statsLift;

	void reset() {
		statsEntity.clear();
		statsArm.clear();
		statsCrafter.clear();
		statsPath.clear();
		statsVehicle.clear();
		statsBelt.clear();
		statsLift.clear();
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
		tick++;
		Entity::preTick();
		statsArm.track(tick, Arm::tick);
		statsCrafter.track(tick, Crafter::tick);
		statsPath.track(tick, Path::tick);
		statsVehicle.track(tick, Vehicle::tick);
		statsBelt.track(tick, Belt::tick);
		statsLift.track(tick, Lift::tick);
	}
}