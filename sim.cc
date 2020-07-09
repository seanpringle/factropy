#include "common.h"
#include "sim.h"

namespace Sim {
	OpenSimplex* opensimplex;

	void Seed(int64_t seed) {
		opensimplex = OpenSimplexNew(seed);
	}

	double Noise2D(int x, int y, int layers, double persistence, double frequency) {
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
}