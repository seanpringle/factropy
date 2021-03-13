#include "common.h"
#include "chunk.h"
#include "point.h"
#include "part.h"
#include "sim.h"

Chunk::Chunk(int cx, int cy) {
	meshMutex.lock();
	x = cx;
	y = cy;
	XY xy = {x,y};
	all[xy] = this;
	ZERO(heightmap);
	ZERO(newHeightmap);
	ZERO(tiles);
	generated = false;
	meshReady = false;
	meshLoaded = false;
	meshRegenerate = false;
	meshSwitch = false;
	meshTickLastViewed = 0;
	meshMutex.unlock();
}

Chunk::~Chunk() {
	meshMutex.lock();
	if (meshLoaded) {
		rlUnloadMesh(heightmap);
	}
	if (meshReady) {
		UnloadMesh2(heightmap);
	}
	if (meshSwitch) {
		UnloadMesh2(newHeightmap);
	}
	meshMutex.unlock();
}

bool Chunk::ready() {
	return generated;
}

Point Chunk::centroid() {
	return Point(x*size, 0, y*size) + Point(size/2,0,size/2);
}

Box Chunk::box() {
	return centroid().box().grow(size/2);
}

gridwalk Chunk::walkTiles(Box box) {
	return gridwalk(1, box);
}

Chunk::XY Chunk::tileXYtoChunkXY(int x, int y) {
	int cx = std::floor((float)x/Chunk::size);
	int cy = std::floor((float)y/Chunk::size);
	return {cx, cy};
}

Chunk::XY Chunk::tileXYtoOffsetXY(int x, int y) {
	auto [cx, cy] = tileXYtoChunkXY(x, y);
	int ox = x-(cx*Chunk::size);
	int oy = y-(cy*Chunk::size);
	return {ox, oy};
}

Chunk* Chunk::Tile::chunk() {
	auto [cx, cy] = Chunk::tileXYtoChunkXY(x, y);
	return tryGet(cx, cy);
}

void Chunk::generator(Chunk::Generator fn) {
	generators.push_back(fn);
}

Chunk* Chunk::tryGet(int x, int y) {
	mutex.lock();
	// happens a lot when iterating individual tiles
	if (lastChunk && lastChunk->x == x && lastChunk->y == y) {
		mutex.unlock();
		return lastChunk;
	}

	XY xy = {x,y};

	if (all.count(xy)) {
		auto chunk = all[xy];
		if (chunk->ready()) {
			lastChunk = chunk;
			mutex.unlock();
			return chunk;
		}
	}

	mutex.unlock();
	return nullptr;
}

Chunk* Chunk::tryGet(Point p) {
	Tile* tile = tileTryGet(p);
	return tile ? tile->chunk(): nullptr;
}

Chunk* Chunk::request(int x, int y) {
	mutex.lock();
	// happens a lot when iterating individual tiles
	if (lastChunk && lastChunk->x == x && lastChunk->y == y) {
		mutex.unlock();
		return lastChunk;
	}

	XY xy = {x,y};

	if (all.count(xy)) {
		auto chunk = all[xy];
		if (chunk->ready()) {
			lastChunk = chunk;
			mutex.unlock();
			return chunk;
		}
	}
	else {
		requested.insert(xy);
	}

	mutex.unlock();
	return nullptr;
}

Chunk::Tile* Chunk::tileTryGet(int x, int y) {
	auto [cx,cy] = tileXYtoChunkXY(x, y);
	auto [ox,oy] = tileXYtoOffsetXY(x, y);
	Chunk *chunk = tryGet(cx, cy);
	return (chunk != nullptr && chunk->ready()) ? &chunk->tiles[oy][ox]: nullptr;
}

Chunk::Tile* Chunk::tileTryGet(Point p) {
	int x = (int)std::floor(p.x);
	int y = (int)std::floor(p.z); // 3d is Y-up
	return tileTryGet(x, y);
}

