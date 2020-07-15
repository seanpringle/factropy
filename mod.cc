#include "common.h"
#include "mod.h"
#include <map>
#include <fstream>
#include <stdlib.h>
#include <string.h>

namespace {
	void writeFn(WrenVM* vm, const char* text) {
		if (!strcmp(text, "\n")) {
			return;
		}
		Mod *mod = (Mod*)wrenGetUserData(vm);
		notef("mod %s: %s", mod->name, text);
	}

	void* reallocateFn(void* memory, size_t newSize) {
		if (newSize == 0) {
			free(memory);
			return NULL;
		}
		return realloc(memory, newSize);
	}

	char* managedString(const char *s) {
		char *buf = (char*)reallocateFn(NULL, strlen(s)+1);
		memmove(buf, s, strlen(s)+1);
		return buf;
	}

	char* loadModuleFn(WrenVM* vm, const char* name) {
		Mod *mod = (Mod*)wrenGetUserData(vm);
		char buf[1024];
		int len = 0;
		if (!strcmp(name, "_sim")) {
			len = snprintf(buf, 1024, "script/sim.wren");
		} else
		if (!strcmp(name, "_gui")) {
			len = snprintf(buf, 1024, "script/gui.wren");
		} else {
			len = snprintf(buf, 1024, "mods/%s/%s.wren", mod->name, name);
		}
		notef("loading %s", buf);
		std::string path = std::string(buf, len);
		std::ifstream ifs(path);
		std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
		if (strcmp(name, "_sim")) {
			content.insert(0, std::string("import \"_sim\" for Sim\n"));
		}
		if (strcmp(name, "_gui")) {
			content.insert(0, std::string("import \"_gui\" for Gui\n"));
		}
		return managedString(content.c_str());
	}

	typedef void (*cb)(WrenVM*);

	void simTick(WrenVM *vm) {
		wrenEnsureSlots(vm, 1);
		wrenSetSlotDouble(vm, 0, 42);
	}

	std::map<const std::string, std::map<const std::string, std::map<const std::string, const cb>>> lookup = {
		{ "_sim", {
			{ "Sim", {
				{ "tick()", simTick },
			}},
		}},
	};

	WrenForeignMethodFn bindForeignMethodFn(WrenVM* vm, const char* module, const char* className, bool isStatic, const char* signature) {
		notef("%s %s %s", module, className, signature);
		if (lookup.count(module) == 0) return NULL;
		if (lookup[module].count(className) == 0) return NULL;
		if (lookup[module][className].count(signature) == 0) return NULL;
		return lookup[module][className][signature];
	}

	void errorFn(WrenVM* vm, WrenErrorType type, const char* module, int line, const char* message) {
		Mod *mod = (Mod*)wrenGetUserData(vm);
		notef("mod error %s %s.wren:%d: %s", mod->name, module, line, message);
	}
}

Mod::Mod(const char *name) {
	this->name = name;

	WrenConfiguration config;
	wrenInitConfiguration(&config);
	config.writeFn = writeFn;
	config.reallocateFn = reallocateFn;
	config.loadModuleFn = loadModuleFn;
	config.bindForeignMethodFn = bindForeignMethodFn;
	config.errorFn = errorFn;
	vm = wrenNewVM(&config);
	wrenSetUserData(vm, this);
	
	ensuref(
		WREN_RESULT_SUCCESS == wrenInterpret(vm, "_mod", "import \"mod\" for Mod"),
		"mod load failed: %s", name
	);

	wrenEnsureSlots(vm, 1);
	wrenGetVariable(vm, "_mod", "Mod", 0);
	modClass = wrenGetSlotHandle(vm, 0);

	onLoad = wrenMakeCallHandle(vm, "load()");
	onUpdate = wrenMakeCallHandle(vm, "update()");
}

Mod::~Mod() {
	wrenReleaseHandle(vm, modClass);
	wrenReleaseHandle(vm, onLoad);
	wrenReleaseHandle(vm, onUpdate);
	wrenFreeVM(vm);
}

void Mod::call(WrenHandle *fn) {
	wrenEnsureSlots(vm, 1);
	wrenSetSlotHandle(vm, 0, modClass);
	ensure(WREN_RESULT_SUCCESS == wrenCall(vm, fn));
}

void Mod::load() {
	call(onLoad);
}

void Mod::update() {
	call(onUpdate);
}