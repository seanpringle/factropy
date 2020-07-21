#ifndef _H_view
#define _H_view

struct View;
struct SiteCamera;
struct MainCamera;

#include "raylib.h"
#include "entity.h"
#include "panel.h"
#include <vector>

struct View {
	static inline Model waterCube;
	static constexpr float fovy = 45.0f;

	View();
	virtual ~View();
	virtual void update();
	virtual void draw();
	virtual void draw(RenderTexture canvas);
};

struct SiteCamera : View {
	Vector3 pos;
	Vector3 dir;
	std::vector<GuiEntity*> entities;
	uint64_t refresh;

	SiteCamera(Vector3, Vector3);
	virtual void update();
	virtual void draw(RenderTexture canvas);
};

struct MainCamera : View {
	Vector3 position;
	Vector3 direction;
	Vector3 up;

	Vector3 nextPosition;
	Vector3 nextDirection;
	bool moving;

	struct MouseXY {
		int x, y;
	};

	struct MouseButton {
		bool down;
		bool changed;
		MouseXY downAt, upAt, drag;
		bool dragged;
		bool pressed;
		bool released;
		bool clicked;
	};

	struct MouseState {
		int x, y, dx, dy;
		int wheel;
		float rH, rV, zW;
		Ray ray;

		MouseButton left;
		MouseButton right;
		MouseButton middle;
	};

	MouseState mouse;
	bool showGrid;

	GuiEntity* hovering;
	GuiFakeEntity* placing;
	std::vector<GuiEntity*> entities;
	std::vector<GuiEntity*> hovered;
	bool worldFocused;

	Panel *popup;
	BuildPopup *buildPopup;
	EntityPopup *entityPopup;
	bool popupFocused;

	MainCamera(Vector3, Vector3);
	virtual void update();
	virtual void draw();

	Vector3 groundTarget(float ground);
	Camera3D raylibCamera();
	void updateMouseState();
	void updateCamera();
	void lookAt(Point);
	void build(Spec*);
};

#endif