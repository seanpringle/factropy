
/*
Currently this uses the Cairo image backend (software renderer)
targeting an SDL surface. Eventually it should be possible to make
it use the Cairo OpenGL backend and share SDL's GL context.
*/

#include <stdarg.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_BUTTON_TRIGGER_ON_RELEASE
#define NK_IMPLEMENTATION
#define NK_CAIRO_IMPLEMENTATION
#include "nuklear.h"
#include "nuklear_cairo.h"

NK_GLOBAL struct nk_color nk_none = {0,0,0,0};

void _nuklear_hack() {
	nk_cos(1);
	nk_sin(1);
	nk_sqrt(1);
}
