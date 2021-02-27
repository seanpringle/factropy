#include "common.h"
#include "energy.h"

Energy Energy::J(int j) {
	return Energy(j);
}

Energy Energy::kJ(int j) {
	return Energy(j*1000);
}

Energy Energy::MJ(int j) {
	return Energy(j*1000000);
}

Energy Energy::W(int j) {
	return Energy((int)std::ceil((float)j/60.0f));
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

Energy::operator double() const {
	return value;
}

Energy::operator float() const {
	return value;
}

Energy::operator int() const {
	return value;
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
	if (o.value == 0) return 0.0f;
	return std::max(0.0f, std::min(1.0f, (float)value / (float)o.value));
}

Energy Energy::magnitude() {
	if (value == 0) return 1;
	double n = value*60;
	bool negative = n < 0;
	double log = std::log10(std::abs(n));
	double decimalPlaces = ((log > 0)) ? (std::ceil(log)) : (std::floor(log) + 1);
	double rounded = std::pow(10, decimalPlaces)/60;
	return std::round(negative ? -rounded : rounded);
}
