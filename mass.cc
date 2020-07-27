#include "common.h"
#include "mass.h"

Mass::Mass() {
	value = 0;
}

Mass::Mass(float m) {
	value = std::floor(m*1000.0f);
}

bool Mass::operator==(const Mass& o) const {
	return value == o.value;
}

bool Mass::operator!=(const Mass& o) const {
	return value != o.value;
}

bool Mass::operator<(const Mass& o) const {
	return value < o.value;
}

bool Mass::operator>(const Mass& o) const {
	return value > o.value;
}

Mass Mass::operator+(const Mass& o) const {
	return value + o.value;
}

Mass Mass::operator-(const Mass& o) const {
	return value - o.value;
}

void Mass::operator+=(const Mass& o) {
	value += o.value;
}

void Mass::operator-=(const Mass& o) {
	value -= o.value;
}

float Mass::operator/(const Mass& o) const {
	return (float)value / (float)o.value;
}

std::string Mass::format() {
	if (value < 1000) {
		return fmt("%dg", value);
	}
	return fmt("%dkg", value/1000);
}
