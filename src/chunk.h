#pragma once

// Chunks are large square map areas that work mostly independently. They
// terrain and resource layers using a deterministic noise function, and
// are rendered as separate meshes.

struct Chunk;

#include "raylib-ex.h"
#include "item.h"
#include "gridwalk.h"
#include <map>
#include <set>
#include <vector>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>

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
		bool isLand();
		bool isWater();
		bool isHill();
	};

	static inline Chunk* lastChunk = nullptr;

	static XY tileXYtoChunkXY(int x, int y);
	static XY tileXYtoOffsetXY(int x, int y);

	static gridwalk walkTiles(Box);

	static inline std::mutex mutex;
	static inline std::map<XY,Chunk*> all;
	static inline std::set<XY> requested;

	typedef std::function<void(Chunk*)> Generator;
	static inline std::vector<Generator> generators;
	static void generator(Generator fn);

	static Chunk* tryGet(int x, int y);
	static Chunk* tryGet(Point p);
	void generate();
	static Chunk* request(int x, int y);
	static Chunk::Tile* tileTryGet(int x, int y);
	static Chunk::Tile* tileTryGet(Point p);
	static bool isLand(Box b);
	static bool isWater(Box b);
	static bool isHill(Box b);
	static float hillPlatform(Box b);
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

	std::mutex meshMutex;
	Mesh heightmap;
	Mesh newHeightmap;
	bool generated;
	bool meshReady;
	bool meshLoaded;
	bool meshRegenerate;
	bool meshSwitch;
	std::vector<XY> meshMinerals;
	uint64_t meshTickLastViewed;

	Matrix transform;

	Chunk(int cx, int cy);
	~Chunk();

	bool ready();
	void regenerate();
	void backgroundgenerate();
	void terrainNormals();
	void autoload();
	void findHills();
	Point centroid();
	Box box();
};
