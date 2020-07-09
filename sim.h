#ifndef _H_sim
#define _H_sim

#include "opensimplex.h"

namespace Sim {
	extern OpenSimplex* opensimplex;

	void Seed(int64_t seed);
	double Noise2D(int x, int y, int layers, double persistence, double frequency);
}

#endif
