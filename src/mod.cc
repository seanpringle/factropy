#include "common.h"
#include "mod.h"
#include "sim.h"
#include "item.h"
#include "part.h"
#include <map>
#include <fstream>
#include <stdlib.h>
#include <string.h>

namespace {
	void writeFn(WrenVM* vm, const char* text) {
		if (!strcmp(text, "\n")) {
			return;
		}
		ModWren *mod = (ModWren*)wrenGetUserData(vm);
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
		ModWren *mod = (ModWren*)wrenGetUserData(vm);
		char buf[1024];
		int len = 0;

		if (!strcmp(name, "_sim")) {
			len = snprintf(buf, 1024, "script/sim.wren");
		}

		if (!len) {
			len = snprintf(buf, 1024, "mods/%s/%s.wren", mod->name, name);
		}

		notef("loading %s %s", name, buf);

		std::string path = std::string(buf, len);
		std::ifstream ifs(path);
		std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

		if (strcmp(name, "_sim")) {
			content.insert(0, std::string("import \"_sim\" for Sim\n"));
		}

		return managedString(content.c_str());
	}

	typedef void (*cb)(WrenVM*);

	enum simRefType {
		simRefNone = 0,
		simRefItem,
		simRefPart,
	};

	struct simRef {
		simRefType type = simRefNone;
		union {
			uint id;
			Part* part;
		};
	};

	void simRefDel(void *data) {
	}

	void simTick(WrenVM* vm) {
		wrenEnsureSlots(vm, 1);
		wrenSetSlotDouble(vm, 0, Sim::tick);
	}

	std::map<const std::string, std::map<const std::string, std::map<const std::string, const cb>>> lookup = {
		{ "_sim", {
			{ "Sim", {
				{ "tick()", simTick },
			}},
		}},
	};

	WrenForeignMethodFn bindForeignMethodFn(WrenVM* vm, const char* module, const char* className, bool isStatic, const char* signature) {
		notef("foreign method %s %s %s", module, className, signature);
		if (lookup.count(module) == 0) return NULL;
		if (lookup[module].count(className) == 0) return NULL;
		if (lookup[module][className].count(signature) == 0) return NULL;
		return lookup[module][className][signature];
	}

	WrenForeignClassMethods bindForeignClassFn(WrenVM* vm, const char* module, const char* className) {
		notef("foreign class: %s %s", module, className);
		ensure(lookup[module][className].count("_allocate"));
		return (WrenForeignClassMethods){
			.allocate = lookup[module][className]["_allocate"],
			.finalize = simRefDel,
		};
	}

	void errorFn(WrenVM* vm, WrenErrorType type, const char* module, int line, const char* message) {
		ModWren *mod = (ModWren*)wrenGetUserData(vm);
		notef("mod error %s %s.wren:%d: %s", mod->name, module, line, message);
	}
}

