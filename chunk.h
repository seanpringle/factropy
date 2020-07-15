#ifndef _H_chunk
#define _H_chunk

#include "raylib.h"
#include "raymath.h"
#include <map>

struct Chunk {
	static const int size = 32;

	struct Tile {
		int elevation;
	};
		
	struct XY {
		int x, y;
		bool operator=(const XY &o) const;
		bool operator<(const XY &o) const;
	};

	static inline std::map<XY,Chunk*> all;

	static Chunk* tryGet(int x, int y);
	static Chunk* get(int x, int y);
	static Chunk::Tile* tileTryGet(int x, int y);

	static void saveAll(const char* name);
	static void loadAll(const char* name);
	static void reset();

	int x, y;
	Tile tiles[size][size];
	Model heightmap;

	Chunk(int cx, int cy);

	Image heightImage();
	Image colorImage();
	void genHeightMap();
	void dropHeightMap();
};

#endif