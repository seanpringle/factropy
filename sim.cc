#include "common.h"
#include "spec.h"
#include "entity.h"
#include "sim.h"
#include <filesystem>

namespace Sim {
	OpenSimplex* opensimplex;
	std::mutex mutex;

	void locked(lockCallback cb) {
		mutex.lock();
		cb();
		mutex.unlock();
	}

	void seed(int64_t seed) {
		opensimplex = OpenSimplexNew(seed);
	}

	double noise2D(int x, int y, int layers, double persistence, double frequency) {
		double amp = 1.0;
		double ampSum = 0.0;
		double result = 0.0;

		for (int i = 0; i < layers; i++) {
			result += OpenSimplexNoise(opensimplex, (double)x*frequency, (double)y*frequency) * amp;
			ampSum += amp;
			amp *= persistence;
			frequency *= 2;
		}
		return result / ampSum;
	}

	void save(const char *name) {
		auto path = std::filesystem::path(name);
		std::filesystem::remove_all(path);
	}

	void load(const char *path) {

	}
}