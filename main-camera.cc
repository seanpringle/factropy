#include "common.h"
#include "sim.h"
#include "view.h"
#include "mat4.h"

namespace {

	const float speedH = 0.0025f;
	const float speedV = 0.0025f;
	const float speedW = 0.25f;

	void drawGrid(Vector3 p, int slices, float spacing) {
		int halfSlices = slices/2;

		if (rlCheckBufferLimit(slices*4)) rlglDraw();

		rlBegin(RL_LINES);
			for (int i = -halfSlices; i <= halfSlices; i++) {
				//if (i == 0) {
				//	rlColor3f(0.5f, 0.5f, 0.5f);
				//	rlColor3f(0.5f, 0.5f, 0.5f);
				//	rlColor3f(0.5f, 0.5f, 0.5f);
				//	rlColor3f(0.5f, 0.5f, 0.5f);
				//}
				//else {
					rlColor3f(0.75f, 0.75f, 0.75f);
					rlColor3f(0.75f, 0.75f, 0.75f);
					rlColor3f(0.75f, 0.75f, 0.75f);
					rlColor3f(0.75f, 0.75f, 0.75f);
				//}

				rlVertex3f(p.x+(float)i*spacing, p.y, p.z+(float)-halfSlices*spacing);
				rlVertex3f(p.x+(float)i*spacing, p.y, p.z+(float)halfSlices*spacing);

				rlVertex3f(p.x+(float)-halfSlices*spacing, p.y, p.z+(float)i*spacing);
				rlVertex3f(p.x+(float)halfSlices*spacing, p.y, p.z+(float)i*spacing);
			}
		rlEnd();
	}
}

MainCamera::MainCamera(Point pos, Point dir) {
	position = pos;
	nextPosition = pos;
	direction = dir;
	nextDirection = dir;
	up = Point::Up;

	ZERO(mouse);
	showGrid = true;
	selecting = false;

	hovering = NULL;
	placing = NULL;
	directing = NULL;

	popup = NULL;
	buildPopup = NULL;
	entityPopup = NULL;
	recipePopup = NULL;
	popupFocused = false;
	worldFocused = true;

	info = NULL;
	entityInfo = NULL;
	ghostInfo = NULL;

	buildLevel = 0.0f;

	statsUpdate.clear();
	statsDraw.clear();
	statsFrame.clear();
}

MainCamera::~MainCamera() {
	while (entities.size() > 0) {
		delete entities.back();
		entities.pop_back();
	}
}

void MainCamera::lookAt(Point p) {
}

void MainCamera::build(Spec* spec) {
	delete placing;
	placing = NULL;
	if (spec) {
		placing = new Plan(Point::Zero);
		auto ge = new GuiFakeEntity(spec);
		ge->floor(buildLevel);
		placing->add(ge);
	}
}

Point MainCamera::groundTarget(float ground) {
	RayHitInfo spot = GetCollisionRayGround((Ray){position, direction}, ground);
	return Point(spot.position);
}

