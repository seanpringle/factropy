#ifndef _H_view
#define _H_view

struct View;
struct SiteCamera;
struct MainCamera;

#include "raylib.h"
#include "entity.h"
#include "panel.h"
#include "time-series.h"
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
	~SiteCamera();
	virtual void update();
	virtual void draw(RenderTexture canvas);
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

	Point nextPosition;
	Point nextDirection;
	bool moving;

	MouseState mouse;
	bool showGrid;

	bool selecting;
	Rectangle selection;

	GuiEntity* hovering;
	GuiFakeEntity* placing;
	bool placingFits;
	std::vector<GuiEntity*> entities;
	std::vector<GuiEntity*> hovered;
	std::vector<GuiEntity*> selected;

	bool worldFocused;

	Panel *popup;
	BuildPopup *buildPopup;
	EntityPopup *entityPopup;
	RecipePopup *recipePopup;
	ItemPopup *itemPopup;
	StatsPopup *statsPopup;
	bool popupFocused;

	uint resource;

	TimeSeries statsUpdate;
	TimeSeries statsDraw;
	TimeSeries statsFrame;

	MainCamera(Point, Point);
	~MainCamera();
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