void Chunk::generate() {
	notef("generate %d %d (%lu)", x, y, all.size());

	for (int ty = 0; ty < size; ty++) {
		for (int tx = 0; tx < size; tx++) {

			//float elevation = (float)Sim::noise2D(x*size+tx, y*size+ty, 8, 0.5, 0.007) - 0.5f; // -0.5->0.5
			float elevation = (float)Sim::noise2D(x*size+tx, y*size+ty, 8, 0.5, 0.008) - 0.5f; // -0.5->0.5

			elevation += 0.1;

			// floodplain
			if (elevation > 0.0f) {
				if (elevation < 0.3f) {
					elevation = 0.0f;
				} else {
					elevation -= 0.3f;
				}
			}

			double offset = 1000000;

			uint mineral = 0;
			float resource = 0.0f;

			float hint = Sim::random(); //(float)Sim::noise2D(x*size+tx+offset, y*size+ty+offset, 8, 0.6, 0.3);
			offset += 1000000;

			for (auto iid: Item::mining) {
				//float density = (float)Sim::noise2D(x*size+tx+offset, y*size+ty+offset, 8, 0.5, 0.007);
				float density = (float)Sim::noise2D(x*size+tx+offset, y*size+ty+offset, 8, 0.4, 0.005);
				if (density > resource) {
					mineral = iid;
					resource = density;
				}
				offset += 1000000;
			}

			tiles[ty][tx] = (Tile){
				.x = x*size+tx,
				.y = y*size+ty,
				.hill = nullptr,
				.elevation = elevation,
				.mineral = {mineral, (uint)(resource*100.0f)},
			};

			// minables visible on hills
			if (elevation > 0.01f && mineral != 0 && hint > 0.99) {
				meshMinerals.push_back({tx,ty});
			}
		}
	}

	if (tryGet(x-1, y-1)) tryGet(x-1,y-1)->regenerate();
	if (tryGet(x-1, y+0)) tryGet(x-1,y+0)->regenerate();
	if (tryGet(x-1, y+1)) tryGet(x-1,y+1)->regenerate();
	if (tryGet(x+0, y-1)) tryGet(x+0,y-1)->regenerate();

	if (tryGet(x+0, y+1)) tryGet(x+0,y+1)->regenerate();
	if (tryGet(x+1, y-1)) tryGet(x+1,y-1)->regenerate();
	if (tryGet(x+1, y+0)) tryGet(x+1,y+0)->regenerate();
	if (tryGet(x+1, y+1)) tryGet(x+1,y+1)->regenerate();
}

bool Chunk::Tile::isLand() {
	return !isWater() && !isHill();
}

bool Chunk::Tile::isWater() {
	return elevation < -0.001f;
}

bool Chunk::Tile::isHill() {
	return elevation > 0.001f;
}

bool Chunk::isLand(Box b) {
	for (auto [x,y]: walkTiles(b)) {
		Tile *tile = tileTryGet(x, y);
		if (!tile || !tile->isLand()) {
			return false;
		}
	}
	return true;
}

bool Chunk::isWater(Box b) {
	for (auto [x,y]: walkTiles(b)) {
		Tile *tile = tileTryGet(x, y);
		if (!tile || !tile->isWater()) {
			return false;
		}
	}
	return true;
}

bool Chunk::isHill(Box b) {
	for (auto [x,y]: walkTiles(b)) {
		Tile *tile = tileTryGet(x, y);
		if (tile && !tile->isHill()) {
			return false;
		}
	}
	return true;
}

float Chunk::hillPlatform(Box b) {
	float h = -1.0f;
	for (auto [x,y]: walkTiles(b)) {
		Tile *tile = tileTryGet(x, y);
		if (!tile || !tile->isHill()) return 0.0f;
		float e = tile->elevation*100.0f;
		h = h < 0.0f ? e: std::min(e, h);
	}
	return std::round(std::max(h, 0.0f));
}

Stack Chunk::mine(Box b, uint iid) {
	Stack stack = {0,0};
	for (auto [x,y]: walkTiles(b)) {
		Tile *tile = tileTryGet(x, y);
		if (tile && tile->hill) {
			Hill* hill = tile->hill;
			if (hill->minerals.count(iid) && hill->minerals[iid] > 0) {
				for (auto tile: hill->tiles) {
					if (tile->mineral.iid == iid && tile->mineral.size > 0) {
						tile->mineral.size -= 1;
						break;
					}
				}
				hill->minerals[iid] -= 1;
				stack = {iid,1};
			}
			break;
		}
	}
	return stack;
}

