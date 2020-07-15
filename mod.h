#ifndef _H_mod
#define _H_mod

#include "wren/wren.hpp"
#include <string>

class Mod {
private:
	void call(WrenHandle *fn);

public:
	const char* name;
	WrenVM *vm;
	WrenHandle *modClass;
	WrenHandle *onLoad;
	WrenHandle *onUpdate;

	Mod(const char* name);
	~Mod();
	void load();
	void update();
};

#endif