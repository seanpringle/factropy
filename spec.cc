#include "common.h"
#include "spec.h"

namespace Specs {

	std::map<std::string,Spec*> all = {};

	Spec* byName(std::string name) {
		ensuref(all.count(name) == 1, "unknown spec name %s", name.c_str());
		return all[name];
	}
}

using namespace Specs;

Spec::Spec(std::string name) {
	ensuref(all.count(name) == 0, "duplicate spec name %s", name.c_str());
	notef("Spec: %s", name.c_str());
	this->name = name;
	all[name] = this;
}