Camera3D MainCamera::raylibCamera() {
	return (Camera3D){
		position : position,
		target   : groundTarget(buildLevel),
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
		Point ahead = {direction.x, 0, direction.z};
		position += ahead.normalize();
		nextPosition = position;
	}

	if (IsKeyDown(KEY_S)) {
		Point ahead = {direction.x, 0, direction.z};
		position -= ahead.normalize();
		nextPosition = position;
	}

	if (IsKeyDown(KEY_D)) {
		Point ahead = {direction.x, 0, direction.z};
		Point left = ahead.transform(Mat4::rotate(up, -90*DEG2RAD));
		position += left.normalize();
		nextPosition = position;
	}

	if (IsKeyDown(KEY_A)) {
		Point ahead = {direction.x, 0, direction.z};
		Point left = ahead.transform(Mat4::rotate(up, 90*DEG2RAD));
		position += left.normalize();
		nextPosition = position;
	}

	if (position != nextPosition) {
		position += (nextPosition - position) * 0.025f;
	}

	if (direction != nextDirection) {
		direction += (nextDirection - direction) * 0.25f;
		direction = direction.normalize();
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

		Point target = groundTarget(buildLevel);
		Point radius = position - target;
		Point right = radius.cross(up).normalize();
		Mat4 rotateH = Mat4::rotate(up, -rDH);
		Mat4 rotateV = Mat4::rotate(right, rDV);
		Mat4 rotate = rotateH * rotateV;
		radius = radius.transform(rotate);

		position = target + radius;
		position.y = std::max(5.0f, position.y);

		direction = (target - position).normalize();

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

		Point target = groundTarget(buildLevel);
		Point radius = position - target;

		if (radius.length() > 10.0f || zDW > 0.0f) {
			position += radius * zDW;
			position.y = std::max(5.0f, position.y);

			direction = (target - position).normalize();

			nextPosition = position;
			nextDirection = direction;
		}
	}

	mouse.ray = GetMouseRay((Vector2){(float)mouse.x, (float)mouse.y}, raylibCamera());

	if (mouse.right.clicked) {
		RayHitInfo spot = GetCollisionRayGround(mouse.ray, buildLevel);
		nextDirection = -(position - Point(spot.position)).normalize();
	}

	mouse.ray = GetMouseRay((Vector2){(float)mouse.x, (float)mouse.y}, raylibCamera());
}

