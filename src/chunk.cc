#include "common.h"
#include "chunk.h"
#include "point.h"
#include "part.h"
#include "sim.h"

Point Chunk::centroid() {
	return Point(x*size, 0, y*size) + Point(size/2,0,size/2);
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
	// happens a lot when iterating individual tiles
	if (lastChunk && lastChunk->x == x && lastChunk->y == y) {
		return lastChunk;
	}

	XY xy = {x,y};

	if (all.count(xy)) {
		lastChunk = all[xy];
		return lastChunk;
	}

	return NULL;
}

Chunk* Chunk::get(int x, int y) {
	Chunk *chunk = tryGet(x, y);
	if (chunk == NULL) {
		chunk = new Chunk(x, y);

		for (int ty = 0; ty < Chunk::size; ty++) {
			for (int tx = 0; tx < Chunk::size; tx++) {

				//float elevation = (float)Sim::noise2D(x*Chunk::size+tx, y*Chunk::size+ty, 8, 0.5, 0.007) - 0.5f; // -0.5->0.5
				float elevation = (float)Sim::noise2D(x*Chunk::size+tx, y*Chunk::size+ty, 8, 0.5, 0.008) - 0.5f; // -0.5->0.5

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

				float hint = Sim::random(); //(float)Sim::noise2D(x*Chunk::size+tx+offset, y*Chunk::size+ty+offset, 8, 0.6, 0.3);
				offset += 1000000;

				for (auto iid: Item::mining) {
					//float density = (float)Sim::noise2D(x*Chunk::size+tx+offset, y*Chunk::size+ty+offset, 8, 0.5, 0.007);
					float density = (float)Sim::noise2D(x*Chunk::size+tx+offset, y*Chunk::size+ty+offset, 8, 0.4, 0.005);
					if (density > resource) {
						mineral = iid;
						resource = density;
					}
					offset += 1000000;
				}

				chunk->tiles[ty][tx] = (Tile){
					.x = x*Chunk::size+tx,
					.y = y*Chunk::size+ty,
					.hill = NULL,
					.elevation = elevation,
					.mineral = {mineral, (uint)(resource*100.0f)},
				};

				// minables visible on hills
				if (elevation > 0.01f && mineral != 0 && hint > 0.99) {
					chunk->minerals.push_back({tx,ty});
				}
			}
		}

		for (auto fn: generators) {
			fn(chunk);
		}

		if (tryGet(x-1, y-1)) get(x-1,y-1)->regenerate = true;
		if (tryGet(x-1, y+0)) get(x-1,y+0)->regenerate = true;
		if (tryGet(x-1, y+1)) get(x-1,y+1)->regenerate = true;
		if (tryGet(x+0, y-1)) get(x+0,y-1)->regenerate = true;

		if (tryGet(x+0, y+1)) get(x+0,y+1)->regenerate = true;
		if (tryGet(x+1, y-1)) get(x+1,y-1)->regenerate = true;
		if (tryGet(x+1, y+0)) get(x+1,y+0)->regenerate = true;
		if (tryGet(x+1, y+1)) get(x+1,y+1)->regenerate = true;
	}
	return chunk;
}

Chunk::Tile* Chunk::tileTryGet(int x, int y) {
	auto [cx,cy] = tileXYtoChunkXY(x, y);
	auto [ox,oy] = tileXYtoOffsetXY(x, y);
	Chunk *chunk = tryGet(cx, cy);
	return (chunk != NULL) ? &chunk->tiles[oy][ox]: NULL;
}

Chunk::Tile* Chunk::tileTryGet(Point p) {
	int x = (int)std::floor(p.x);
	int y = (int)std::floor(p.z); // 3d is Y-up
	return tileTryGet(x, y);
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
			tile->chunk()->regenerate = true;
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
	for (auto& pair: all) {
		delete pair.second;
	}
}

Chunk::Chunk(int cx, int cy) {
	x = cx;
	y = cy;
	XY xy = {x,y};
	all[xy] = this;
	regenerate = true;
	ZERO(heightmap);
	ZERO(tiles);
	meshLoaded = false;
	meshGenerated = false;
}

Image Chunk::heightImage() {
	// When generating a Mesh from a heightmap each pixel becomes a vertex on a tile centroid.
	// Without the +1 there would be gaps on the +X and +Y edges.
	int width = size+1;
	int height = size+1;

	Color *pixels = (Color *)RL_MALLOC(width*height*sizeof(Color));

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			Color *pixel = &pixels[y*width+x];
			*pixel = BLANK;
			Tile *tile = tileTryGet(this->x*size+x, this->y*size+y);
			if (tile != NULL) {
				uint8_t intensity = std::clamp(tile->elevation+(0.5f), 0.0f, 1.0f)*255.0f;
				*pixel = (Color){intensity, intensity, intensity, 255};
			}
		}
	}

	Image image = {
		.data = pixels,
		.width = width,
		.height = height,
		.mipmaps = 1,
		.format = UNCOMPRESSED_R8G8B8A8,
	};

	return image;
}

void Chunk::genHeightMap() {
	dropHeightMap();

	Image heightImg = heightImage();
	heightmap = GenMeshHeightmap(heightImg, (Vector3){ size+1, 100, size+1 });
	Part::terrainNormals(&heightmap);
	transform = MatrixTranslate((float)(x*size)+0.5f, -49.82, (float)(y*size)+0.5f);
	UnloadImage(heightImg);
	regenerate = false;

	for (auto it = minerals.begin(); it != minerals.end(); ) {
		Tile* tile = &tiles[it->y][it->x];
		if (tile->elevation < 0.01f) {
			it = minerals.erase(it);
		} else {
			it++;
		}
	}

  rlUnloadMesh(heightmap);
	heightmap.vaoId = 0;
	heightmap.vboId = nullptr;
  meshLoaded = false;
  meshGenerated = true;
}

void Chunk::dropHeightMap() {
	if (meshGenerated) {
		loadMesh();
		UnloadMesh(heightmap);
		meshLoaded = false;
		meshGenerated = false;
	}
}

void Chunk::loadMesh() {
	if (!meshLoaded) {
		rlLoadMesh(&heightmap, false);
		meshLoaded = true;
	}
}

void Chunk::unloadMesh() {
	if (meshLoaded) {
		rlUnloadMesh(heightmap);
		heightmap.vaoId = 0;
		heightmap.vboId = nullptr;
		meshLoaded = false;
	}
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

			if (tile->elevation > 0.001f && tile->hill == NULL) {
				auto hgroup = hillTiles({(float)tile->x, 0.0f, (float)tile->y});

				for (auto htile: hgroup) {
					if (htile && htile->hill) {
						tile->hill = htile->hill;
						tile->hill->tiles.insert(tile);
						break;
					}
				}

				Hill* hill = tile->hill;
				if (!hill) {
					hill = new Hill;
					hills.insert(hill);
				}

				for (auto htile: hgroup) {
					ensure(htile->hill == hill || htile->hill == NULL);
					if (!htile->hill) {
						htile->hill = hill;
						hill->tiles.insert(htile);
						hill->minerals[htile->mineral.iid] += htile->mineral.size;
					}
				}
			}
		}
	}
}

