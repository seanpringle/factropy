#include "common.h"
#include "spec.h"
#include "entity.h"
#include "chunk.h"
#include "sim.h"
#include <filesystem>

namespace Sim {
	OpenSimplex* opensimplex;
	std::mutex mutex;
	uint64_t tick;

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

	namespace fs = std::filesystem;

	void save(const char *name) {
		auto path = fs::path(name);
		fs::remove_all(path);
		fs::create_directory(path);
		Spec::saveAll(name);
		Chunk::saveAll(name);
		Entity::saveAll(name);
	}

	void load(const char *name) {
		auto path = fs::path(name);
		Spec::loadAll(name);
		Chunk::loadAll(name);
		Entity::loadAll(name);
	}

	void update() {
		tick++;
	}
}