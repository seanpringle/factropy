#include "common.h"
#include "chunk.h"
#include "sim.h"
#include "json.hpp"
#include <filesystem>
#include <fstream>

bool Chunk::XY::operator=(const XY &o) const {
	return x == o.x && y == o.y;
}

bool Chunk::XY::operator<(const XY &o) const {
	return x < o.x || (x == o.x && y < o.y);
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

				double n = Sim::noise2D(x*Chunk::size+tx, y*Chunk::size+ty, 8, 0.5, 0.007);
				n -= 0.5;
				n *= 1.5;
				n += 0.5;

				int elevation = 2;

				if (n < 0.2) {
					elevation = -2;
				}
				else
				if (n < 0.4) {
					elevation = -1;
				}
				else
				if (n < 0.6) {
					elevation = 0;
				}
				else
				if (n < 0.8) {
					elevation = 1;
				}

				chunk->tiles[ty][tx] = (Chunk::Tile){.elevation = elevation};
			}
		}
	}
	return chunk;
}

Chunk::Tile* Chunk::tileTryGet(int x, int y) {
	int cx = (int)floor((float)x/Chunk::size);
	int cy = (int)floor((float)y/Chunk::size);
	Chunk *chunk = tryGet(cx, cy);
	int ox = x - (cx*Chunk::size);
	int oy = y - (cy*Chunk::size);
	return (chunk != NULL) ? &chunk->tiles[oy][ox]: NULL;
}

namespace fs = std::filesystem;
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
				uint8_t intensity = 0;
				switch (tile->elevation) {
					case -2: {
						intensity = 0;
						break;
					}
					case -1: {
						intensity = 128;
						break;
					}
					case 0: {
						intensity = 255;
						break;
					}
					case 1: {
						intensity = 255;
						break;
					}
					case 2: {
						intensity = 255;
						break;
					}
				}
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

Image Chunk::colorImage() {
	// When generating a Mesh from a heightmap each pixel becomes a vertex on a tile centroid.
	// Without the +1 there would be gaps on the +X and +Y edges.
	int width = size+1;
	int height = size+1;

	Color colors[5] = {
		GetColor(0x45320AFF),
		GetColor(0x85421AFF),
		GetColor(0xA5622AFF),
		GetColor(0xE18339FF),
		GetColor(0xE6A644FF),
	};

	Color *pixels = (Color *)RL_MALLOC(width*height*sizeof(Color));

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			Color *pixel = &pixels[y*width+x];
			Tile *tile = tileTryGet(this->x*size+x, this->y*size+y);
			*pixel = BLANK;
			if (tile != NULL) {
				*pixel = colors[tile->elevation+2];
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
	Image colorImg = colorImage();

	Mesh mesh = GenMeshHeightmap(heightImg, (Vector3){ size+1, 3, size+1 });
	Model model = LoadModelFromMesh(mesh);

	Texture2D texture = LoadTextureFromImage(colorImg);
	model.materials[0].maps[MAP_DIFFUSE].texture = texture;

	//model.transform = MatrixMultiply(model.transform, MatrixScale(1.0, 1.0, 1.0));

	// Move to chunk position
	model.transform = MatrixMultiply(model.transform, MatrixTranslate(this->x*size, -3.1, this->y*size));

	UnloadImage(heightImg);
	UnloadImage(colorImg);

	heightmap = model;
}

void Chunk::dropHeightMap() {
	UnloadModel(heightmap);
}
