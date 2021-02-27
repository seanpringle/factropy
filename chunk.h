#pragma once

// Chunks are large square map areas that work mostly independently. They
// terrain and resource layers using a deterministic noise function, and
// are rendered as separate meshes.

struct Chunk;

#include "raylib.h"
#include "raymath.h"
#include "item.h"
#include "gridwalk.h"
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

	typedef gridwalk::xy XY;

	struct Tile {
		int x, y;
		Hill* hill;
		float elevation;
		Stack mineral;

		Chunk* chunk();
	};

	static XY tileXYtoChunkXY(int x, int y);
	static XY tileXYtoOffsetXY(int x, int y);

	static gridwalk walkTiles(Box);

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
