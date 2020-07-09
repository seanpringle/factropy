#ifndef _H_chunk
#define _H_chunk

#include <map>

struct Chunk {
	static const int size = 32;

	struct Tile {
		float elevation;
	};

	int x, y;
	Tile tiles[size][size];

	Chunk(int cx, int cy);
};

namespace Chunks {
	struct XY {
		int x, y;
		bool operator=(const XY &o) const;
		bool operator<(const XY &o) const;
	};

	extern std::map<XY,Chunk*> all;

	Chunk* tryGet(int x, int y);

	Chunk* get(int x, int y);
}

#endif