ModWren::ModWren(const char *name) : Mod(name) {

	WrenConfiguration config;
	wrenInitConfiguration(&config);
	config.writeFn = writeFn;
	config.reallocateFn = reallocateFn;
	config.loadModuleFn = loadModuleFn;
	config.bindForeignMethodFn = bindForeignMethodFn;
	config.bindForeignClassFn = bindForeignClassFn;
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

ModWren::~ModWren() {
	wrenReleaseHandle(vm, modClass);
	wrenReleaseHandle(vm, onLoad);
	wrenReleaseHandle(vm, onUpdate);
	wrenFreeVM(vm);
}

void ModWren::call(WrenHandle *fn) {
	wrenEnsureSlots(vm, 1);
	wrenSetSlotHandle(vm, 0, modClass);
	ensure(WREN_RESULT_SUCCESS == wrenCall(vm, fn));
}

void ModWren::load() {
	call(onLoad);
}

void ModWren::update() {
	call(onUpdate);
}

namespace {
	duk_ret_t dukPrint(duk_context *ctx) {
		int n = duk_get_top(ctx);
		for (int i = 0; i < n; i++) {
			fprintf(stderr, "%s", duk_to_string(ctx, i));
			if (i < n-1) fprintf(stderr, " ");
		}
		fprintf(stderr, "\n");
		return 0;
	}

	bool objGet(duk_context* ctx, int idx, const char* key, char* buf, int len) {
		if (duk_is_object(ctx, idx) && duk_has_prop_string(ctx, idx, key)) {
			duk_get_prop_string(ctx, idx, key);
			std::snprintf(buf, len, "%s", duk_to_string(ctx, -1));
			duk_pop(ctx);
			return true;
		}
		return false;
	}

	bool objGet(duk_context* ctx, int idx, const char* key, bool* d) {
		if (duk_is_object(ctx, idx) && duk_has_prop_string(ctx, idx, key)) {
			duk_get_prop_string(ctx, idx, key);
			*d = duk_to_boolean(ctx, -1);
			duk_pop(ctx);
			return true;
		}
		return false;
	}

	bool objGet(duk_context* ctx, int idx, const char* key, uint* d) {
		if (duk_is_object(ctx, idx) && duk_has_prop_string(ctx, idx, key)) {
			duk_get_prop_string(ctx, idx, key);
			*d = duk_to_uint(ctx, -1);
			duk_pop(ctx);
			return true;
		}
		return false;
	}

	bool objGet(duk_context* ctx, int idx, const char* key, double* d) {
		if (duk_is_object(ctx, idx) && duk_has_prop_string(ctx, idx, key)) {
			duk_get_prop_string(ctx, idx, key);
			*d = duk_to_number(ctx, -1);
			duk_pop(ctx);
			return true;
		}
		return false;
	}

	bool objGet(duk_context* ctx, int idx, const char* key, Point* p) {
		bool ok = false;
		if (duk_is_object(ctx, idx) && duk_has_prop_string(ctx, idx, key)) {
			duk_get_prop_string(ctx, idx, key);
				int vidx = duk_get_top_index(ctx);
				if (duk_is_object(ctx, vidx)) {
					duk_get_prop_index(ctx, vidx, 0);
					duk_get_prop_index(ctx, vidx, 1);
					duk_get_prop_index(ctx, vidx, 2);
					*p = Point(
						duk_to_number(ctx, -3),
						duk_to_number(ctx, -2),
						duk_to_number(ctx, -1)
					);
					duk_pop_3(ctx);
					ok = true;
				}
			duk_pop(ctx);
		}
		return ok;
	}

	duk_ret_t dukSimAdditem(duk_context* ctx) {
		ensure(duk_is_object(ctx, 0) && duk_has_prop_string(ctx, 0, "name"));

		char name[100];
		objGet(ctx, 0, "name", name, sizeof(name));
		auto item = new Item(Item::next(), name);

		if (duk_get_prop_string(ctx, 0, "parts")) {

			int i = 0;
			while (duk_get_prop_index(ctx, -1, i++)) {
				int idx = duk_get_top_index(ctx);

				char hdSTL[100];
				char ldSTL[100];

				objGet(ctx, idx, "hdSTL", hdSTL, sizeof(hdSTL));
				Part* part = objGet(ctx, idx, "ldSTL", ldSTL, sizeof(ldSTL))
					? new Part(Thing(hdSTL, ldSTL)) : new Part(Thing(hdSTL));

				uint paint = 0;
				objGet(ctx, idx, "paint", &paint);
				part->paint(paint);

				Point scale;

				if (objGet(ctx, idx, "scale", &scale)) {
					part->scale(scale.x, scale.y, scale.z);
				}

				Point translate;

				if (objGet(ctx, idx, "translate", &translate)) {
					part->translate(translate.x, translate.y, translate.z);
				}

				double gloss = 0;
				if (objGet(ctx, idx, "gloss", &gloss)) {
					part->gloss(gloss);
				}

				item->parts.push_back(part);

				duk_pop(ctx);
			}
			duk_pop(ctx);
		}
		duk_pop(ctx);

		double armV = 0;
		if (objGet(ctx, 0, "armV", &armV)) {
			item->armV = armV;
		}

		bool minable = false;
		if (objGet(ctx, 0, "minable", &minable)) {
			Item::mining.insert(item->id);
		}

		if (duk_get_prop_string(ctx, 0, "fuel")) {
			int fuel = duk_get_top_index(ctx);

			bool ok = duk_is_object(ctx, fuel);
			ok = ok && duk_get_prop_string(ctx, fuel, "type");
			ok = ok && duk_get_prop_string(ctx, fuel, "energy");
			ensure(ok);

			item->fuel = Fuel(duk_to_string(ctx, -2), duk_to_int(ctx, -1));
			duk_pop_2(ctx);
		}
		duk_pop(ctx);

		return 0;
	}
}

ModDuktape::ModDuktape(const char* name) : Mod(name) {
	ctx = duk_create_heap_default();

	duk_push_c_function(ctx, dukPrint, DUK_VARARGS);
	duk_put_global_string(ctx, "print");

	duk_push_object(ctx);
		duk_push_object(ctx);
			duk_push_c_function(ctx, dukSimAdditem, 1);
			duk_put_prop_string(ctx, -2, "new");
		duk_put_prop_string(ctx, -2, "item");
	duk_put_global_string(ctx, "sim");

	std::string path = std::string("mods/base/mod.js");
	std::ifstream ifs(path);
	std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

	duk_eval_string_noresult(ctx, content.c_str());
}

ModDuktape::~ModDuktape() {
	duk_destroy_heap(ctx);
}

void ModDuktape::load() {
	duk_get_global_string(ctx, "load");
	duk_call(ctx, 0);
	duk_pop(ctx);
}

void ModDuktape::update() {
	duk_get_global_string(ctx, "update");
	duk_call(ctx, 0);
	duk_pop(ctx);
}
