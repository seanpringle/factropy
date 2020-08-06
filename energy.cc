#include "common.h"
#include "energy.h"

Energy Energy::kJ(int j) {
	return Energy(j*1000);
}

Energy Energy::MJ(int j) {
	return Energy(j*1000000);
}

Energy Energy::kW(int j) {
	return Energy((int)std::ceil((float)(j*1000)/60.0f));
}

Energy Energy::MW(int j) {
	return Energy((int)std::ceil((float)(j*1000000)/60.0f));
}

Energy::Energy() {
	value = 0;
}

Energy::Energy(int n) {
	value = n;
}

bool Energy::operator==(const Energy& o) const {
	return value == o.value;
}

bool Energy::operator!=(const Energy& o) const {
	return value != o.value;
}

bool Energy::operator<(const Energy& o) const {
	return value < o.value;
}

bool Energy::operator<=(const Energy& o) const {
	return value <= o.value;
}

bool Energy::operator>(const Energy& o) const {
	return value > o.value;
}

bool Energy::operator>=(const Energy& o) const {
	return value >= o.value;
}

Energy Energy::operator+(const Energy& o) const {
	return value + o.value;
}

Energy Energy::operator-(const Energy& o) const {
	return value - o.value;
}

Energy Energy::operator*(float n) const {
	return (int)((float)value * n);
}

void Energy::operator+=(const Energy& o) {
	value += o.value;
}

void Energy::operator-=(const Energy& o) {
	value -= o.value;
}

Energy::operator bool() const {
	return value != 0;
}

std::string Energy::format() const {
	if (value < 1000) {
		return fmt("%dJ", value);
	}
	if (value < 1000000) {
		return fmt("%dkJ", value/1000);
	}
	return fmt("%dMJ", value/1000000);
}

std::string Energy::formatRate() const {
	int v = value*60;
	if (v < 1000) {
		return fmt("%dW", v);
	}
	if (v < 1000000) {
		return fmt("%dkW", v/1000);
	}
	return fmt("%dMW", v/1000000);
}

float Energy::portion(Energy o) {
	if (o.value == 0) return 1.0f;
	return std::max(0.0f, std::min(1.0f, (float)value / (float)o.value));
}
