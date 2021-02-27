#pragma once

#include <vector>
#include <functional>

struct TimeSeries {
	// circular buffers
	double tickMax;
	double secondMax;
	double minuteMax;
	double hourMax;
	double ticks[60];
	double seconds[60];
	double minutes[60];
	double hours[60];

	void clear();
	static uint tick(uint64_t t);
	static uint second(uint64_t t);
	static uint minute(uint64_t t);
	static uint hour(uint64_t t);
	void set(uint64_t t, double v);
	void add(uint64_t t, double v);
	void update(uint64_t t);
	void track(uint64_t t, std::function<void(void)> fn);
};

