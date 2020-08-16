#include "common.h"
#include "currency.h"

Currency Currency::k(int n) {
	return n*1000;
}

Currency::Currency() {
	value = 0;
}

Currency::Currency(int m) {
	value = m;
}

bool Currency::operator==(const Currency& o) const {
	return value == o.value;
}

bool Currency::operator!=(const Currency& o) const {
	return value != o.value;
}

bool Currency::operator<(const Currency& o) const {
	return value < o.value;
}

bool Currency::operator<=(const Currency& o) const {
	return value <= o.value;
}

bool Currency::operator>(const Currency& o) const {
	return value > o.value;
}

bool Currency::operator>=(const Currency& o) const {
	return value >= o.value;
}

Currency Currency::operator+(const Currency& o) const {
	return value + o.value;
}

Currency Currency::operator*(uint n) const {
	return value * n;
}

Currency Currency::operator-(const Currency& o) const {
	return value - o.value;
}

void Currency::operator+=(const Currency& o) {
	value += o.value;
}

void Currency::operator-=(const Currency& o) {
	value -= o.value;
}

Currency::operator bool() const {
	return value != 0;
}

std::string Currency::format() {
	if (value < 1000) {
		return fmt("$%d", value);
	}
	return fmt("$%dK", value/1000);
}

float Currency::portion(Currency o) {
	if (o.value == 0) return 1.0f;
	return std::max(0.0f, std::min(1.0f, (float)value / (float)o.value));
}
