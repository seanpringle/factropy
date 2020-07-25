#ifndef _H_common
#define _H_common

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <memory>

#define notef(...) { fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); }
#define fatalf(...) { notef(__VA_ARGS__); exit(EXIT_FAILURE); }

#define ensure(cond,...) if (!(cond)) { throw; }
#define ensuref(cond,...) if (!(cond)) { notef(__VA_ARGS__); throw; }

#define ZERO(s) memset(&s, 0, sizeof(s))

template <typename... Args>
std::string fmt(const std::string &fmt, Args... args) {
	size_t required = std::snprintf(NULL, 0, fmt.c_str(), args...)+1;
	char bytes[required];
	std::snprintf(bytes, required, fmt.c_str(), args...);
	return std::string(bytes);
}

#endif
