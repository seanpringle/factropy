#ifndef _H_gui
#define _H_gui

#include "raylib.h"
#include "raymath.h"
#include "sim.h"
#include "entity.h"
#include "panel.h"

namespace Gui {

	struct MouseState {
		int x, y, dx, dy;
		int wheel;
		bool left, leftChanged;
		bool middle, middleChanged;
		bool right, rightChanged;
		float rH, rV, zW;
		Ray ray;
	};

	extern Camera3D camera;
	extern MouseState mouse;
	extern bool showGrid;

	extern GuiEntity* hovering;
	extern GuiFakeEntity* placing;
	extern std::vector<GuiEntity*> entities;
	extern std::vector<GuiEntity*> hovered;
	extern bool worldFocused;

	extern Panel *popup;
	extern BuildPopup *buildPopup;
	extern EntityPopup *entityPopup;
	extern bool popupFocused;

	extern Rectangle selectBox;

	void reset();

	void updateMouseState();
	void updateCamera();

	void build(Spec* spec);
	void findEntities();	

	void drawGrid(Vector3 p, int slices, float spacing);
};

#endif