#pragma once

#define WREN_OPT_RANDOM 0

#include "../wren/src/include/wren.hpp"
#include <string>

#include "../duktape/duktape.h"

class Mod {
public:
	const char* name;
	Mod(const char* nname) {
		name = nname;
	};
	virtual ~Mod() {};
	virtual void load() = 0;
	virtual void update() = 0;
};

class ModWren : public Mod {
private:
	void call(WrenHandle *fn);

public:
	WrenVM *vm;
	WrenHandle *modClass;
	WrenHandle *onLoad;
	WrenHandle *onUpdate;

	ModWren(const char* name);
	~ModWren();
	void load();
	void update();
};


class ModDuktape : public Mod {

public:
	duk_context* ctx;

	ModDuktape(const char* name);
	~ModDuktape();
	void load();
	void update();
};

