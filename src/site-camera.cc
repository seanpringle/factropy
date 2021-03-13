#include "common.h"
#include "sim.h"
#include "view.h"
#include "mat4.h"

SiteCamera::SiteCamera(Point ppos, Point ddir) {
	pos = ppos;
	dir = ddir.normalize();
	refresh = 60;
}

SiteCamera::~SiteCamera() {
	while (entities.size() > 0) {
		delete entities.back();
		entities.pop_back();
	}
}

Point SiteCamera::groundTarget(float ground) {
	RayHitInfo spot = GetCollisionRayGround((Ray){pos, dir}, ground);
	return Point(spot.position);
}

void SiteCamera::update(bool worldFocused) {
	if (Sim::tick%refresh != 0) return;

	while (entities.size() > 0) {
		delete entities.back();
		entities.pop_back();
	}

	Point target = groundTarget(0.0f);
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
		.position = pos,
		.target   = groundTarget(0.0f),
		.up       = Point::Up,
		.fovy     = fovy,
		.type     = CAMERA_PERSPECTIVE,
	};

	Box view = groundTarget(0.0f).box().grow(Chunk::size*3);

	BeginTextureMode(canvas);

		ClearBackground(SKYBLUE);

		BeginMode3D(camera);

			std::vector<Mat4> water;
			std::vector<Mesh> chunk_meshes;
			std::vector<Mat4> chunk_transforms;

			float size = (float)Chunk::size;
			for (auto [cx,cy]: gridwalk(Chunk::size, view)) {
				Chunk* chunk = Chunk::request(cx, cy);
				if (!chunk || !chunk->ready()) continue;
				chunk->meshTickLastViewed = Sim::tick;
				chunk->autoload();

				chunk->meshMutex.lock();

				if (chunk->meshLoaded) {
					float x = chunk->x;
					float y = chunk->y;
					chunk_meshes.push_back(chunk->heightmap);
					chunk_transforms.push_back(chunk->transform);
					water.push_back(
						Mat4::translate(x+0.5f, -0.52f, y+0.5f) * Mat4::scale(size,size,size)
					);
				}

				chunk->meshMutex.unlock();
			}

			rlDrawMaterialMeshes(Chunk::material, chunk_meshes.size(), chunk_meshes.data(), chunk_transforms.data());
			rlDrawMeshInstanced2(waterCube.meshes[0], waterCube.materials[0], water.size(), water.data());

			std::map<Part*,std::vector<Mat4>> batches;

			for (auto ge: entities) {
				for (uint i = 0; i < ge->spec->parts.size(); i++) {
					Part *part = ge->spec->parts[i];
					batches[part].push_back(part->specInstance(ge->spec, i, ge->state, ge->transform));
				}
			}

			for (auto pair: batches) {
				Part *part = pair.first;
				part->drawInstanced(false, pair.second.size(), pair.second.data());
			}

		EndMode3D();

	EndTextureMode();
}
