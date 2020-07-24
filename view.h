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
	static inline Model redCube;
	static inline Model greenCube;
	static constexpr float fovy = 45.0f;

	View();
	virtual ~View();
	virtual void update();
	virtual void draw();
	virtual void draw(RenderTexture canvas);
};

struct SiteCamera : View {
	Point pos;
	Point dir;
	std::vector<GuiEntity*> entities;
	uint64_t refresh;

	SiteCamera(Point, Point);
	virtual void update();
	virtual void draw(RenderTexture canvas);
};

struct MainCamera : View {
	Point position;
	Point direction;
	Point up;

	Point nextPosition;
	Point nextDirection;
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

	bool selecting;
	Rectangle selection;

	GuiEntity* hovering;
	GuiFakeEntity* placing;
	std::vector<GuiEntity*> entities;
	std::vector<GuiEntity*> hovered;
	std::vector<GuiEntity*> selected;

	bool worldFocused;

	Panel *popup;
	BuildPopup *buildPopup;
	EntityPopup *entityPopup;
	bool popupFocused;

	MainCamera(Point, Point);
	virtual void update();
	virtual void draw();

	Point groundTarget(float ground);
	Camera3D raylibCamera();
	void updateMouseState();
	void updateCamera();
	void lookAt(Point);
	void build(Spec*);
};

#endif