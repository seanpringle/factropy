#include "common.h"
#include "sim.h"
#include "view.h"

namespace {

	const float speedH = 0.005f;
	const float speedV = 0.005f;
	const float speedW = 0.25f;

	void drawGrid(Vector3 p, int slices, float spacing) {
		int halfSlices = slices/2;

		if (rlCheckBufferLimit(slices*4)) rlglDraw();

		rlBegin(RL_LINES);
			for (int i = -halfSlices; i <= halfSlices; i++) {
				if (i == 0) {
					rlColor3f(0.5f, 0.5f, 0.5f);
					rlColor3f(0.5f, 0.5f, 0.5f);
					rlColor3f(0.5f, 0.5f, 0.5f);
					rlColor3f(0.5f, 0.5f, 0.5f);
				}
				else {
					rlColor3f(0.75f, 0.75f, 0.75f);
					rlColor3f(0.75f, 0.75f, 0.75f);
					rlColor3f(0.75f, 0.75f, 0.75f);
					rlColor3f(0.75f, 0.75f, 0.75f);
				}

				rlVertex3f(p.x+(float)i*spacing, p.y, p.z+(float)-halfSlices*spacing);
				rlVertex3f(p.x+(float)i*spacing, p.y, p.z+(float)halfSlices*spacing);

				rlVertex3f(p.x+(float)-halfSlices*spacing, p.y, p.z+(float)i*spacing);
				rlVertex3f(p.x+(float)halfSlices*spacing, p.y, p.z+(float)i*spacing);
			}
		rlEnd();
	}
}

MainCamera::MainCamera(Vector3 ppos, Vector3 tar) {
	position = ppos;
	target = tar;
	up = (Vector3){0,1,0};

	mouse = {0};
	showGrid = false;

	hovering = NULL;
	placing = NULL;

	popup = NULL;
	buildPopup = NULL;
	entityPopup = NULL;
	popupFocused = false;
	worldFocused = true;
}

void MainCamera::lookAt(Point p) {
	Vector3 delta = Vector3Subtract(p.vec(), target);
	target = p.vec();
	position = Vector3Add(position, delta);
}

void MainCamera::build(Spec* spec) {
	delete placing;
	placing = NULL;
	if (spec) {
		placing = new GuiFakeEntity(spec);
	}
}

void MainCamera::updateMouseState() {
	MouseState last = mouse;

	Vector2 pos = GetMousePosition();
	bool left = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
	bool middle = IsMouseButtonDown(MOUSE_MIDDLE_BUTTON);
	bool right = IsMouseButtonDown(MOUSE_RIGHT_BUTTON);

	mouse = (MouseState) {
		x : (int)pos.x,
		y : (int)pos.y,
		dx : (int)pos.x - last.x,
		dy : (int)pos.y - last.y,
		wheel : GetMouseWheelMove(),
		left : left,
		leftChanged : left != last.left,
		middle : middle,
		middleChanged : middle != last.middle,
		right : right,
		rightChanged : right != last.right,
		rH : last.rH,
		rV : last.rV,
		zW : last.zW,
	};

	popupFocused = popup ? popup->contains(mouse.x, mouse.y): false;
	worldFocused = !popupFocused;

	if (mouse.right) {
		mouse.rH += speedH * (float)mouse.dx;
		mouse.rV += speedV * (float)mouse.dy;
	}

	if (mouse.wheel) {
		bool tooFar = mouse.wheel < 0 && Vector3Distance(position, target) > 100;
		bool tooClose = mouse.wheel > 0 && Vector3Distance(position, target) < 5;
		if (!tooClose && !tooFar) {
			mouse.zW += speedW * -(float)mouse.wheel;
		}
	}
}

