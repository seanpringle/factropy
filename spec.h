#ifndef _H_spec
#define _H_spec

#include <string>
#include <map>
#include "raylib.h"
#include "raymath.h"

struct Spec {
	std::string name;
	float w;
	float h;
	float d;
	std::string obj;
	Model model;

	Spec(std::string name);
};

namespace Specs {
	extern std::map<std::string,Spec*> all;

	void init();
	Spec* byName(std::string name);
}

#endif