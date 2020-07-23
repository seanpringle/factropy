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

MainCamera::MainCamera(Vector3 pos, Vector3 dir) {
	position = pos;
	nextPosition = pos;
	direction = dir;
	nextDirection = dir;
	up = (Vector3){0,1,0};

	ZERO(mouse);
	showGrid = false;
	selecting = false;

	hovering = NULL;
	placing = NULL;

	popup = NULL;
	buildPopup = NULL;
	entityPopup = NULL;
	popupFocused = false;
	worldFocused = true;
}

void MainCamera::lookAt(Point p) {
}

void MainCamera::build(Spec* spec) {
	delete placing;
	placing = NULL;
	if (spec) {
		placing = new GuiFakeEntity(spec);
	}
}

Vector3 MainCamera::groundTarget(float ground) {
	RayHitInfo spot = GetCollisionRayGround((Ray){position, direction}, ground);
	return spot.position;
}

Camera3D MainCamera::raylibCamera() {
	return (Camera3D){
		position : position,
		target   : groundTarget(0),
		up       : up,
		fovy     : fovy,
		type     : CAMERA_PERSPECTIVE,
	};
}

void MainCamera::updateMouseState() {
	MouseState last = mouse;
	Vector2 pos = GetMousePosition();

	mouse = (MouseState) {
		x : (int)pos.x,
		y : (int)pos.y,
		dx : (int)pos.x - last.x,
		dy : (int)pos.y - last.y,
		wheel : GetMouseWheelMove(),
		rH : last.rH,
		rV : last.rV,
		zW : last.zW,
	};

	mouse.left = last.left;
	mouse.right = last.right;
	mouse.middle = last.middle;

	mouse.left.down = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
	mouse.right.down = IsMouseButtonDown(MOUSE_RIGHT_BUTTON);
	mouse.middle.down = IsMouseButtonDown(MOUSE_MIDDLE_BUTTON);

	mouse.left.changed = last.left.down != mouse.left.down;
	mouse.right.changed = last.right.down != mouse.right.down;
	mouse.middle.changed = last.middle.down != mouse.middle.down;

	for (MouseButton *button: {&mouse.left, &mouse.right, &mouse.middle}) {
		if (button->down && button->changed) {
			button->downAt = {mouse.x, mouse.y};
			button->drag = {0,0};
		}

		if (!button->down && button->changed) {
			button->upAt = {mouse.x, mouse.y};
			button->drag = {mouse.x-button->downAt.x, mouse.y-button->downAt.y};
		}

		if (button->down && !button->changed) {
			button->drag = {mouse.x-button->downAt.x, mouse.y-button->downAt.y};
		}

		if (!button->down && !button->changed) {
			button->drag = {0,0};
		}

		button->dragged = (std::abs(button->drag.x) > 5 || std::abs(button->drag.y) > 5);
		button->pressed = button->down && button->changed;
		button->released = !button->down && button->changed;
		button->clicked = button->released && !button->dragged;
	}

	popupFocused = popup ? popup->contains(mouse.x, mouse.y): false;
	worldFocused = !popupFocused;

	if (mouse.right.down) {
		mouse.rH += speedH * (float)mouse.dx;
		mouse.rV += speedV * (float)mouse.dy;
	}

	if (mouse.wheel) {
		mouse.zW += speedW * -(float)mouse.wheel;
	}

	if (mouse.left.dragged) {
		selection = {(float)mouse.left.downAt.x, (float)mouse.left.downAt.y, (float)mouse.left.drag.x, (float)mouse.left.drag.y};
		selecting = std::abs(selection.width) > 5 && std::abs(selection.height) > 5;
	}

	if (selecting && mouse.left.clicked) {
		selection = {0,0,0,0};
		selecting = false;
	}
}

void MainCamera::updateCamera() {

	if (IsKeyDown(KEY_W)) {
		Vector3 ahead = (Vector3){direction.x, 0, direction.z};
		position = Vector3Add(position, Vector3Normalize(ahead));
		nextPosition = position;
	}

	if (IsKeyDown(KEY_S)) {
		Vector3 ahead = (Vector3){direction.x, 0, direction.z};
		position = Vector3Subtract(position, Vector3Normalize(ahead));
		nextPosition = position;
	}

	if (IsKeyDown(KEY_D)) {
		Vector3 ahead = (Vector3){direction.x, 0, direction.z};
		Vector3 left = Vector3Transform(ahead, MatrixRotate(up, -90*DEG2RAD));
		position = Vector3Add(position, Vector3Normalize(left));
		nextPosition = position;
	}

	if (IsKeyDown(KEY_A)) {
		Vector3 ahead = (Vector3){direction.x, 0, direction.z};
		Vector3 left = Vector3Transform(ahead, MatrixRotate(up, 90*DEG2RAD));
		position = Vector3Add(position, Vector3Normalize(left));
		nextPosition = position;
	}

	if (!Vector3Equal(position, nextPosition, 0.0001)) {
		Vector3 delta = Vector3Subtract(nextPosition, position);
		position = Vector3Add(position, Vector3Scale(delta, 0.01));
	}

	if (!Vector3Equal(direction, nextDirection, 0.0001)) {
		Vector3 delta = Vector3Subtract(nextDirection, direction);
		direction = Vector3Normalize(Vector3Add(direction, Vector3Scale(delta, 0.1)));
	}

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

		Vector3 target = groundTarget(0);
		Vector3 radius = Vector3Subtract(position, target);
		Vector3 right = Vector3Normalize(Vector3CrossProduct(radius, up));
		Matrix rotateH = MatrixRotate(up, -rDH);
		Matrix rotateV = MatrixRotate(right, rDV);
		Matrix rotate = MatrixMultiply(rotateH, rotateV);
		radius = Vector3Transform(radius, rotate);

		position = Vector3Add(target, radius);
		position.y = std::max(5.0f, position.y);

		direction = Vector3Normalize(Vector3Subtract(target, position));

		nextPosition = position;
		nextDirection = direction;
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

		Vector3 target = groundTarget(0);
		Vector3 radius = Vector3Subtract(position, target);

		if (Vector3Length(radius) > 10.0f || zDW > 0.0f) {
			Vector3 delta = Vector3Scale(radius, zDW);

			position = Vector3Add(position, delta);
			position.y = std::max(5.0f, position.y);

			direction = Vector3Normalize(Vector3Subtract(target, position));

			nextPosition = position;
			nextDirection = direction;
		}
	}

	mouse.ray = GetMouseRay((Vector2){(float)mouse.x, (float)mouse.y}, raylibCamera());

	if (mouse.right.clicked) {
		RayHitInfo spot = GetCollisionRayGround(mouse.ray, 0);
		nextDirection = Vector3Normalize(Vector3Negate(Vector3Subtract(position, spot.position)));
	}

	mouse.ray = GetMouseRay((Vector2){(float)mouse.x, (float)mouse.y}, raylibCamera());
}

