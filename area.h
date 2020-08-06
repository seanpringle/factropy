#ifndef _H_area
#define _H_area

#include <initializer_list>

struct Area {
	float w;
	float d;

	Area();
	Area(std::initializer_list<float>);
	Area(float w, float d);

	operator bool() const;
};

#endif