bool Chunk::canMine(Box b, uint iid) {
	for (auto [x,y]: walkTiles(b)) {
		Tile *tile = tileTryGet(x, y);
		if (tile && tile->hill && tile->hill->minerals.count(iid) && tile->hill->minerals[iid] > 0) {
			return true;
		}
	}
	return false;
}

uint Chunk::countMine(Box b, uint iid) {
	for (auto [x,y]: walkTiles(b)) {
		Tile *tile = tileTryGet(x, y);
		if (tile && tile->hill && tile->hill->minerals.count(iid)) {
			return tile->hill->minerals[iid];
		}
	}
	return 0;
}

void Chunk::flatten(Box b) {
	for (auto [x,y]: walkTiles(b)) {
		Tile *tile = tileTryGet(x, y);
		if (tile && tile->elevation > 0.0f) {
			tile->elevation = 0.0f;
			tile->chunk()->regenerate();
		}
	}
}

std::vector<Stack> Chunk::minables(Box b) {
	std::vector<Stack> counts;
	for (auto [x,y]: walkTiles(b)) {
		Tile *tile = tileTryGet(x, y);
		if (tile && tile->hill) {
			for (auto [iid,count]: tile->hill->minerals) {
				if (count) counts.push_back({iid,count});
			}
			break;
		}
	}
	return counts;
}

void Chunk::reset() {
	for (auto& [_,chunk]: all) {
		delete chunk;
	}
	for (auto hill: hills) {
		delete hill;
	}
	all.clear();
	hills.clear();
}

void Chunk::terrainNormals() {

	struct Vertex {
		Vector3 v;
		const float epsilon = 0.01;

		bool operator==(const Vertex &o) const {
			bool match = true;
			match = match && std::abs(v.x - o.v.x) < epsilon;
			match = match && std::abs(v.y - o.v.y) < epsilon;
			match = match && std::abs(v.z - o.v.z) < epsilon;
			return match;
		}

		bool operator<(const Vertex &o) const {
			return Vector3Length(v) < Vector3Length(o.v);
		}
	};

	Mesh* mesh = &newHeightmap;

	// Slightly randomize normals so ground appears slightly uneven or rough, even when untextured
	for (int i = 0; i < mesh->vertexCount; i++) {
		Vector3 n = { mesh->normals[i*3 + 0], mesh->normals[i*3 + 1], mesh->normals[i*3 + 2] };
		Vector3 d = { Sim::random()*0.25f, Sim::random()*0.25f, Sim::random()*0.25f };
		n = Vector3Normalize(Vector3Add(n, d));
		mesh->normals[i*3 + 0] = n.x;
		mesh->normals[i*3 + 1] = n.y;
		mesh->normals[i*3 + 2] = n.z;
	}

	// Now sum and average vertex normals so mesh has an overall smooth terrain look
	std::map<Vertex,Vector3> normals;

	for (int i = 0; i < mesh->vertexCount; i++) {
		Vector3 v = { mesh->vertices[i*3 + 0], mesh->vertices[i*3 + 1], mesh->vertices[i*3 + 2] };
		Vector3 n = { mesh->normals[i*3 + 0], mesh->normals[i*3 + 1], mesh->normals[i*3 + 2] };

		Vertex vt = {v};

		if (normals.count(vt)) {
			normals[vt] = Vector3Add(normals[vt], n);
		} else {
			normals[vt] = n;
		}
	}

	for (int i = 0; i < mesh->vertexCount; i++) {
		Vector3 v = { mesh->vertices[i*3 + 0], mesh->vertices[i*3 + 1], mesh->vertices[i*3 + 2] };
		Vertex vt = {v};
		Vector3 n = Vector3Normalize(normals[vt]);
		mesh->normals[i*3 + 0] = n.x;
		mesh->normals[i*3 + 1] = n.y;
		mesh->normals[i*3 + 2] = n.z;
	}
}

