#ifndef _H_panel
#define _H_panel

class Panel;
class BuildPopup;
class EntityPopup;
class RecipePopup;
class ItemPopup;
class StatsPopup;

#include "view.h"
#include "entity.h"
#include <set>

struct Nuklear;

class Panel {
public:
	MainCamera *camera;
	int x;
	int y;
	int w;
	int h;
	int mx;
	int my;
	int wy;
	int refresh;

	struct Nuklear *nuklear;

	Image canvas;
	Texture2D texture;

	bool changed;
	bool buttons[3];
	bool keys[512];

	Panel(MainCamera *cam, int w, int h);
	virtual ~Panel();
	void center();
	void render();
	void draw();
	virtual void build();
	void update();
	bool contains(int x, int y);
	void input();
};

namespace Panels {
	void init();
}

class MessagePopup : public Panel {
public:
	std::string text;
	MessagePopup(int w, int h);
	void build() override;
};

class BuildPopup : public Panel {
public:
	BuildPopup(MainCamera *cam, int w, int h);
	void build() override;
};

class EntityPopup : public Panel {
public:
	uint eid;
	uint iidSelected;
	EntityPopup(MainCamera *cam, int w, int h);
	void useEntity(uint eid);
	void build() override;
};

class RecipePopup : public EntityPopup {
public:
	RecipePopup(MainCamera *cam, int w, int h);
	void build() override;
};

class ItemPopup : public Panel {
public:
	Panel *revert;
	std::function<void(uint)> callback;

	ItemPopup(MainCamera *cam, int w, int h);
	void build() override;
};

class StatsPopup : public Panel {
public:
	StatsPopup(MainCamera *cam, int w, int h);
	void build() override;
};

#endif