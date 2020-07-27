#include "common.h"
#include "energy.h"

Energy::Energy(float m) {
	value = m;
}

std::string Energy::format() {
	return fmt("%f", value);
}