#include "common.h"
#include "sim.h"
#include "view.h"

SiteCamera::SiteCamera(Vector3 ppos, Vector3 ddir) {
	pos = ppos;
	dir = ddir;
	refresh = 60;
}

void SiteCamera::update() {
	if (Sim::tick%refresh != 0) return;

	while (entities.size() > 0) {
		delete entities.back();
		entities.pop_back();
	}

	Vector3 target = Vector3Add(pos, dir);
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
		target   : Vector3Add(pos, dir),
		up       : (Vector3){0,-1,0},
		fovy     : -fovy,
		type     : CAMERA_PERSPECTIVE,
	};

	BeginTextureMode(canvas);

		ClearBackground(SKYBLUE);

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
				for (auto part: ge->spec->parts) {
					batches[part].push_back(part->instance(ge));
				}
			}

			for (auto pair: batches) {
				Part *part = pair.first;
				part->drawInstanced(pair.second.size(), pair.second.data());
			}

		EndMode3D();

	EndTextureMode();
}
