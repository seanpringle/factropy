#ifndef _H_gui
#define _H_gui

#include "raylib.h"
#include "raymath.h"
#include "sim.h"
#include "entity.h"
#include "panel.h"

namespace Gui {

	struct MouseState {
		int x, y, dx, dy, wheel;
		bool left, leftChanged;
		bool middle, middleChanged;
		bool right, rightChanged;
		Ray ray;
	};

	extern Camera3D camera;
	extern MouseState mouse;

	extern GuiEntity* hovering;
	extern GuiFakeEntity* placing;
	extern std::vector<GuiEntity*> entities;
	extern std::vector<GuiEntity*> hovered;

	extern Panel *popup;
	extern BuildPopup *buildPopup;

	void reset();

	void updateMouseState();
	void updateCamera();

	void build(Spec* spec);
	void findEntities();	
};

#endif