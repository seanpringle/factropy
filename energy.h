#ifndef _H_energy
#define _H_energy

struct Energy;

#include <string>
#include <set>

struct Energy {
	static Energy kJ(int j);
	static Energy MJ(int j);
	static Energy kW(int j); // joules/tick
	static Energy MW(int j); // joules/tick

	int value;

	bool operator==(const Energy& o) const;
	bool operator!=(const Energy& o) const;
	bool operator<(const Energy& o) const;
	bool operator<=(const Energy& o) const;
	bool operator>(const Energy& o) const;
	bool operator>=(const Energy& o) const;
	Energy operator+(const Energy& o) const;
	Energy operator-(const Energy& o) const;
	Energy operator*(float n) const;
	void operator+=(const Energy& o);
	void operator-=(const Energy& o);

	//float operator/(const Energy& o) const; // ratio

	operator bool() const;

	Energy();
	Energy(int);
	std::string format() const;
	std::string formatRate() const;
	float portion(Energy o);
};

#endif