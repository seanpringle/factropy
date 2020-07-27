#ifndef _H_energy
#define _H_energy

#include <string>

struct Energy {
	float value;

	Energy(float);
	std::string format();
};

#endif