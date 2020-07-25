#include "common.h"
#include "chunk.h"
#include "part.h"
#include "sim.h"
#include "json.hpp"
#include <fstream>

bool Chunk::XY::operator==(const XY &o) const {
	return x == o.x && y == o.y;
}

bool Chunk::XY::operator<(const XY &o) const {
	return x < o.x || (x == o.x && y < o.y);
}

Chunk::Coords::Coords(float xx, float yy) {
	x = xx;
	y = yy;
	tx = (int)std::floor(x);
	ty = (int)std::floor(y);
	cx = (int)std::floor((float)tx/Chunk::size);
	cy = (int)std::floor((float)ty/Chunk::size);
	ox = tx - (cx*Chunk::size);
	oy = ty - (cy*Chunk::size);
}

void Chunk::generator(Chunk::Generator fn) {
	generators.push_back(fn);
}

Chunk* Chunk::tryGet(int x, int y) {
	XY xy = {x,y};
	return all.count(xy) == 1 ? all[xy]: NULL;
}

Chunk* Chunk::get(int x, int y) {
	Chunk *chunk = tryGet(x, y);
	if (chunk == NULL) {
		chunk = new Chunk(x, y);

		for (int ty = 0; ty < Chunk::size; ty++) {
			for (int tx = 0; tx < Chunk::size; tx++) {

				float elevation = (float)Sim::noise2D(x*Chunk::size+tx, y*Chunk::size+ty, 8, 0.5, 0.007) - 0.5f; // -0.5->0.5

				elevation += 0.1;

				// floodplain
				if (elevation > 0.0f) {
					if (elevation < 0.3f) {
						elevation = 0.0f;
					} else {
						elevation -= 0.3f;
					}
				}

				chunk->tiles[ty][tx] = (Chunk::Tile){
					elevation : elevation,
				};
			}
		}

		for (auto fn: generators) {
			fn(chunk);
		}
	}
	return chunk;
}

Chunk::Tile* Chunk::tileTryGet(int x, int y) {
	auto co = Chunk::Coords(x, y);
	Chunk *chunk = tryGet(co.cx, co.cy);
	return (chunk != NULL) ? &chunk->tiles[co.oy][co.ox]: NULL;
}

Chunk::Tile* Chunk::tileTryGet(Point p) {
	int x = (int)std::floor(p.x);
	int y = (int)std::floor(p.z); // 3d is Y-up
	return tileTryGet(x, y);
}

bool Chunk::isLand(Box b) {
	for (auto [x,y]: walkTiles(b)) {
		Tile *tile = tileTryGet(x, y);
		if (!tile || tile->elevation > 0.001f || tile->elevation < -0.001f) {
			return false;
		}
	}
	return true;
}

bool Chunk::isWater(Box b) {
	for (auto [x,y]: walkTiles(b)) {
		Tile *tile = tileTryGet(x, y);
		if (!tile || tile->elevation > 0.0f) {
			return false;
		}
	}
	return true;
}

bool Chunk::isHill(Box b) {
	for (auto [x,y]: walkTiles(b)) {
		Tile *tile = tileTryGet(x, y);
		if (tile && tile->elevation > 0.001f) {
			return true;
		}
	}
	return false;
}

Stack Chunk::mine(Box b) {
	for (auto [x,y]: walkTiles(b)) {
		Tile *tile = tileTryGet(x, y);
		if (tile && tile->elevation > 0.001f) {
			auto it = Item::mining.begin();
			std::advance(it, Sim::choose((int)Item::mining.size()));
			return {(*it)->id,1};
		}
	}
	return {0,0};
}

using json = nlohmann::json;

void Chunk::saveAll(const char* name) {
	auto path = std::string(name);
	auto out = std::ofstream(path + "/chunks.json");

	for (auto pair: all) {
		Chunk *chunk = pair.second;
		json state;
		state["x"] = chunk->x;
		state["y"] = chunk->y;
		for (int ty = 0; ty < size; ty++) {
			for (int tx = 0; tx < size; tx++) {
				state["elevations"][ty][tx] = chunk->tiles[ty][tx].elevation;
			}
		}
		out << state << "\n";
	}

	out.close();
}

