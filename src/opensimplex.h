#pragma once

// OpenSimplex (Simplectic) Noise in C.
// Ported from Kurt Spencer's java implementation

// 2D only.

#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

typedef struct {
	int16_t *perm;
} OpenSimplex;

OpenSimplex* OpenSimplexNew(int64_t seed);

void OpenSimplexFree(OpenSimplex *os);

double OpenSimplexNoise(OpenSimplex *os, double x, double y);
