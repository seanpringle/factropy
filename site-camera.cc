#include "common.h"
#include "sim.h"
#include "view.h"

SiteCamera::SiteCamera(Point ppos, Point ddir) {
	pos = ppos;
	dir = ddir;
	refresh = 60;
}

SiteCamera::~SiteCamera() {
	while (entities.size() > 0) {
		delete entities.back();
		entities.pop_back();
	}
}

void SiteCamera::update() {
	if (Sim::tick%refresh != 0) return;

	while (entities.size() > 0) {
		delete entities.back();
		entities.pop_back();
	}

	Point target = pos + dir;
	Box view = (Box){target.x, target.y, target.z, 500, 500, 500};

	Sim::locked([&]() {
		for (int id: Entity::intersecting(view)) {
			entities.push_back(new GuiEntity(id));
		}
	});
}

void SiteCamera::draw(RenderTexture canvas) {
	if (Sim::tick%refresh != 0) return;

	Camera3D camera = {
		position : pos,
		target   : pos + dir,
		up       : -Point::Up(),
		fovy     : -fovy,
		type     : CAMERA_PERSPECTIVE,
	};

	BeginTextureMode(canvas);

		ClearBackground(GRAY);

		BeginMode3D(camera);

			std::vector<Matrix> water;
			std::vector<Mesh> chunk_meshes;
			std::vector<Matrix> chunk_transforms;

			float size = (float)Chunk::size;
			for (auto pair: Chunk::all) {
				Chunk *chunk = pair.second;
				float x = chunk->x;
				float y = chunk->y;
				chunk_meshes.push_back(chunk->heightmap);
				chunk_transforms.push_back(chunk->transform);
				water.push_back(
					MatrixMultiply(
						MatrixTranslate(x+0.5f, -0.52f, y+0.5f),
						MatrixScale(size,size,size)
					)
				);
			}

			rlDrawMaterialMeshes(Chunk::material, chunk_meshes.size(), chunk_meshes.data(), chunk_transforms.data());
			rlDrawMeshInstanced(waterCube.meshes[0], waterCube.materials[0], water.size(), water.data());

			std::map<Part*,std::vector<Matrix>> batches;

			for (auto ge: entities) {
				for (uint i = 0; i < ge->spec->parts.size(); i++) {
					Part *part = ge->spec->parts[i];
					batches[part].push_back(part->instance(ge->spec, i, ge->state, ge->transform));
				}
			}

			for (auto pair: batches) {
				Part *part = pair.first;
				part->drawInstanced(false, pair.second.size(), pair.second.data());
			}

		EndMode3D();

	EndTextureMode();
}

Point SiteCamera::groundTarget(float ground) {
	RayHitInfo spot = GetCollisionRayGround((Ray){pos, dir}, ground);
	return Point(spot.position);
}