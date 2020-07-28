#ifndef _H_time_series
#define _H_time_series

#include <vector>
#include <functional>

struct TimeSeries {
	// circular buffers
	double secondMax;
	double minuteMax;
	double hourMax;
	double seconds[60];
	double minutes[60];
	double hours[60];

	void clear();
	uint second(uint64_t t);
	uint minute(uint64_t t);
	uint hour(uint64_t t);
	void set(uint64_t t, double v);
	void add(uint64_t t, double v);
	void update(uint64_t t);
	void track(uint64_t t, std::function<void(void)> fn);
};

#endif