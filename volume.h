#pragma once

#include <initializer_list>

struct Volume {
	float w;
	float h;
	float d;

	Volume();
	Volume(std::initializer_list<float>);
	Volume(float w, float h, float d);

	operator bool() const;
};
