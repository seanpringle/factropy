#ifndef _H_panel
#define _H_panel

struct Nuklear;

class Panel {
public:
	int x;
	int y;
	int w;
	int h;
	Image canvas;
	Texture2D texture;

	struct Nuklear *nuklear;

	bool buttons[3];
	bool keys[512];

	Panel(int w, int h);
	virtual ~Panel();
	void center();
	void draw();
	virtual void update();
	bool contains(int x, int y);
	void input();
};

namespace Panels {
	void init();
}

class BuildPopup : public Panel {
public:
	BuildPopup(int w, int h);
	void update() override;
};

#endif