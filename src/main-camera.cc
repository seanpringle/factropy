
#include "raylib-ex.h"
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
	direction = dir;
	up = Point::Up;

	ZERO(mouse);
	showGrid = true;
	selecting = false;

	hovering = nullptr;
	placing = nullptr;
	directing = nullptr;
	connecting = nullptr;

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

void MainCamera::build(Spec* spec, Point dir) {
	delete placing;
	placing = nullptr;
	if (spec) {
		placing = new Plan(Point::Zero);
		auto ge = new GuiFakeEntity(spec);
		ge->dir = dir;
		ge->floor(0.0f); // build level applied by plan
		placing->add(ge);
	}
}

Point MainCamera::groundTarget(float ground) {
	RayHitInfo spot = GetCollisionRayGround((Ray){position, direction}, ground);
	return Point(spot.position);
}

Camera3D MainCamera::raylibCamera() {
	return (Camera3D){
		.position = position,
		.target   = groundTarget(buildLevel),
		.up       = up,
		.fovy     = fovy,
		.type     = CAMERA_PERSPECTIVE,
	};
}

void MainCamera::updateMouseState() {
	MouseState last = mouse;
	Vector2 pos = GetMousePosition();

	mouse = (MouseState) {
		.x = (int)pos.x,
		.y = (int)pos.y,
		.dx = (int)pos.x - last.x,
		.dy = (int)pos.y - last.y,
		.wheel = (int)GetMouseWheelMove(),
		.rH = last.rH,
		.rV = last.rV,
		.zW = last.zW,
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

	if (mouse.right.down) {
		mouse.rH += speedH * (float)mouse.dx;
		mouse.rV += speedV * (float)mouse.dy;
	}

	if (mouse.wheel) {
		mouse.zW += speedW * -(float)mouse.wheel;
	}
}

void MainCamera::updateCamera() {

	float pitchAngleMax = 75.0f*DEG2RAD;

	if (IsKeyDown(KEY_W)) {
		Point ahead = {direction.x, 0, direction.z};
		position += ahead.normalize();
	}

	if (IsKeyDown(KEY_S)) {
		Point ahead = {direction.x, 0, direction.z};
		position -= ahead.normalize();
	}

	if (IsKeyDown(KEY_D)) {
		Point ahead = {direction.x, 0, direction.z};
		Point left = ahead.transform(Mat4::rotate(up, -90*DEG2RAD));
		position += left.normalize();
	}

	if (IsKeyDown(KEY_A)) {
		Point ahead = {direction.x, 0, direction.z};
		Point left = ahead.transform(Mat4::rotate(up, 90*DEG2RAD));
		position += left.normalize();
	}

	if (IsKeyReleased(KEY_SPACE)) {
		Point target = groundTarget(buildLevel);
		float radius = (position - target).length();

		if (position.z < target.z) {
			Mat4 rotateV = Mat4::rotate(Point::East, pitchAngleMax);
			position = Point(target.x, buildLevel, -radius).transform(rotateV);
			direction = (target - position).normalize();
		} else {
			Mat4 rotateV = Mat4::rotate(Point::West, pitchAngleMax);
			position = Point(target.x, buildLevel, radius).transform(rotateV);
			direction = (target - position).normalize();
		}
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
		Point view = position - target;
		Point right = view.cross(up).normalize();

		// as camera always orients to Up, prevent rotation past the zenith and causing a spin
		Point groundRadius = Point(view.x, buildLevel, view.z);
		float pitchAngle = std::acos(view.dot(groundRadius)/(view.length() * groundRadius.length()));
		rDV = (pitchAngle + rDV > pitchAngleMax) ? pitchAngleMax - pitchAngle: rDV;

		Mat4 rotateH = Mat4::rotate(up, -rDH);
		Mat4 rotateV = Mat4::rotate(right, rDV);
		Mat4 rotate = rotateH * rotateV;
		view = view.transform(rotate);

		position = target + view;
		position.y = std::max(5.0f, position.y);

		direction = (target - position).normalize();
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
		Point view = position - target;

		if (view.length() > 10.0f || zDW > 0.0f) {
			position += view * zDW;
			position.y = std::max(5.0f, position.y);

			direction = (target - position).normalize();
		}
	}

	mouse.ray = GetMouseRay((Vector2){(float)mouse.x, (float)mouse.y}, raylibCamera());

	if (mouse.left.dragged) {
		if (!selecting) {
			Ray rayA = GetMouseRay((Vector2){(float)mouse.left.downAt.x,(float)mouse.left.downAt.y}, raylibCamera());
			RayHitInfo hitA = GetCollisionRayGround(rayA, buildLevel);
			selection.a = hitA.position;
		}
		Ray rayB = mouse.ray;
		RayHitInfo hitB = GetCollisionRayGround(rayB, buildLevel);
		selection.b = {hitB.position};
		selecting = !placing;
	}

	if (selecting && mouse.left.clicked) {
		selection = {Point::Zero, Point::Zero};
		selecting = false;
	}
}

void MainCamera::update(bool worldFocused) {
	statsUpdate.track(frame, [&]() {
		hovering = nullptr;
		hovered.clear();
		selected.clear();
		placingFits = false;

		while (entities.size() > 0) {
			delete entities.back();
			entities.pop_back();
		}

		if (worldFocused) {
			updateMouseState();
			updateCamera();
		}

		if (IsKeyReleased(KEY_ESCAPE)) {
			if (placing) {
				delete placing;
				placing = nullptr;
			}
			else
			if (selecting) {
				selection = {Point::Zero, Point::Zero};
				selecting = false;
			}
		}

		if (placing) {
			RayHitInfo hit = GetCollisionRayGround(mouse.ray, buildLevel);
			placing->move(Point(hit.position));
			placing->floor(buildLevel);
		}

		Point target = groundTarget(buildLevel);
		Box view = (Box){target.x, target.y, target.z, 500, 500, 500};

		Sim::locked([&]() {
			for (int id: Entity::intersecting(view)) {
				entities.push_back(new GuiEntity(id));
			}

			if (directing && !Entity::exists(directing->id)) {
				delete directing;
				directing = nullptr;
			}

			if (directing) {
				uint id = directing->id;
				delete directing;
				directing = new GuiEntity(id);
			}

			if (connecting && !Entity::exists(connecting->id)) {
				delete connecting;
				connecting = nullptr;
			}

			if (connecting) {
				uint id = connecting->id;
				delete connecting;
				connecting = new GuiEntity(id);
			}

			placingFits = !placing || placing->fits();
		});

		for (auto ge: entities) {
			if (!ge->spec->select) continue;
			if (CheckCollisionRayBox(mouse.ray, ge->box().bounds())) {
				hovered.push_back(ge);
			}
		}

		if (hovered.size() > 0) {
			float distance = 0.0f;
			for (auto ge: hovered) {
				float d = ge->pos.distance(position);
				if (hovering == nullptr || d < distance) {
					hovering = ge;
					distance = d;
				}
			}
		}

		if (selecting) {
			Box box = Box(selection.a, selection.b + (Point::Up*1000.0f));
			for (auto ge: entities) {
				if (!ge->spec->select) continue;
				if (ge->box().intersects(box)) {
					selected.push_back(ge);
				}
			}
		}
	});
}

void MainCamera::draw() {
	statsDraw.track(frame, [&]() {

		auto drawBox = [&](Box box, Point dir, Color color) {
			Point bounds = {box.w, box.h, box.d};
			rlPushMatrix();
				rlLoadIdentity();
				rlMultMatrixf(MatrixToFloat(dir.rotation()));
				rlMultMatrixf(MatrixToFloat(box.centroid().translation()));
				DrawCubeWiresV(Point::Zero, bounds + 0.01f, color);
			rlPopMatrix();
		};

		Camera3D camera = {
			.position = position,
			.target   = groundTarget(buildLevel),
			.up       = up,
			.fovy     = fovy,
			.type     = CAMERA_PERSPECTIVE,
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
						resource_transforms[tile->mineral.iid].push_back(r * m);
					}
				}
			}

			rlDrawMaterialMeshes(Chunk::material, chunk_meshes.size(), chunk_meshes.data(), chunk_transforms.data());
			rlDrawMeshInstanced2(waterCube.meshes[0], waterCube.materials[0], water.size(), water.data());

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
			std::map<Part*,std::vector<Mat4>> items_hd;
			std::map<Part*,std::vector<Mat4>> items_ld;

			for (auto ge: entities) {
				bool hd = ge->pos.distance(position) < 100;

				for (uint i = 0; i < ge->spec->parts.size(); i++) {
					Part *part = ge->spec->parts[i];
					Mat4 instance = part->specInstance(ge->spec, i, ge->state, ge->partTransform(part));
					(ge->ghost ? (hd ? ghosts_hd: ghosts_ld): (hd ? extant_hd: extant_ld))[part].push_back(instance);
				}

				Entity& en = Entity::get(ge->id);

				if (!ge->ghost && ge->spec->vehicle && hovering && ge->id == hovering->id) {
					Vehicle& vehicle = en.vehicle();
					Point p = en.pos;
					for (auto n: vehicle.path) {
						DrawSphere(n, 0.25f, RED);
						DrawLine3D(p, n, RED);
						p = n;
					}
					p = en.pos;
					for (auto n: vehicle.waypoints) {
						DrawSphere(n->position, 0.25f, BLUE);
						DrawLine3D(p, n->position, BLUE);
						p = n->position;
					}
				}

				if (!ge->ghost && ge->spec->conveyor && ge->conveyor.iid) {
					Item* item = Item::get(ge->conveyor.iid);
					Point p = ge->pos;
					Mat4 move = Mat4::translate(ge->pos - ((Point::Up*0.5f) + item->beltV));
					Mat4 bump = ge->spec->conveyorTransforms[ge->conveyor.offset];
					Mat4 m = bump * ge->dir.rotation() * move;
					for (uint i = 0; i < item->parts.size(); i++) {
						Part* part = item->parts[i];
						(p.distance(position) < 100 ? items_hd: items_ld)[part].push_back(part->instance(m));
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

				if (!ge->ghost && ge->spec->projector) {
					Projector& projector = en.projector();

					if (projector.iid && hd) {
						Item* item = Item::get(projector.iid);
						Point p = ge->pos + (Point::Up*0.35f);
						float rot = (float)(frame%360) * DEG2RAD;
						float bob = std::sin(rot*4.0f)*0.1f;
						Mat4 t1 = Mat4::scale(0.75f) * Mat4::rotate(Point::Up, rot) * Mat4::translate(p.x, p.y+bob, p.z);
						for (uint i = 0; i < item->parts.size(); i++) {
							Part* part = item->parts[i];
							(p.distance(position) < 100 ? items_hd: items_ld)[part].push_back(part->instance(t1));
						}
					}
					if (projector.fid && hd) {
						Fluid* fluid = Fluid::get(projector.fid);
						Point p = ge->pos + (Point::Up*0.35f);
						float rot = (float)(frame%360) * DEG2RAD;
						float bob = std::sin(rot*4.0f)*0.1f;
						Mat4 t1 = Mat4::scale(0.75f) * Mat4::rotate(Point::Up, rot) * Mat4::translate(p.x, p.y+bob, p.z);
						Part* part = fluid->droplet;
						(p.distance(position) < 100 ? items_hd: items_ld)[part].push_back(part->instance(t1));
					}
				}

				if (!ge->ghost && ge->spec->ropeway) {
					Ropeway& ropeway = en.ropeway();

					if (ropeway.next) {
						Entity& sibling = Entity::get(ropeway.next);

						Point a = ropeway.arrive();
						Point b = sibling.ropeway().arrive();
						DrawLine3D(a, b, BLACK);
						Point c = ropeway.depart();
						Point d = sibling.ropeway().depart();
						DrawLine3D(c, d, BLACK);
					}
				}
			}

			for (auto [part,batch]: extant_ld) {
				part->drawInstanced(false, batch.size(), batch.data());
			}

			for (auto [part,batch]: extant_hd) {
				part->drawInstanced(true, batch.size(), batch.data());
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

			for (auto [part,batch]: ghosts_ld) {
				part->drawGhostInstanced(false, batch.size(), batch.data());
			}

			for (auto [part,batch]: ghosts_hd) {
				part->drawGhostInstanced(true, batch.size(), batch.data());
			}

			for (auto ge: entities) {
				if (ge->spec->explosion) {
					DrawSphere(ge->pos, ge->explosion.radius, RED);
				}
			}

			if (hovering) {
				drawBox(hovering->southBox(), hovering->dir, SKYBLUE);

				if (hovering->spec->pipe) {
					Sim::locked([&]() {
						Entity& en = Entity::get(hovering->id);
						for (Point p: en.pipe().pipeConnections()) {
							DrawCube(p, 0.25f, 0.25f, 0.25f, RED);
						}

						if (en.pipe().network) {
							for (auto id: en.pipe().network->pipes) {
								Entity& sib = Entity::get(id);
								drawBox(sib.spec->southBox(sib.pos), sib.dir, GREEN);
							}
						}
					});
				}

				if (hovering->spec->crafter) {
					Entity& en = Entity::get(hovering->id);
					for (Point p: en.crafter().pipeInputConnections()) {
						DrawCube(p, 0.25f, 0.25f, 0.25f, GREEN);
					}
					for (Point p: en.crafter().pipeOutputConnections()) {
						DrawCube(p, 0.25f, 0.25f, 0.25f, RED);
					}
				}

//				if (hovering->spec->arm) {
//					Entity& en = Entity::get(hovering->id);
//					DrawCube(en.arm().input(), 0.25f, 0.25f, 0.25f, GREEN);
//					DrawCube(en.arm().output(), 0.25f, 0.25f, 0.25f, RED);
//				}

				if (hovering->spec->turret) {
					DrawSphereWires(hovering->pos, hovering->spec->turretRange, 72, 72, RED);
				}

//				if (hovering->spec->conveyor) {
//					Sim::locked([&]() {
//						Entity& en = Entity::get(hovering->id);
//						DrawCube(en.conveyor().input(), 0.25f, 0.25f, 0.25f, GREEN);
//						DrawCube(en.conveyor().output(), 0.25f, 0.25f, 0.25f, RED);
//
//						if (en.conveyor().prev) {
//							Entity& sib = Entity::get(en.conveyor().prev);
//							drawBox(sib.spec->southBox(sib.pos), sib.dir, GREEN);
//						}
//
//						if (en.conveyor().next) {
//							Entity& sib = Entity::get(en.conveyor().next);
//							drawBox(sib.spec->southBox(sib.pos), sib.dir, RED);
//						}
//
//						if (en.conveyor().side) {
//							Entity& sib = Entity::get(en.conveyor().side);
//							drawBox(sib.spec->southBox(sib.pos), sib.dir, ORANGE);
//						}
//					});
//				}

				if (hovering->spec->unveyor) {
					Sim::locked([&]() {
						Entity& en = Entity::get(hovering->id);
						drawBox(en.unveyor().range(), Point::South, YELLOW);

						if (en.unveyor().partner) {
							Entity& ep = Entity::get(en.unveyor().partner);
							drawBox(en.spec->southBox(en.pos), en.dir, YELLOW);
							drawBox(ep.spec->southBox(ep.pos), ep.dir, YELLOW);
						}
					});
				}

				if (hovering->spec->pipeUnderground) {
					Sim::locked([&]() {
						Entity& en = Entity::get(hovering->id);
						drawBox(en.pipe().undergroundRange(), Point::South, YELLOW);

						if (en.pipe().partner) {
							Entity& ep = Entity::get(en.pipe().partner);
							drawBox(en.spec->southBox(en.pos), en.dir, YELLOW);
							drawBox(ep.spec->southBox(ep.pos), ep.dir, YELLOW);
						}
					});
				}

//				if (hovering->spec->ropeway) {
//					Sim::locked([&]() {
//						Entity& en = Entity::get(hovering->id);
//						DrawCube(en.ropeway().arrive(), 0.25f, 0.25f, 0.25f, GREEN);
//						DrawCube(en.ropeway().depart(), 0.25f, 0.25f, 0.25f, RED);
//					});
//				}
			}

			if (selecting) {
				for (auto ge: selected) {
					drawBox(ge->southBox(), ge->dir, SKYBLUE);
				}
			}

			if (placing) {
				for (auto te: placing->entities) {
					for (uint i = 0; i < te->spec->parts.size(); i++) {
						Part *part = te->spec->parts[i];
						Mat4 instance = part->specInstance(te->spec, i, te->state, te->transform) * (Point::Up*0.01f).translation();
						part->drawGhostInstanced(true, 1, &instance);
					}

					drawBox(te->southBox(), te->dir, placingFits ? GREEN: RED);

					if (te->spec->pipeConnections.size()) {
						for (Point p: te->spec->relativePoints(te->spec->pipeConnections, te->dir.rotation(), te->pos)) {
							DrawCube(p, 0.25f, 0.25f, 0.25f, BLUE);
						}
					}

					if (te->spec->pipeInputConnections.size()) {
						for (Point p: te->spec->relativePoints(te->spec->pipeInputConnections, te->dir.rotation(), te->pos)) {
							DrawCube(p, 0.25f, 0.25f, 0.25f, GREEN);
						}
					}

					if (te->spec->pipeOutputConnections.size()) {
						for (Point p: te->spec->relativePoints(te->spec->pipeOutputConnections, te->dir.rotation(), te->pos)) {
							DrawCube(p, 0.25f, 0.25f, 0.25f, RED);
						}
					}

					if (te->spec->supportPoints.size()) {
						for (Point p: te->spec->relativePoints(te->spec->supportPoints, te->dir.rotation(), te->pos)) {
							DrawCube(p, 0.25f, 0.25f, 0.25f, GOLD);
						}
					}
				}
			}

			if (directing) {
				drawBox(directing->southBox(), directing->dir, ORANGE);
			}

			if (connecting) {
				drawBox(connecting->southBox(), connecting->dir, BLUE);

				if (placing) {
					for (auto& ge: placing->entities) {
						if (connecting->connectable(ge)) {
							DrawLine3D(connecting->pos, ge->pos, GREEN);
						}
					}
				}

				if (hovering && connecting->connectable(hovering)) {
					DrawLine3D(connecting->pos, hovering->pos, GREEN);
				}
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
					rlDrawMeshInstanced2(redCube.meshes[0], redCube.materials[0], reds.size(), reds.data());

				if (greens.size() > 0)
					rlDrawMeshInstanced2(greenCube.meshes[0], greenCube.materials[0], greens.size(), greens.data());
			}

			if (selecting) {
				Box box = Box(selection.a, selection.b);
				DrawCube(box.centroid(), box.w, box.h, box.d, GetColor(0x0000ff99));
				drawBox(box, Point::South, ORANGE);
			}

		EndMode3D();
	});
}