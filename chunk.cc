#include "common.h"
#include "chunk.h"
#include "sim.h"

namespace Chunks {

	bool XY::operator=(const XY &o) const {
		return x == o.x && y == o.y;
	}

	bool XY::operator<(const XY &o) const {
		return x < o.x || (x == o.x && y < o.y);
	}

	std::map<XY,Chunk*> all;

	Chunk* tryGet(int x, int y) {
		XY xy = {x,y};
		return all.count(xy) == 1 ? all[xy]: NULL;
	}

	Chunk* get(int x, int y) {
		Chunk *chunk = tryGet(x, y);
		if (chunk == NULL) {
			XY xy = {x,y};
			chunk = new Chunk(x, y);
			all[xy] = chunk;
			notef("Chunk generate %d %d", x, y);

			for (int ty = 0; ty < Chunk::size; ty++) {
				for (int tx = 0; tx < Chunk::size; tx++) {
					chunk->tiles[ty][tx] = (Chunk::Tile){
						.elevation = (float)Sim::Noise2D(x*Chunk::size+tx, y*Chunk::size+ty, 8, 0.5, 0.007),
					};
				}
			}
		}
		return chunk;
	}
}

using namespace Chunks;

Chunk::Chunk(int cx, int cy) {
	x = cx;
	y = cy;
}
