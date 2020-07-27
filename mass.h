#ifndef _H_mass
#define _H_mass

#include <string>

struct Mass {
	uint value;

	bool operator==(const Mass& o) const;
	bool operator!=(const Mass& o) const;
	bool operator<(const Mass& o) const;
	bool operator>(const Mass& o) const;
	Mass operator+(const Mass& o) const;
	Mass operator-(const Mass& o) const;
	void operator+=(const Mass& o);
	void operator-=(const Mass& o);

	float operator/(const Mass& o) const; // ratio

	Mass();
	Mass(float);
	std::string format();
};

#endif