#include "common.h"
#include "time-series.h"
#include "raylib-ex.h"

void TimeSeries::clear() {
	tickMax = 0.0f;
	secondMax = 0.0f;
	minuteMax = 0.0f;
	hourMax = 0.0f;
	ZERO(ticks);
	ZERO(seconds);
	ZERO(minutes);
	ZERO(hours);
}

uint TimeSeries::tick(uint64_t t) {
	return t%60;
}

uint TimeSeries::second(uint64_t t) {
	return (t/60)%60;
}

uint TimeSeries::minute(uint64_t t) {
	return (t/60/60)%60;
}

uint TimeSeries::hour(uint64_t t) {
	return (t/60/60/60)%60;
}

void TimeSeries::set(uint64_t t, double v) {
	ticks[tick(t)] = v;
}

void TimeSeries::add(uint64_t t, double v) {
	ticks[tick(t)] += v;
}

void TimeSeries::update(uint64_t t) {
	tickMax = 0.0;
	secondMax = 0.0;
	minuteMax = 0.0;
	hourMax = 0.0;

	double tsum = 0.0;
	for (uint i = 0; i < 60; i++) {
		tsum += ticks[i];
		tickMax = std::max(tickMax, ticks[i]);
	}
	uint s = second(t);
	seconds[s] = tsum/60.0;

	double msum = 0.0;
	for (uint i = 0; i < 60; i++) {
		msum += seconds[i];
		secondMax = std::max(secondMax, seconds[i]);
	}
	uint m = minute(t);
	minutes[m] = msum/60.0;

	double hsum = 0;
	for (uint i = 0; i < 60; i++) {
		hsum += minutes[i];
		minuteMax = std::max(minuteMax, minutes[i]);
	}
	uint h = hour(t);
	hours[h] = hsum/60.0;

	for (uint i = 0; i < 60; i++) {
		hourMax = std::max(hourMax, hours[i]);
	}
}

void TimeSeries::track(uint64_t t, std::function<void(void)> fn) {
	double start = GetTime();
	fn();
	set(t, (GetTime()-start)*1000.0f);
	update(t);
}