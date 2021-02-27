#pragma once

// ImGUI popup windows.

#include "view.h"

struct Popup {
	MainCamera* camera = nullptr;
	bool visible = false;
	bool mouseOver = false;
	Popup(MainCamera* c);
	virtual ~Popup();
	virtual void draw();
	void show(bool state = true);
	void center(int w, int h);
};

struct MessagePopup : Popup {
	std::string text;
	MessagePopup(MainCamera* c);
	~MessagePopup();
	void draw();
};

struct StatsPopup2 : Popup {
	StatsPopup2(MainCamera* c);
	~StatsPopup2();
	void draw();
};

struct WaypointsPopup : Popup {
	uint eid = 0;
	WaypointsPopup(MainCamera* c);
	~WaypointsPopup();
	void draw();
	void useEntity(uint eid);
};

struct TechPopup : Popup {
	TechPopup(MainCamera* c);
	~TechPopup();
	void draw();
};

struct BuildPopup2 : Popup {
	BuildPopup2(MainCamera* c);
	~BuildPopup2();
	void draw();
};

struct EntityPopup2 : Popup {
	uint eid = 0;
	EntityPopup2(MainCamera* c);
	~EntityPopup2();
	void draw();
	void useEntity(uint eid);
};
