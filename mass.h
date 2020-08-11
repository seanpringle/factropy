#ifndef _H_mass
#define _H_mass

#include <string>

struct Mass {
	int value;

	bool operator==(const Mass& o) const;
	bool operator!=(const Mass& o) const;
	bool operator<(const Mass& o) const;
	bool operator<=(const Mass& o) const;
	bool operator>(const Mass& o) const;
	bool operator>=(const Mass& o) const;
	Mass operator+(const Mass& o) const;
	Mass operator*(uint n) const;
	Mass operator-(const Mass& o) const;
	void operator+=(const Mass& o);
	void operator-=(const Mass& o);

	//float operator/(const Mass& o) const; // ratio

	operator bool() const;

	static const Mass Inf;
	static Mass kg(int g);

	Mass();
	Mass(int);
	std::string format();

	float portion(Mass o);
	uint items(uint iid);
};

#endif