#ifndef _H_popup
#define _H_popup

struct Popup {
	Popup();
	~Popup();
	void center();
	virtual void draw();
};

struct StatsPopup2 : Popup {
	StatsPopup2();
	virtual ~StatsPopup2();
	virtual void draw();
};

#endif
