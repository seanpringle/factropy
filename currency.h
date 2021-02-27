#pragma once

#include <string>

struct Currency {
	int value;

	bool operator==(const Currency& o) const;
	bool operator!=(const Currency& o) const;
	bool operator<(const Currency& o) const;
	bool operator<=(const Currency& o) const;
	bool operator>(const Currency& o) const;
	bool operator>=(const Currency& o) const;
	Currency operator+(const Currency& o) const;
	Currency operator*(uint n) const;
	Currency operator-(const Currency& o) const;
	Currency operator-() const;
	void operator+=(const Currency& o);
	void operator-=(const Currency& o);

	//float operator/(const Currency& o) const; // ratio

	operator bool() const;

	static Currency k(int n);

	Currency();
	Currency(int);
	std::string format();

	float portion(Currency o);
};
