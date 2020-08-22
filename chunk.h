#ifndef _H_chunk
#define _H_chunk

struct Chunk;

#include "raylib.h"
#include "raymath.h"
#include "item.h"
#include "box.h"
#include <map>
#include <set>
#include <vector>
#include <functional>

struct Chunk {
	static const int size = 128;

	struct Hill;
	struct Tile;

	struct Hill {
		std::set<Tile*> tiles;
		std::map<uint,uint> minerals;
	};

	static inline std::set<Hill*> hills;

	struct Tile {
		int x, y;
		Hill* hill;
		float elevation;
		Stack mineral;
	};

	struct XY {
		int x, y;
		bool operator==(const XY &o) const;
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

	struct ChunkWalkerIter {
		int cx0, cy0;
		int cx1, cy1;
		int cx, cy;

		typedef XY value_type;
		typedef std::ptrdiff_t difference_type;
		typedef XY* pointer;
		typedef XY& reference;
		typedef std::input_iterator_tag iterator_category;

		XY operator*() const;
		bool operator==(const ChunkWalkerIter& other) const;
		bool operator!=(const ChunkWalkerIter& other) const;
		ChunkWalkerIter& operator++();
	};

	struct ChunkWalker {
		Coords a;
		Coords b;
		ChunkWalker(Box box);
		ChunkWalkerIter begin();
		ChunkWalkerIter end();
	};

	static ChunkWalker walk(Box);

	struct TileWalkerIter {
		int tx0, ty0;
		int tx1, ty1;
		int tx, ty;

		typedef XY value_type;
		typedef std::ptrdiff_t difference_type;
		typedef XY* pointer;
		typedef XY& reference;
		typedef std::input_iterator_tag iterator_category;

		XY operator*() const;
		bool operator==(const TileWalkerIter& other) const;
		bool operator!=(const TileWalkerIter& other) const;
		TileWalkerIter& operator++();
	};

	struct TileWalker {
		Coords a;
		Coords b;
		TileWalker(Box box);
		TileWalkerIter begin();
		TileWalkerIter end();
	};

	static TileWalker walkTiles(Box);

	static inline std::map<XY,Chunk*> all;

	typedef std::function<void(Chunk*)> Generator;
	static inline std::vector<Generator> generators;
	static void generator(Generator fn);

	static Chunk* tryGet(int x, int y);
	static Chunk* get(int x, int y);
	static Chunk::Tile* tileTryGet(int x, int y);
	static Chunk::Tile* tileTryGet(Point p);
	static bool isLand(Box b);
	static bool isWater(Box b);
	static bool isHill(Box b);
	static Stack mine(Box b, uint iid);
	static bool canMine(Box b, uint iid);
	static uint countMine(Box b, uint iid);
	static void flatten(Box b);
	static std::vector<Stack> minables(Box b);
	static std::set<Tile*> hillTiles(Point p);

	static void saveAll(const char* name);
	static void loadAll(const char* name);
	static void reset();

	static inline Material material;

	int x, y;
	Tile tiles[size][size];
	bool regenerate = false;
	Mesh heightmap;
	Matrix transform;
	std::vector<XY> minerals;

	Chunk(int cx, int cy);

	Image heightImage();
	Image colorImage();
	void genHeightMap();
	void dropHeightMap();
	void findHills();
};


#endif