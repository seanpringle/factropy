#ifndef _H_common
#define _H_common

#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#define notef(...) { fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); }
#define fatalf(...) { notef(__VA_ARGS__); exit(EXIT_FAILURE); }

#define ensure(cond,...) if (!(cond)) { throw; }
#define ensuref(cond,...) if (!(cond)) { notef(__VA_ARGS__); throw; }

#define ZERO(s) memset(&(s), 0, sizeof((s)))

#endif
