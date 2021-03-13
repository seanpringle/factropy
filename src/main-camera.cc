
#include "raylib-ex.h"
#include "common.h"
#include "sim.h"
#include "view.h"
#include "mat4.h"

namespace {

	const float speedH = 0.0025f;
	const float speedV = 0.0025f;
	const float speedW = 0.25f;
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

	frame = 0;

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

		if (placing) {
			if (placing->entities.size() == 1 && placing->entities[0]->spec->placeOnHill) {
				RayHitInfo hit = GetCollisionRayGround(mouse.ray, buildLevel);
				auto pe = placing->entities[0];
				Box box = pe->spec->box(hit.position, pe->dir, pe->spec->collision);
				placing->move(Point(hit.position.x, Chunk::hillPlatform(box), hit.position.z));
			} else {
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
			if (CheckCollisionRayBox(mouse.ray, ge->selectionBox().bounds())) {
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
				if (ge->selectionBox().intersects(box)) {
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

		objects = 0;
		float hdDistanceSquared = 100.0f*100.0f;
		Point groundZero = groundTarget(buildLevel);
		Box view = groundZero.box().grow(Chunk::size*5);

		BeginMode3D(camera);

			minivec<Mat4> water;
			minivec<Mesh> chunk_meshes;
			minivec<Mat4> chunk_transforms;

			std::unordered_map<uint,minivec<Mat4>> resource_transforms;

			float size = (float)Chunk::size;
			Mat4 scale = Mat4::scale(size,size,size);

			for (auto [cx,cy]: gridwalk(Chunk::size, view)) {
				Chunk* chunk = Chunk::request(cx, cy);
				if (!chunk || !chunk->ready()) continue;

				chunk->meshMutex.lock();
				chunk->meshTickLastViewed = Sim::tick;

				if (chunk->meshLoaded) {
					float x = chunk->x;
					float y = chunk->y;

					chunk_meshes.push_back(chunk->heightmap);
					chunk_transforms.push_back(chunk->transform);
					water.push_back(Mat4::translate(x+0.5f, -0.52f, y+0.5f) * scale);

					int ctx = chunk->x*Chunk::size;
					int ctz = chunk->y*Chunk::size;

					for (auto [tx,ty]: chunk->meshMinerals) {
						Chunk::Tile* tile = &chunk->tiles[ty][tx];
						if (!tile->hill) continue;
						if (!tile->hill->minerals.count(tile->mineral.iid)) continue;
						if (!tile->hill->minerals[tile->mineral.iid]) continue;

						float h = tile->elevation*100.0f-0.25f;
						Mat4 r = Mat4::rotate(Point(ctx+tx, h, ctz+ty), tx);
						Mat4 m = Mat4::translate(ctx+tx, h, ctz+ty);
						resource_transforms[tile->mineral.iid].push_back(r * m);
					}
				}

				chunk->meshMutex.unlock();
			}

			rlDrawMaterialMeshes(Chunk::material, chunk_meshes.size(), chunk_meshes.data(), chunk_transforms.data());
			rlDrawMeshInstanced2(waterCube.meshes[0], waterCube.materials[0], water.size(), water.data());

			objects += chunk_transforms.size();
			objects += water.size();

			for (auto& [iid,transforms]: resource_transforms) {
				Item* item = Item::get(iid);
				for (uint i = 0; i < item->parts.size(); i++) {
					item->parts[i]->drawInstanced(true, transforms.size(), transforms.data());
					objects += transforms.size();
				}
			}

			std::unordered_map<Part*,minivec<Mat4>> extant_ld;
			std::unordered_map<Part*,minivec<Mat4>> extant_hd;
			std::unordered_map<Part*,minivec<Mat4>> ghosts_ld;
			std::unordered_map<Part*,minivec<Mat4>> ghosts_hd;
			std::unordered_map<Part*,minivec<Mat4>> items_hd;
			std::unordered_map<Part*,minivec<Mat4>> items_ld;

			minivec<Mat4> gridSquares;

			if (showGrid && groundZero.distanceSquared(position) < hdDistanceSquared) {
				Point zero = {std::floor(groundZero.x),0,std::floor(groundZero.z)};

				for (int x = -33; x <= 32; x++) {
					for (int z = -33; z <= 32; z++) {
						Point corner = zero + Point(x,0,z);
						Point center = corner + Point(0.5,0,0.5);

						auto tile = Chunk::tileTryGet(corner.x, corner.z);
						if (!tile) continue;
						if (!tile->isLand()) continue;

						gridSquares.push_back(gridSquareLand->instance(center.translation()));
					}
				}
			}

			//std::sort(entities.begin(), entities.end(), [&](const GuiEntity* a, const GuiEntity* b) {
			//	return a->pos.distanceSquared(position) > b->pos.distanceSquared(position);
			//});

			for (auto& ge: entities) {
				bool hd = ge->pos.distanceSquared(position) < hdDistanceSquared;

				for (uint i = 0; i < ge->spec->parts.size(); i++) {
					Part *part = ge->spec->parts[i];
					Mat4 instance = part->specInstance(ge->spec, i, ge->state, ge->partTransform(part));
					(ge->ghost ? (hd ? ghosts_hd: ghosts_ld): (hd ? extant_hd: extant_ld))[part].push_back(instance);
				}

				if (!ge->ghost && ge->spec->vehicle && hovering && ge->id == hovering->id) {
					Sim::locked([&]() {
						Entity& en = Entity::get(ge->id);
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
						for (auto p: vehicle.sensors()) {
							DrawSphere(p, 0.25f, BLUE);
						}
					});
				}

				if (!ge->ghost && ge->spec->conveyor && ge->conveyor.iid) {
					Item* item = Item::get(ge->conveyor.iid);
					Point p = ge->pos;
					Mat4 move = Mat4::translate(ge->pos - ((Point::Up*0.5f) + item->beltV));
					Mat4 bump = ge->spec->conveyorTransforms[ge->conveyor.offset];
					Mat4 m = bump * ge->dir.rotation() * move;
					for (uint i = 0; i < item->parts.size(); i++) {
						Part* part = item->parts[i];
						(p.distanceSquared(position) < hdDistanceSquared ? items_hd: items_ld)[part].push_back(part->instance(m));
					}
				}

				if (!ge->ghost && ge->spec->arm) {
					if (ge->arm.iid && hd) {
						Item* item = Item::get(ge->arm.iid);
						uint grip = ge->spec->states[0].size()-1;
						for (uint i = 0; i < item->parts.size(); i++) {
							Part* part = item->parts[i];
							Mat4 o = ge->spec->parts[grip]->specInstance(ge->spec, grip, ge->state, ge->dir.rotation());
							Point p = Point::Zero.transform(o) + ge->pos + Point::Up;
							Mat4 t = Mat4::translate(p.x, p.y + item->armV, p.z);
							(p.distanceSquared(position) < hdDistanceSquared ? items_hd: items_ld)[part].push_back(part->instance(t));
						}
					}
				}

				if (!ge->ghost && ge->spec->drone) {
					if (ge->drone.iid && hd) {
						Item* item = Item::get(ge->drone.iid);
						Point p = ge->pos - (Point::Up*0.75f);
						Mat4 t = Mat4::translate(p.x, p.y + item->armV, p.z);
						for (uint i = 0; i < item->parts.size(); i++) {
							Part* part = item->parts[i];
							(p.distanceSquared(position) < hdDistanceSquared ? items_hd: items_ld)[part].push_back(part->instance(t));
						}
					}
				}

				if (!ge->ghost && ge->spec->projector) {
					Sim::locked([&]() {
						Projector& projector = Projector::get(ge->id);

						if (projector.iid && hd) {
							Item* item = Item::get(projector.iid);
							Point p = ge->pos + (Point::Up*0.35f);
							float rot = (float)(frame%360) * DEG2RAD;
							float bob = std::sin(rot*4.0f)*0.1f;
							Mat4 t1 = Mat4::scale(0.75f) * Mat4::rotate(Point::Up, rot) * Mat4::translate(p.x, p.y+bob, p.z);
							for (uint i = 0; i < item->parts.size(); i++) {
								Part* part = item->parts[i];
								(p.distanceSquared(position) < hdDistanceSquared ? items_hd: items_ld)[part].push_back(part->instance(t1));
							}
						}
						if (projector.fid && hd) {
							Fluid* fluid = Fluid::get(projector.fid);
							Point p = ge->pos + (Point::Up*0.35f);
							float rot = (float)(frame%360) * DEG2RAD;
							float bob = std::sin(rot*4.0f)*0.1f;
							Mat4 t1 = Mat4::scale(0.75f) * Mat4::rotate(Point::Up, rot) * Mat4::translate(p.x, p.y+bob, p.z);
							Part* part = fluid->droplet;
							(p.distanceSquared(position) < hdDistanceSquared ? items_hd: items_ld)[part].push_back(part->instance(t1));
						}
					});
				}

				if (!ge->ghost && ge->spec->ropeway) {
					if (ge->ropeway.next) {
						DrawLine3D(ge->ropeway.a, ge->ropeway.b, BLACK);
						DrawLine3D(ge->ropeway.c, ge->ropeway.d, BLACK);
					}
				}
			}

			for (auto& [part,batch]: extant_ld) {
				part->drawInstanced(false, batch.size(), batch.data());
				objects += batch.size();
			}

			for (auto& [part,batch]: extant_hd) {
				part->drawInstanced(true, batch.size(), batch.data());
				objects += batch.size();
			}

			for (auto& [part,batch]: items_hd) {
				part->drawInstanced(true, batch.size(), batch.data());
				objects += batch.size();
			}

			for (auto& [part,batch]: items_ld) {
				part->drawInstanced(false, batch.size(), batch.data());
				objects += batch.size();
			}

			// Ghosts and the build grid are translucent so draw order matters
			gridSquareLand->drawGhostInstanced(true, gridSquares.size(), gridSquares.data());
			objects += gridSquares.size();

			for (auto& [part,batch]: ghosts_ld) {
				part->drawGhostInstanced(false, batch.size(), batch.data());
				objects += batch.size();
			}

			for (auto& [part,batch]: ghosts_hd) {
				part->drawGhostInstanced(true, batch.size(), batch.data());
				objects += batch.size();
			}

			for (auto& ge: entities) {
				if (ge->spec->explosion) {
					DrawSphere(ge->pos, ge->explosion.radius, RED);
				}
			}

			if (hovering) {
				drawBox(hovering->southBox(), hovering->dir, hovering->spec->operable() ? SKYBLUE: VIOLET);

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

				if (hovering->spec->turret) {
					DrawSphereWires(hovering->pos, hovering->spec->turretRange, 72, 72, RED);
				}

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

				minivec<Mat4> reds;
				minivec<Mat4> greens;

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

				objects += reds.size();
				objects += greens.size();
			}

			if (selecting) {
				Box box = Box(selection.a, selection.b);
				DrawCube(box.centroid(), box.w, box.h, box.d, GetColor(0x0000ff99));
				drawBox(box, Point::South, ORANGE);
			}

			//DrawSphere(groundZero, 0.5f, RED);
			//Sim::locked([&]() {
			//	DrawCubeWiresV(Chunk::tryGet(groundZero)->centroid(), {Chunk::size, 1, Chunk::size}, RED);
			//});

		EndMode3D();
	});
}