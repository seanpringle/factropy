#ifndef _H_popup
#define _H_popup

struct Popup {
	Popup();
	virtual ~Popup();
	virtual void draw();
	void center();
};

struct StatsPopup2 : Popup {
	StatsPopup2();
	virtual ~StatsPopup2();
	virtual void draw();
};

struct WaypointsPopup : Popup {
	uint eid;
	WaypointsPopup();
	virtual ~WaypointsPopup();
	virtual void draw();
	void useEntity(uint eid);
};

struct TechPopup : Popup {
	TechPopup();
	virtual ~TechPopup();
	virtual void draw();
};

#endif
