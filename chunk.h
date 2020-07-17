#ifndef _H_chunk
#define _H_chunk

#include "raylib.h"
#include "raymath.h"
#include "box.h"
#include <map>
#include <set>

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

	struct Coords {
		float x;
		float y;
		int tx;
		int ty;
		int cx;
		int cy;
		int ox;
		int oy;

		inline Coords(){};
		Coords(float x, float y);
	};

	struct ChunkBoxIter {
		int cx0, cy0;
		int cx1, cy1;
		int cx, cy;

		typedef XY value_type;
		typedef std::ptrdiff_t difference_type;
		typedef XY* pointer;
		typedef XY& reference;
		typedef std::input_iterator_tag iterator_category;

		XY operator*() const;
		bool operator==(const ChunkBoxIter& other) const;
		bool operator!=(const ChunkBoxIter& other) const;
		ChunkBoxIter& operator++();
	};

	struct ChunkBox {
		Coords a;
		Coords b;
		ChunkBox(Box box);
		ChunkBoxIter begin();
		ChunkBoxIter end();
	};

	static ChunkBox walk(Box);
	static inline std::map<XY,Chunk*> all;

	static Chunk* tryGet(int x, int y);
	static Chunk* get(int x, int y);
	static Chunk::Tile* tileTryGet(int x, int y);

	static void saveAll(const char* name);
	static void loadAll(const char* name);
	static void reset();

	int x, y;
	Tile tiles[size][size];
	std::set<int> entities;
	Model heightmap = {0};

	Chunk(int cx, int cy);

	Image heightImage();
	Image colorImage();
	void genHeightMap();
	void dropHeightMap();
};


#endif