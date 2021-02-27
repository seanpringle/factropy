#pragma once

// A 3D perspective View of the map, from a position, looking in a direction.
// The rendering thread is mostly driven by a MainCamera but multiple cameras
// can exist that render in different ways.

// Plan is to add the option of camera drones or security cameras that can
// display their view in the player's HUD or peripheral vision.

struct View;
struct SiteCamera;
struct MainCamera;

#include "raylib-ex.h"
#include "entity.h"
#include "plan.h"
#include "panel.h"
#include "time-series.h"
#include <vector>

struct View {
	static inline Model waterCube;
	static inline Model redCube;
	static inline Model greenCube;
	static inline Part* beltPillar1;
	static inline Part* beltPillar2;
	static inline Part* electricalCoverage;

	static constexpr float fovy = 45.0f;

	View();
	virtual ~View();
	virtual void update(bool worldFocused);
	virtual void draw();
	virtual void draw(RenderTexture canvas);
};

struct SiteCamera : View {
	Point pos;
	Point dir;
	std::vector<GuiEntity*> entities;
	uint64_t refresh;

	SiteCamera(Point, Point);
	~SiteCamera();
	void update(bool worldFocused);
	void draw(RenderTexture canvas);
	Point groundTarget(float ground);
};

struct MainCamera : View {

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

	uint64_t frame;

	Point position;
	Point direction;
	Point up;

	MouseState mouse;
	bool showGrid;

	bool selecting;

	struct {
		Point a;
		Point b;
	} selection;

	GuiEntity* hovering;
	GuiEntity* directing;
	GuiEntity* connecting;
	Plan* placing;
	bool placingFits;
	std::vector<GuiEntity*> entities;
	std::vector<GuiEntity*> hovered;
	std::vector<GuiEntity*> selected;

	float buildLevel;

	TimeSeries statsUpdate;
	TimeSeries statsDraw;
	TimeSeries statsFrame;

	MainCamera(Point, Point);
	~MainCamera();
	void update(bool worldFocused);
	void draw();

	Point groundTarget(float ground);
	Camera3D raylibCamera();
	void updateMouseState();
	void updateCamera();
	void lookAt(Point);
	void build(Spec* = nullptr, Point dir = Point::South);
};