void MainCamera::update() {
	statsUpdate.track(frame, [&]() {
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

		if (worldFocused) {
			if (placing) {
				RayHitInfo hit = GetCollisionRayGround(mouse.ray, buildLevel);
				placing->move(Point(hit.position));
				placing->floor(buildLevel);
			}
		}

		Point target = groundTarget(buildLevel);
		Box view = (Box){target.x, target.y, target.z, 500, 500, 500};

		Sim::locked([&]() {
			for (int id: Entity::intersecting(view)) {
				entities.push_back(new GuiEntity(id));
			}

			placingFits = !placing || placing->fits();

			if (directing && !Entity::exists(directing->id)) {
				delete directing;
				directing = NULL;
			}

			if (directing) {
				uint id = directing->id;
				delete directing;
				directing = new GuiEntity(id);
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
					float d = ge->pos.distance(position);
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
				placing->floor(buildLevel);
			}
		}
	});
}

void MainCamera::draw() {
	statsDraw.track(frame, [&]() {

		Camera3D camera = {
			position : position,
			target   : groundTarget(buildLevel),
			up       : up,
			fovy     : fovy,
			type     : CAMERA_PERSPECTIVE,
		};

		ClearBackground(SKYBLUE);

		BeginMode3D(camera);
			Point groundZero = groundTarget(buildLevel);

			if (showGrid) {
				groundZero.x = std::floor(groundZero.x);
				groundZero.z = std::floor(groundZero.z);
				drawGrid(groundZero, 64, 1);
			}

			std::vector<Mat4> water;
			std::vector<Mesh> chunk_meshes;
			std::vector<Mat4> chunk_transforms;

			std::map<uint,std::vector<Mat4>> resource_transforms;

			float size = (float)Chunk::size;
			Mat4 scale = Mat4::scale(size,size,size);
			for (auto pair: Chunk::all) {
				Chunk *chunk = pair.second;
				float x = chunk->x;
				float y = chunk->y;
				if (chunk->regenerate) {
					chunk->genHeightMap();
				}
				chunk_meshes.push_back(chunk->heightmap);
				chunk_transforms.push_back(chunk->transform);
				water.push_back(Mat4::translate(x+0.5f, -0.52f, y+0.5f) * scale);

				int cx = chunk->x*Chunk::size;
				int cz = chunk->y*Chunk::size;
				int halt = Chunk::size/2;

				if (groundZero.distance(Point(cx+halt, 0.0f, cz+halt)) < 1000) {
					for (auto [tx,ty]: chunk->minerals) {
						Chunk::Tile* tile = &chunk->tiles[ty][tx];
						float h = tile->elevation*(49.82*2)-0.25;
						Mat4 r = Mat4::rotate(Point(cx+tx, h, cz+ty), tx);
						Mat4 m = Mat4::translate(cx+tx, h, cz+ty);
						resource_transforms[tile->mineral].push_back(r * m);
					}
				}
			}

			rlDrawMaterialMeshes(Chunk::material, chunk_meshes.size(), chunk_meshes.data(), chunk_transforms.data());
			rlDrawMeshInstanced(waterCube.meshes[0], waterCube.materials[0], water.size(), water.data());

			for (auto [iid,transforms]: resource_transforms) {
				Item* item = Item::get(iid);
				for (uint i = 0; i < item->parts.size(); i++) {
					item->parts[i]->drawInstanced(true, transforms.size(), transforms.data());
				}
			}

			std::map<Part*,std::vector<Mat4>> extant_ld;
			std::map<Part*,std::vector<Mat4>> extant_hd;
			std::map<Part*,std::vector<Mat4>> ghosts_ld;
			std::map<Part*,std::vector<Mat4>> ghosts_hd;

			std::vector<Mat4> belt_pillars;
			std::set<BeltSegment*> belt_segments;
			std::map<Part*,std::vector<Mat4>> items_hd;
			std::map<Part*,std::vector<Mat4>> items_ld;

			for (auto ge: entities) {
				bool hd = ge->pos.distance(position) < 100;

				for (uint i = 0; i < ge->spec->parts.size(); i++) {
					Part *part = ge->spec->parts[i];
					Mat4 instance = part->specInstance(ge->spec, i, ge->state, ge->transform);
					(ge->ghost ? (hd ? ghosts_hd: ghosts_ld): (hd ? extant_hd: extant_ld))[part].push_back(instance);
				}

				Entity& en = Entity::get(ge->id);

				if (!ge->ghost && ge->spec->vehicle) {
					Vehicle& vehicle = en.vehicle();
					Point p = en.pos;
					for (auto n: vehicle.path) {
						DrawSphere(n, 0.25f, RED);
						DrawLine3D(p, n, RED);
						p = n;
					}
				}

				if (!ge->ghost && ge->spec->belt) {
					Belt& belt = en.belt();

					if (belt.segment) {
						belt_segments.insert(belt.segment);
					}

					if (en.ground().y < 0.01f && (belt.offset == 0 || belt.offset == belt.segment->belts.size()-1)) {
						belt_pillars.push_back(View::beltPillar1->instance(Mat4::translate(en.pos.x, en.pos.y, en.pos.z)));
					}
				}

				if (!ge->ghost && ge->spec->lift) {
					Lift& lift = en.lift();

					if (lift.iid && hd) {
						Entity& en = Entity::get(lift.id);
						Point p = en.pos;
						Mat4 m = Mat4::translate(p.x, p.y+1.0f, p.z);
						Item* item = Item::get(lift.iid);
						for (uint i = 0; i < item->parts.size(); i++) {
							Part* part = item->parts[i];
							(p.distance(position) < 100 ? items_hd: items_ld)[part].push_back(part->instance(en.spec->states[en.state][4] * m));
						}
					}
				}

				if (!ge->ghost && ge->spec->arm) {
					Arm& arm = en.arm();

					if (arm.iid && hd) {
						Item* item = Item::get(arm.iid);
						uint grip = en.spec->states[0].size()-1;
						for (uint i = 0; i < item->parts.size(); i++) {
							Part* part = item->parts[i];
							Mat4 o = en.spec->parts[grip]->specInstance(en.spec, grip, en.state, en.dir.rotation());
							Point p = Point::Zero.transform(o) + ge->pos + Point::Up;
							Mat4 t = Mat4::translate(p.x, p.y + item->armV, p.z);
							(p.distance(position) < 100 ? items_hd: items_ld)[part].push_back(part->instance(t));
						}
					}
				}

				if (!ge->ghost && ge->spec->drone) {
					Drone& drone = en.drone();

					if (drone.iid && hd) {
						Item* item = Item::get(drone.iid);
						Point p = ge->pos - (Point::Up*0.75f);
						Mat4 t = Mat4::translate(p.x, p.y + item->armV, p.z);
						for (uint i = 0; i < item->parts.size(); i++) {
							Part* part = item->parts[i];
							(p.distance(position) < 100 ? items_hd: items_ld)[part].push_back(part->instance(t));
						}
					}
				}
			}

			for (BeltSegment* segment: belt_segments) {
				Point spot = segment->front();
				Point step = segment->step();

				for (auto beltItem: segment->items) {
					Item* item = Item::get(beltItem.iid);

					spot += step * (float)beltItem.offset;
					Point p = spot + (step * ((float)BeltSegment::slot/2.0f));

					Mat4 m = Mat4::translate(p.x, p.y + item->beltV, p.z);
					for (uint i = 0; i < item->parts.size(); i++) {
						Part* part = item->parts[i];
						(p.distance(position) < 100 ? items_hd: items_ld)[part].push_back(part->instance(m));
					}

					spot += step * (float)BeltSegment::slot;
				}
			}

			for (auto [part,batch]: extant_ld) {
				part->drawInstanced(false, batch.size(), batch.data());
			}

			for (auto [part,batch]: extant_hd) {
				part->drawInstanced(true, batch.size(), batch.data());
			}

			for (auto [part,batch]: ghosts_ld) {
				part->drawGhostInstanced(false, batch.size(), batch.data());
			}

			for (auto [part,batch]: ghosts_hd) {
				part->drawGhostInstanced(true, batch.size(), batch.data());
			}

			if (belt_pillars.size() > 0) {
				View::beltPillar1->drawInstanced(false, belt_pillars.size(), belt_pillars.data());
			}

			for (auto [part,batch]: items_hd) {
				part->drawInstanced(true, batch.size(), batch.data());
			}

			for (auto [part,batch]: items_ld) {
				part->drawInstanced(false, batch.size(), batch.data());
			}

			if (hovering) {
				Box box = hovering->box();
				Point bounds = {box.w, box.h, box.d};
				DrawCubeWiresV(hovering->pos, bounds + 0.01f, SKYBLUE);
			}

			if (selecting) {
				for (auto ge: selected) {
					Box box = ge->box();
					Point bounds = {box.w, box.h, box.d};
					DrawCubeWiresV(ge->pos, bounds + 0.01f, SKYBLUE);
				}
			}

			if (placing) {
				for (auto te: placing->entities) {
					for (uint i = 0; i < te->spec->parts.size(); i++) {
						Part *part = te->spec->parts[i];
						Mat4 instance = part->specInstance(te->spec, i, te->state, te->transform);
						part->drawGhostInstanced(true, 1, &instance);
					}

					Box box = te->box();
					Point bounds = {box.w, box.h, box.d};
					DrawCubeWiresV(te->pos, bounds + 0.01f, placingFits ? GREEN: RED);
				}
			}

			if (directing) {
				Box box = directing->box();
				Point bounds = {box.w, box.h, box.d};
				DrawCubeWiresV(directing->pos, bounds + 0.01f, ORANGE);
			}

			if (Path::jobs.size() > 0) {
				Path* job = Path::jobs.front();
				DrawCube(job->target, 0.5f, 0.5f, 0.5f, GOLD);

				std::vector<Mat4> reds;
				std::vector<Mat4> greens;

				for (auto pair: job->nodes) {
					Path::Node* node = pair.second;
					if (job->inOpenSet(node)) {
						greens.push_back(Mat4::translate(node->point.x, node->point.y, node->point.z));
					} else {
						reds.push_back(Mat4::translate(node->point.x, node->point.y, node->point.z));
					}
				}

				if (reds.size() > 0)
					rlDrawMeshInstanced(redCube.meshes[0], redCube.materials[0], reds.size(), reds.data());

				if (greens.size() > 0)
					rlDrawMeshInstanced(greenCube.meshes[0], greenCube.materials[0], greens.size(), greens.data());
			}

		EndMode3D();

		if (selecting) {
			DrawRectangleRec(selection, GetColor(0x0000ff99));
		}

		if (popup) {
			popup->draw();
		}

		if (info) {
			info->draw();
		}
	});
}