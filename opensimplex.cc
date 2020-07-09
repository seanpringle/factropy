
// OpenSimplex (Simplectic) Noise in C.
// Ported from Kurt Spencer's java implementation

// 2D only.

#include "opensimplex.h"

#define STRETCH -0.211324865405187 // (1 / sqrt(2 + 1) - 1 ) / 2;
#define SQUISH 0.366025403784439 // (sqrt(2 + 1) -1) / 2;
#define NORM 47.0
#define	NORMMIN 0.864366441
#define	NORMSCALE 0.5784583670

/*
 * Gradients approximate the directions to the
 * vertices of an octagon from the center.
 */
static const int8_t gradients[] = {
	 5,  2,  2,  5,
	-5,  2, -2,  5,
	 5, -2,  2, -5,
	-5, -2, -2, -5,
};

static double opensimplexExtrapolate(OpenSimplex *os, int xsb, int ysb, double dx, double dy) {
	int16_t *perm = os->perm;
	int index = perm[(perm[xsb & 0xFF] + ysb) & 0xFF] & 0x0E;
	return gradients[index] * dx + gradients[index + 1] * dy;
}

static int opensimplexFloor(double x) {
	int xi = (int) x;
	return x < xi ? xi - 1 : xi;
}

/*
 * Initializes using a permutation array generated from a 64-bit seed.
 * Generates a proper permutation (i.e. doesn't merely perform N successive pair
 * swaps on a base array).  Uses a simple 64-bit LCG.
 */
OpenSimplex* OpenSimplexNew(int64_t seed) {
	OpenSimplex *os = (OpenSimplex*)calloc(1, sizeof(OpenSimplex));
	os->perm = (int16_t*)calloc(256, sizeof(int16_t));

	int16_t *perm = os->perm;
	int16_t source[256];

	for (int i = 0; i < 256; i++) {
		source[i] = (int16_t) i;
	}

	seed = seed * 6364136223846793005LL + 1442695040888963407LL;
	seed = seed * 6364136223846793005LL + 1442695040888963407LL;
	seed = seed * 6364136223846793005LL + 1442695040888963407LL;

	for (int32_t i = 255; i >= 0; i--) {
		seed = seed * 6364136223846793005LL + 1442695040888963407LL;
		int32_t r = (int32_t)((seed + 31) % (i + 1));
		if (r < 0) {
			r += (i + 1);
		}
		perm[i] = source[r];
		source[r] = source[i];
	}

	return os;
}

void OpenSimplexFree(OpenSimplex *os) {
	free(os->perm);
	free(os);
}

/* 2D OpenSimplex (Simplectic) Noise. */
double OpenSimplexNoise(OpenSimplex *os, double x, double y) {

	/* Place input coordinates onto grid. */
	double stretchOffset = (x + y) * STRETCH;
	double xs = x + stretchOffset;
	double ys = y + stretchOffset;

	/* Floor to get grid coordinates of rhombus (stretched square) super-cell origin. */
	int xsb = opensimplexFloor(xs);
	int ysb = opensimplexFloor(ys);

	/* Skew out to get actual coordinates of rhombus origin. We'll need these later. */
	double squishOffset = (xsb + ysb) * SQUISH;
	double xb = xsb + squishOffset;
	double yb = ysb + squishOffset;

	/* Compute grid coordinates relative to rhombus origin. */
	double xins = xs - xsb;
	double yins = ys - ysb;

	/* Sum those together to get a value that determines which region we're in. */
	double inSum = xins + yins;

	/* Positions relative to origin point. */
	double dx0 = x - xb;
	double dy0 = y - yb;

	/* We'll be defining these inside the next block and using them afterwards. */
	double dx_ext, dy_ext;
	int xsv_ext, ysv_ext;

	double dx1;
	double dy1;
	double attn1;
	double dx2;
	double dy2;
	double attn2;
	double zins;
	double attn0;
	double attn_ext;

	double value = 0;

	/* Contribution (1,0) */
	dx1 = dx0 - 1 - SQUISH;
	dy1 = dy0 - 0 - SQUISH;
	attn1 = 2 - dx1 * dx1 - dy1 * dy1;
	if (attn1 > 0) {
		attn1 *= attn1;
		value += attn1 * attn1 * opensimplexExtrapolate(os, xsb + 1, ysb + 0, dx1, dy1);
	}

	/* Contribution (0,1) */
	dx2 = dx0 - 0 - SQUISH;
	dy2 = dy0 - 1 - SQUISH;
	attn2 = 2 - dx2 * dx2 - dy2 * dy2;
	if (attn2 > 0) {
		attn2 *= attn2;
		value += attn2 * attn2 * opensimplexExtrapolate(os, xsb + 0, ysb + 1, dx2, dy2);
	}

	if (inSum <= 1) { /* We're inside the triangle (2-Simplex) at (0,0) */
		zins = 1 - inSum;
		if (zins > xins || zins > yins) { /* (0,0) is one of the closest two triangular vertices */
			if (xins > yins) {
				xsv_ext = xsb + 1;
				ysv_ext = ysb - 1;
				dx_ext = dx0 - 1;
				dy_ext = dy0 + 1;
			} else {
				xsv_ext = xsb - 1;
				ysv_ext = ysb + 1;
				dx_ext = dx0 + 1;
				dy_ext = dy0 - 1;
			}
		} else { /* (1,0) and (0,1) are the closest two vertices. */
			xsv_ext = xsb + 1;
			ysv_ext = ysb + 1;
			dx_ext = dx0 - 1 - 2 * SQUISH;
			dy_ext = dy0 - 1 - 2 * SQUISH;
		}
	} else { /* We're inside the triangle (2-Simplex) at (1,1) */
		zins = 2 - inSum;
		if (zins < xins || zins < yins) { /* (0,0) is one of the closest two triangular vertices */
			if (xins > yins) {
				xsv_ext = xsb + 2;
				ysv_ext = ysb + 0;
				dx_ext = dx0 - 2 - 2 * SQUISH;
				dy_ext = dy0 + 0 - 2 * SQUISH;
			} else {
				xsv_ext = xsb + 0;
				ysv_ext = ysb + 2;
				dx_ext = dx0 + 0 - 2 * SQUISH;
				dy_ext = dy0 - 2 - 2 * SQUISH;
			}
		} else { /* (1,0) and (0,1) are the closest two vertices. */
			dx_ext = dx0;
			dy_ext = dy0;
			xsv_ext = xsb;
			ysv_ext = ysb;
		}
		xsb += 1;
		ysb += 1;
		dx0 = dx0 - 1 - 2 * SQUISH;
		dy0 = dy0 - 1 - 2 * SQUISH;
	}

	/* Contribution (0,0) or (1,1) */
	attn0 = 2 - dx0 * dx0 - dy0 * dy0;
	if (attn0 > 0) {
		attn0 *= attn0;
		value += attn0 * attn0 * opensimplexExtrapolate(os, xsb, ysb, dx0, dy0);
	}

	/* Extra Vertex */
	attn_ext = 2 - dx_ext * dx_ext - dy_ext * dy_ext;
	if (attn_ext > 0) {
		attn_ext *= attn_ext;
		value += attn_ext * attn_ext * opensimplexExtrapolate(os, xsv_ext, ysv_ext, dx_ext, dy_ext);
	}

	return ((value / NORM) + NORMMIN) * NORMSCALE;
}
