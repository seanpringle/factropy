#include "common.h"
#include "gui.h"

namespace Gui {

	const float speedH = 0.005f;
	const float speedV = 0.005f;
	const float speedW = 0.25f;

	Camera3D camera;
	MouseState mouse;
	bool showGrid;

	GuiEntity* hovering = NULL;
	GuiFakeEntity* placing = NULL;
	std::vector<GuiEntity*> entities;
	std::vector<GuiEntity*> hovered;
	bool worldFocused;

	Panel *popup = NULL;
	BuildPopup *buildPopup = NULL;
	EntityPopup *entityPopup = NULL;
	bool popupFocused;

	Rectangle selectBox;

	void resetEntities() {
		hovering = NULL;
		hovered.clear();

		while (entities.size() > 0) {
			delete entities.back();
			entities.pop_back();
		}
	}

	void reset() {
		resetEntities();
	}

	void updateMouseState() {
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
			mouse.zW += speedW * -(float)mouse.wheel;
		}
	}

	void updateCamera() {
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

			Vector3 direction = Vector3Subtract(camera.position, camera.target);
			Vector3 right = Vector3Normalize(Vector3CrossProduct(direction, camera.up));
			Matrix rotateH = MatrixRotate(camera.up, -rDH);
			Matrix rotateV = MatrixRotate(right, rDV);
			Matrix rotate = MatrixMultiply(rotateH, rotateV);
			direction = Vector3Transform(direction, rotate);
			camera.position = Vector3Add(camera.target, direction);
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

			Vector3 direction = Vector3Subtract(camera.position, camera.target);
			Vector3 delta = Vector3Scale(direction, zDW);
			camera.position = Vector3Add(camera.position, delta);
		}

		mouse.ray = GetMouseRay((Vector2){(float)mouse.x, (float)mouse.y}, camera);
		SetCameraMode(camera, CAMERA_CUSTOM);
	}

	void build(Spec* spec) {
		delete placing;
		placing = NULL;
		if (spec) {
			placing = new GuiFakeEntity(spec);
		}
	}

	void findEntities() {
		resetEntities();

		Box view = (Box){camera.target.x, camera.target.y, camera.target.z, 1000, 1000, 1000};

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
					float d = Vector3Distance(ge->pos.vec(), camera.position);
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