void Chunk::regenerate() {
	meshMutex.lock();
	meshRegenerate = true;
	meshMutex.unlock();
}

void Chunk::backgroundgenerate() {
	meshMutex.lock();

	if (!ready() || !meshRegenerate) {
		meshMutex.unlock();
		return;
	}

	notef("backgroundgenerate %d %d", x, y);

	if (meshSwitch) {
		UnloadMesh2(newHeightmap);
		meshSwitch = false;
	}

	meshMutex.unlock();

	// When generating a Mesh from a heightmap each pixel becomes a vertex on a tile centroid.
	// Without the +1 there would be gaps on the +X and +Y edges.
	int edge = size+1;

	float heightmap[edge*edge];

	for (int y = 0; y < edge; y++) {
		for (int x = 0; x < edge; x++) {
			heightmap[y*edge+x] = 0.0f;
			Tile* tile = (x < size && y < size) ? &tiles[y][x]: tileTryGet(this->x*size+x, this->y*size+y);
			if (tile != nullptr) {
				heightmap[y*edge+x] = (tile->elevation+0.5f)*100.0f;
			}
		}
	}

	newHeightmap = GenMeshHeightmap2(size+1, heightmap);

	terrainNormals();

	meshMutex.lock();

	transform = MatrixTranslate((float)(x*size)+0.5f, -50.0, (float)(y*size)+0.5f);

	for (auto it = meshMinerals.begin(); it != meshMinerals.end(); ) {
		Tile* tile = &tiles[it->y][it->x];
		if (tile->elevation < 0.01f) {
			it = meshMinerals.erase(it);
		} else {
			it++;
		}
	}

	meshRegenerate = false;
	meshSwitch = true;

	meshMutex.unlock();
}

void Chunk::autoload() {
	meshMutex.lock();

	if (meshSwitch) {
		if (meshLoaded) {
			rlUnloadMesh(heightmap);
			UnloadMesh2(heightmap);
			meshLoaded = false;
			meshReady = false;
		}
		heightmap = newHeightmap;
		meshSwitch = false;
		meshReady = true;
	}

	if (meshTickLastViewed < Sim::tick-10) {
		if (meshLoaded) {
			rlUnloadMesh(heightmap);
			heightmap.vaoId = 0;
			heightmap.vboId = nullptr;
			meshLoaded = false;
		}
	}
	else {
		if (!meshLoaded && meshReady) {
			rlLoadMesh(&heightmap, false);
			meshLoaded = true;
		}
	}

	meshMutex.unlock();
}

std::set<Chunk::Tile*> Chunk::hillTiles(Point p) {
	std::set<Tile*> tiles;

	std::function<void(int x, int y)> add;

	add = [&](int x, int y) {
		Tile* tile = tileTryGet(x, y);
		if (tile && !tiles.count(tile) && tile->elevation > 0.001f) {
			tiles.insert(tile);

			add(x-1, y-1);
			add(x-1, y-0);
			add(x-1, y+1);
			add(x-0, y-1);
			//add(x-0, y-0);
			add(x-0, y+1);
			add(x+1, y-1);
			add(x+1, y-0);
			add(x+1, y+1);
		}
	};

	add(std::floor(p.x),std::floor(p.z));

	return tiles;
}

void Chunk::findHills() {
	for (int ty = 0; ty < size; ty++) {
		for (int tx = 0; tx < size; tx++) {
			Tile* tile = &tiles[ty][tx];

			if (tile->elevation > 0.001f && tile->hill == nullptr) {
				auto hgroup = hillTiles({(float)tile->x, 0.0f, (float)tile->y});

				std::set<Hill*> oldHills;

				for (auto htile: hgroup) {
					if (htile->hill) {
						oldHills.insert(htile->hill);
					}
				}

				for (auto hill: oldHills) {
					hills.erase(hill);
					delete hill;
				}

				Hill* hill = new Hill;
				hills.insert(hill);

				for (auto htile: hgroup) {
					htile->hill = hill;
					hill->tiles.insert(htile);
					hill->minerals[htile->mineral.iid] += htile->mineral.size;
				}
			}
		}
	}
}