void MainCamera::update() {
	hovering = NULL;
	hovered.clear();
	selected.clear();

	while (entities.size() > 0) {
		delete entities.back();
		entities.pop_back();
	}

	updateMouseState();
	updateCamera();

	if (IsKeyReleased(KEY_ESCAPE)) {
		if (popup) {
			popup = NULL;
		}
		else
		if (placing) {
			delete placing;
			placing = NULL;
		}
		else
		if (selecting) {
			selection = {0,0,0,0};
			selecting = false;
		}
	}

	Vector3 target = groundTarget(0);
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
				float d = Vector3Distance(ge->pos, position);
				if (hovering == NULL || d < distance) {
					hovering = ge;
					distance = d;
				}
			}
		}

		if (selecting) {
			Camera3D rcam = raylibCamera();
			for (auto ge: entities) {
				Vector2 at = GetWorldToScreen(ge->pos, rcam);
				if (at.x > selection.x && at.x < selection.x+selection.width && at.y > selection.y && at.y < selection.y+selection.height) {
					selected.push_back(ge);
				}
			}
		}

		if (placing) {
			RayHitInfo hit = GetCollisionRayGround(mouse.ray, 0);
			placing->move(Point(hit.position))->floor(placing->pos.y);
		}
	}
}

void MainCamera::draw() {

	Camera3D camera = {
		position : position,
		target   : groundTarget(0),
		up       : up,
		fovy     : fovy,
		type     : CAMERA_PERSPECTIVE,
	};

	ClearBackground(SKYBLUE);

	BeginMode3D(camera);

		if (showGrid) {
			Vector3 groundZero = groundTarget(0);
			groundZero.x = std::floor(groundZero.x);
			groundZero.z = std::floor(groundZero.z);
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
			for (auto part: ge->spec->parts) {
				batches[part].push_back(part->instance(ge));
			}
		}

		for (auto pair: batches) {
			Part *part = pair.first;
			part->drawInstanced(pair.second.size(), pair.second.data());
		}

		if (hovering) {
			Spec::Animation* animation = &hovering->spec->animations[hovering->dir];
			Vector3 bounds = (Vector3){animation->w, animation->h, animation->d};
			DrawCubeWiresV(hovering->pos, Vector3AddValue(bounds, 0.01), SKYBLUE);
		}

		if (selecting) {
			for (auto ge: selected) {
				Spec::Animation* animation = &ge->spec->animations[ge->dir];
				Vector3 bounds = (Vector3){animation->w, animation->h, animation->d};
				DrawCubeWiresV(ge->pos, Vector3AddValue(bounds, 0.01), SKYBLUE);
			}
		}

		if (placing) {
			for (auto part: placing->spec->parts) {
				part->drawGhost(part->instance(placing));
			}
		}

		if (Path::jobs.size() > 0) {
			Path* job = Path::jobs.front();
			DrawCube(job->target, 0.5f, 0.5f, 0.5f, GOLD);

			for (auto pair: job->nodes) {
				Path::Node* node = pair.second;
				DrawCube(node->point, 0.5f, 0.5f, 0.5f,
					job->inOpenSet(node) ? GREEN: RED
				);
			}
		}

		for (auto ge: entities) {
			Entity& en = Entity::get(ge->id);
			if (en.spec->vehicle) {
				Vehicle& vehicle = en.vehicle();
				Vector3 p = en.pos;
				for (auto n: vehicle.path) {
					DrawSphere(n, 0.25f, RED);
					DrawLine3D(p, n, RED);
					p = n;
				}

				DrawRay((Ray){en.pos, en.looking()}, BLUE);
			}

			for (auto part: ge->spec->parts) {
				batches[part].push_back(part->instance(ge));
			}
		}

		DrawCube(Point(0,0,2.5), 0.5f, 0.5f, 5.0f, BLUE);
		DrawCube(Point(2.5,0,0), 5.0f, 0.5f, 0.5f, RED);
		DrawCube(Point(0,2.5,0), 0.5f, 5.0f, 0.5f, GREEN);

	EndMode3D();

	if (selecting) {
		DrawRectangleRec(selection, GetColor(0x0000ff99));
	}

	if (popup) {
		popup->draw();
	}
}