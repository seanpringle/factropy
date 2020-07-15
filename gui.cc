#include "common.h"
#include "gui.h"

namespace Gui {

	const float speedH = 0.005f;
	const float speedV = 0.005f;

	Camera3D camera;
	MouseState mouse;

	GuiEntity* hovering = NULL;
	GuiFakeEntity* placing = NULL;
	std::vector<GuiEntity*> entities;
	std::vector<GuiEntity*> hovered;

	Panel *popup = NULL;
	BuildPopup *buildPopup = NULL;

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
		};
	}

	void updateCamera() {
		if (mouse.right) {
			float rH = speedH * mouse.dx;
			float rV = speedV * mouse.dy;
			Vector3 direction = Vector3Subtract(camera.position, camera.target);
			Vector3 right = Vector3CrossProduct(direction, camera.up);
			Matrix rotateH = MatrixRotate(camera.up, -rH);
			Matrix rotateV = MatrixRotate(right, rV);
			camera.position = Vector3Transform(camera.position, rotateH);
			camera.position = Vector3Transform(camera.position, rotateV);
		}

		if (mouse.wheel) {
			Vector3 delta = Vector3Scale(camera.position, mouse.wheel < 0 ? 0.25: -0.25);
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
		hovering = NULL;
		hovered.clear();

		while (entities.size() > 0) {
			delete entities.back();
			entities.pop_back();
		}

		Sim::locked([&]() {
			for (auto en: Entity::all) {
				float d = Vector3Distance(en.pos.vec(), camera.target);
				if (d < 500) {
					entities.push_back(new GuiEntity(en.id));
				}
			}
		});

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