void Chunk::loadAll(const char* name) {
	auto path = std::string(name);
	auto in = std::ifstream(path + "/chunks.json");

	for (std::string line; std::getline(in, line);) {
		auto state = json::parse(line);
		Chunk *chunk = new Chunk(state["x"], state["y"]);
		for (int ty = 0; ty < size; ty++) {
			for (int tx = 0; tx < size; tx++) {
				chunk->tiles[ty][tx].elevation = state["elevations"][ty][tx];
			}
		}
	}

	in.close();
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
	ZERO(heightmap);
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

				float elevation = tile->elevation;
				//if (elevation < 0.00001f && elevation > -0.00001f) {
				//	elevation += Sim::random()*0.003f;
				//}

				uint8_t intensity = std::clamp(elevation+(0.5f), 0.0f, 1.0f)*255.0f;
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
	Image heightImg = heightImage();
	heightmap = GenMeshHeightmap(heightImg, (Vector3){ size+1, 100, size+1 });
	Part::terrainNormals(&heightmap);
	transform = MatrixTranslate((float)(this->x*size)+0.5f, -49.82, (float)(this->y*size)+0.5f);
	UnloadImage(heightImg);
}

void Chunk::dropHeightMap() {
	UnloadMesh(heightmap);
}

Chunk::ChunkWalker Chunk::walk(Box box) {
	return ChunkWalker(box);
}

Chunk::ChunkWalker::ChunkWalker(Box box) {
	a = Chunk::Coords(box.x-box.w/2, box.z-box.d/2);
	b = Chunk::Coords(box.x+box.w/2, box.z+box.d/2);
	if (a.cy == b.cy || b.ty > b.cy*Chunk::size) {
		b.cy++;
	}
	if (a.cx == b.cx || b.tx > b.cx*Chunk::size) {
		b.cx++;
	}
}

Chunk::ChunkWalkerIter Chunk::ChunkWalker::begin() {
	return (Chunk::ChunkWalkerIter){a.cx, a.cy, b.cx, b.cy, a.cx, a.cy};
}

Chunk::ChunkWalkerIter Chunk::ChunkWalker::end() {
	return (Chunk::ChunkWalkerIter){a.cx, a.cy, b.cx, b.cy, b.cx, b.cy};
}

Chunk::XY Chunk::ChunkWalkerIter::operator*() const {
	return (Chunk::XY){cx, cy};
}

bool Chunk::ChunkWalkerIter::operator==(const Chunk::ChunkWalkerIter& other) const {
	return cx == other.cx && cy == other.cy;
}

bool Chunk::ChunkWalkerIter::operator!=(const Chunk::ChunkWalkerIter& other) const {
	return cx != other.cx || cy != other.cy;
}

Chunk::ChunkWalkerIter& Chunk::ChunkWalkerIter::operator++() {
	if (cx < cx1) {
		cx++;
	}
	if (cx == cx1 && cy < cy1) {
		cy++;
		if (cy < cy1) {
			cx = cx0;
		}
	}
	return *this;
}

Chunk::TileWalker Chunk::walkTiles(Box box) {
	return TileWalker(box);
}

Chunk::TileWalker::TileWalker(Box box) {
	a = Chunk::Coords(box.x-box.w/2, box.z-box.d/2);
	b = Chunk::Coords(box.x+box.w/2, box.z+box.d/2);
	if (a.ty == b.ty || b.y > (float)b.ty) {
		b.ty++;
	}
	if (a.tx == b.tx || b.x > (float)b.tx) {
		b.tx++;
	}
}

Chunk::TileWalkerIter Chunk::TileWalker::begin() {
	return (Chunk::TileWalkerIter){a.tx, a.ty, b.tx, b.ty, a.tx, a.ty};
}

Chunk::TileWalkerIter Chunk::TileWalker::end() {
	return (Chunk::TileWalkerIter){a.tx, a.ty, b.tx, b.ty, b.tx, b.ty};
}

Chunk::XY Chunk::TileWalkerIter::operator*() const {
	return (Chunk::XY){tx, ty};
}

bool Chunk::TileWalkerIter::operator==(const Chunk::TileWalkerIter& other) const {
	return tx == other.tx && ty == other.ty;
}

bool Chunk::TileWalkerIter::operator!=(const Chunk::TileWalkerIter& other) const {
	return tx != other.tx || ty != other.ty;
}

Chunk::TileWalkerIter& Chunk::TileWalkerIter::operator++() {
	if (tx < tx1) {
		tx++;
	}
	if (tx == tx1 && ty < ty1) {
		ty++;
		if (ty < ty1) {
			tx = tx0;
		}
	}
	return *this;
}