void MainCamera::updateCamera() {
	if (mouse.rH > 0.0f || mouse.rH < 0.0f || mouse.rV > 0.0f || mouse.rV < 0.0f) {
		float rDH = 0.0f;
		float rDV = 0.0f;

		if (fabs(mouse.rH) < 0.0001) {
			rDH = mouse.rH;
			mouse.rH = 0.0f;
		} else {
			float s = fabs(mouse.rH)/4.0;
			rDH = mouse.rH > 0 ? s: -s;
			mouse.rH -= rDH;
		}

		if (fabs(mouse.rV) < 0.0001) {
			rDV = mouse.rV;
			mouse.rV = 0.0f;
		} else {
			float s = fabs(mouse.rV)/4.0;
			rDV = mouse.rV > 0 ? s: -s;
			mouse.rV -= rDV;
		}

		Vector3 direction = Vector3Subtract(position, target);
		Vector3 right = Vector3Normalize(Vector3CrossProduct(direction, up));
		Matrix rotateH = MatrixRotate(up, -rDH);
		Matrix rotateV = MatrixRotate(right, rDV);
		Matrix rotate = MatrixMultiply(rotateH, rotateV);
		direction = Vector3Transform(direction, rotate);
		position = Vector3Add(target, direction);
	}

	if (mouse.zW > 0.0f || mouse.zW < 0.0f) {
		float zDW = 0.0f;

		float d = fabs(mouse.zW);
		float s = d/4.0;

		if (fabs(mouse.zW) < 0.01) {
			zDW = mouse.zW;
			mouse.zW = 0.0f;
		} else {
			zDW = mouse.zW > 0 ? s: -s;
			mouse.zW -= zDW;
		}

		Vector3 direction = Vector3Subtract(position, target);
		Vector3 delta = Vector3Scale(direction, zDW);
		position = Vector3Add(position, delta);
	}

	position.y = std::max(1.0f, position.y);

	Camera3D camera = {
		position : position,
		target   : target,
		up       : up,
		fovy     : fovy,
		type     : CAMERA_PERSPECTIVE,
	};

	mouse.ray = GetMouseRay((Vector2){(float)mouse.x, (float)mouse.y}, camera);
}

void MainCamera::update() {
	hovering = NULL;
	hovered.clear();

	while (entities.size() > 0) {
		delete entities.back();
		entities.pop_back();
	}

	updateMouseState();
	updateCamera();

	Box view = (Box){target.x, target.y, target.z, 500, 500, 500};

	Sim::locked([&]() {
		for (int id: Entity::intersecting(view)) {
			entities.push_back(new GuiEntity(id));
		}
	});

	if (worldFocused) {

		for (auto ge: entities) {
			if (CheckCollisionRayBox(mouse.ray, ge->box().bounds())) {
				hovered.push_back(ge);
			}
		}

		if (hovered.size() > 0) {
			float distance = 0.0f;
			for (auto ge: hovered) {
				float d = Vector3Distance(ge->pos.vec(), position);
				if (hovering == NULL || d < distance) {
					hovering = ge;
					distance = d;
				}
			}
		}

		if (placing) {
			RayHitInfo hit = GetCollisionRayGround(mouse.ray, 0);
			placing->move(Point::fromVec(hit.position))->floor(placing->pos.y);
		}
	}
}

void MainCamera::draw() {

	Camera3D camera = {
		position : position,
		target   : target,
		up       : up,
		fovy     : fovy,
		type     : CAMERA_PERSPECTIVE,
	};

	ClearBackground(SKYBLUE);

	BeginMode3D(camera);

		if (showGrid) {
			Vector3 groundZero = {
				std::floor(target.x),
				std::floor(0),
				std::floor(target.z),
			};
			drawGrid(groundZero, 64, 1);
		}

		std::vector<Matrix> water;
		std::vector<Mesh> chunk_meshes;
		std::vector<Matrix> chunk_transforms;

		float size = (float)Chunk::size;
		for (auto pair: Chunk::all) {
			Chunk *chunk = pair.second;
			float x = chunk->x;
			float y = chunk->y;
			if (!chunk->generated) {
				chunk->genHeightMap();
				chunk->generated = true;
			}
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
			Matrix t = ge->transform();
			for (auto part: ge->spec->parts) {
				batches[part].push_back(t);
			}
		}

		for (auto pair: batches) {
			Part *part = pair.first;
			part->drawInstanced(pair.second.size(), pair.second.data());
		}

		if (hovering) {
			Spec::Animation* animation = &hovering->spec->animations[hovering->dir];
			Vector3 bounds = (Vector3){animation->w, animation->h, animation->d};
			DrawCubeWiresV(hovering->pos.vec(), Vector3AddValue(bounds, 0.01), WHITE);
		}

		if (placing) {
			Matrix t = placing->transform();
			for (auto part: placing->spec->parts) {
				part->drawGhost(t);
			}
		}

	EndMode3D();

	if (popup) {
		popup->draw();
	}
}