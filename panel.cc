
#include "common.h"
#include "panel.h"
#include "spec.h"
#include "chunk.h"
#include <cmath>

extern "C" {
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define STBTT_malloc(x,u) nk_malloc(((nk_handle){.id=0}),NULL,x)
#define STBTT_free(x,u) nk_mfree(((nk_handle){.id=0}),x)
#undef __cplusplus
#include "nuklear/nuklear.h"
#include "nuklear/nuklear_cairo.h"
#define __cplusplus
}

struct Nuklear {
	struct nk_context ctx;
	struct nk_user_font *font;
};

static enum nk_buttons buttonsRaylibNuklear[3];
static int keysRaylibNuklear[512];

static std::map<std::string,struct nk_image> images;

namespace Panels {
	void init() {
		nk_cairo_init();

		ZERO(buttonsRaylibNuklear);
		ZERO(keysRaylibNuklear);

		buttonsRaylibNuklear[MOUSE_LEFT_BUTTON]   = NK_BUTTON_LEFT;
		buttonsRaylibNuklear[MOUSE_MIDDLE_BUTTON] = NK_BUTTON_MIDDLE;
		buttonsRaylibNuklear[MOUSE_RIGHT_BUTTON]  = NK_BUTTON_RIGHT;

		keysRaylibNuklear[KEY_SPACE]        = KEY_SPACE;
		keysRaylibNuklear[KEY_APOSTROPHE]   = KEY_APOSTROPHE;
		keysRaylibNuklear[KEY_COMMA]        = KEY_COMMA;
		keysRaylibNuklear[KEY_MINUS]        = KEY_MINUS;
		keysRaylibNuklear[KEY_PERIOD]       = KEY_PERIOD;
		keysRaylibNuklear[KEY_SLASH]        = KEY_SLASH;
		keysRaylibNuklear[KEY_ZERO]         = KEY_ZERO;
		keysRaylibNuklear[KEY_ONE]          = KEY_ONE;
		keysRaylibNuklear[KEY_TWO]          = KEY_TWO;
		keysRaylibNuklear[KEY_THREE]        = KEY_THREE;
		keysRaylibNuklear[KEY_FOUR]         = KEY_FOUR;
		keysRaylibNuklear[KEY_FIVE]         = KEY_FIVE;
		keysRaylibNuklear[KEY_SIX]          = KEY_SIX;
		keysRaylibNuklear[KEY_SEVEN]        = KEY_SEVEN;
		keysRaylibNuklear[KEY_EIGHT]        = KEY_EIGHT;
		keysRaylibNuklear[KEY_NINE]         = KEY_NINE;
		keysRaylibNuklear[KEY_SEMICOLON]    = KEY_SEMICOLON;
		keysRaylibNuklear[KEY_EQUAL]        = KEY_EQUAL;
		keysRaylibNuklear[KEY_A]            = KEY_A;
		keysRaylibNuklear[KEY_B]            = KEY_B;
		keysRaylibNuklear[KEY_C]            = KEY_C;
		keysRaylibNuklear[KEY_D]            = KEY_D;
		keysRaylibNuklear[KEY_E]            = KEY_E;
		keysRaylibNuklear[KEY_F]            = KEY_F;
		keysRaylibNuklear[KEY_G]            = KEY_G;
		keysRaylibNuklear[KEY_H]            = KEY_H;
		keysRaylibNuklear[KEY_I]            = KEY_I;
		keysRaylibNuklear[KEY_J]            = KEY_J;
		keysRaylibNuklear[KEY_K]            = KEY_K;
		keysRaylibNuklear[KEY_L]            = KEY_L;
		keysRaylibNuklear[KEY_M]            = KEY_M;
		keysRaylibNuklear[KEY_N]            = KEY_N;
		keysRaylibNuklear[KEY_O]            = KEY_O;
		keysRaylibNuklear[KEY_P]            = KEY_P;
		keysRaylibNuklear[KEY_Q]            = KEY_Q;
		keysRaylibNuklear[KEY_R]            = KEY_R;
		keysRaylibNuklear[KEY_S]            = KEY_S;
		keysRaylibNuklear[KEY_T]            = KEY_T;
		keysRaylibNuklear[KEY_U]            = KEY_U;
		keysRaylibNuklear[KEY_V]            = KEY_V;
		keysRaylibNuklear[KEY_W]            = KEY_W;
		keysRaylibNuklear[KEY_X]            = KEY_X;
		keysRaylibNuklear[KEY_Y]            = KEY_Y;
		keysRaylibNuklear[KEY_Z]            = KEY_Z;

		keysRaylibNuklear[KEY_LEFT_SHIFT]   = NK_KEY_SHIFT;
		keysRaylibNuklear[KEY_LEFT_CONTROL] = NK_KEY_CTRL;
		keysRaylibNuklear[KEY_DELETE]       = NK_KEY_DEL;
		keysRaylibNuklear[KEY_ENTER]        = NK_KEY_ENTER;
		keysRaylibNuklear[KEY_TAB]          = NK_KEY_TAB;
		keysRaylibNuklear[KEY_BACKSPACE]    = NK_KEY_BACKSPACE;
		keysRaylibNuklear[KEY_UP]           = NK_KEY_UP;
		keysRaylibNuklear[KEY_DOWN]         = NK_KEY_DOWN;
		keysRaylibNuklear[KEY_LEFT]         = NK_KEY_LEFT;
		keysRaylibNuklear[KEY_RIGHT]        = NK_KEY_RIGHT;
	}
}

Panel::Panel(MainCamera *cam, int w, int h) {
	camera = cam;
	this->w = w;
	this->h = h;
	canvas = GenImageColor(w, h, BLACK);
	ImageFormat(&canvas, UNCOMPRESSED_R8G8B8A8);
	texture = LoadTextureFromImage(canvas);
	nuklear = new Nuklear;
	nuklear->font = nk_cairo_ttf("font/Roboto-Regular.ttf", 18);
	nk_init_default(&nuklear->ctx, NULL);
	nk_style_set_font(&nuklear->ctx, nuklear->font);
	changed = true;
	refresh = 0;
	mx = 0;
	my = 0;
	center();
}

Panel::~Panel() {
	UnloadImage(canvas);
	UnloadTexture(texture);
	nk_free(&nuklear->ctx);
	delete nuklear;
}

void Panel::center() {
	x = (GetScreenWidth()-w)/2;
	y = (GetScreenHeight()-h)/2;
}

bool Panel::contains(int px, int py) {
	return px >= x && px < x+w && py >= y && py < y+h;
}

void Panel::draw() {
	nk_cairo_render(&nuklear->ctx, canvas.data, w, h, w*4);
	UpdateTexture(texture, canvas.data);
	DrawTexture(texture, x, y, WHITE);
}

void Panel::build() {
	nk_begin(&nuklear->ctx, "Panel", nk_rect(0, 0, w, h), NK_WINDOW_TITLE|NK_WINDOW_BORDER);
		nk_layout_row_dynamic(&nuklear->ctx, 0, 1);
		nk_label(&nuklear->ctx, "hello world", NK_TEXT_LEFT);
	nk_end(&nuklear->ctx);
}

void Panel::update() {
	if (refresh == 1) {
		changed = true;
	}

	if (refresh > 0) {
		refresh--;
	}
	
	input();
	center();

	if (changed) {
		build();
		changed = false;
	}
}

void Panel::input() {
	nk_clear(&nuklear->ctx);
	nk_input_begin(&nuklear->ctx);

	if (contains(GetMouseX(), GetMouseY())) {
		int nmx = GetMouseX()-x;
		int nmy = GetMouseY()-y;
		if (nmx != mx || nmy != my) {
			mx = nmx;
			my = nmy;
			nk_input_motion(&nuklear->ctx, mx, my);
			changed = true;
		}
	}

	if (GetMouseWheelMove() != 0) {
		wy = GetMouseWheelMove();
		nk_input_scroll(&nuklear->ctx, (struct nk_vec2){0, (float)wy});
		changed = true;
	}

	for (int i = 0; i < 3; i++) {
		if (buttons[i] && IsMouseButtonReleased(i)) {
			nk_input_button(&nuklear->ctx, buttonsRaylibNuklear[i], mx, my, false);
			buttons[i] = false;
			changed = true;
		}
	}

	for (int i = 0; i < 3; i++) {
		if (!buttons[i] && IsMouseButtonPressed(i)) {
			nk_input_button(&nuklear->ctx, buttonsRaylibNuklear[i], mx, my, true);
			buttons[i] = true;
			changed = true;
		}
	}

	for (int i = 0; i < 512; i++) {
		if (keys[i] && !IsKeyPressed(i)) {
			if (i > 255) {
				nk_input_key(&nuklear->ctx, (enum nk_keys)keysRaylibNuklear[i], false);
			}
			else
			if (i > 0) {
				nk_input_char(&nuklear->ctx, (enum nk_keys)keysRaylibNuklear[i]);
			}
			keys[i] = false;
			changed = true;
		}
	}

	for (int i = 0; i < 512; i++) {
		if (!keys[i] && IsKeyPressed(i)) {
			if (i > 255) {
				nk_input_key(&nuklear->ctx, (enum nk_keys)keysRaylibNuklear[i], true);
			}
			keys[i] = true;
			changed = true;
		}
	}

	if (IsKeyReleased(KEY_ESCAPE)) {
		camera->popup = NULL;
	}

	nk_input_end(&nuklear->ctx);
}

BuildPopup::BuildPopup(MainCamera *cam, int w, int h) : Panel(cam, w, h) {
}

void BuildPopup::build() {
	nk_begin(&nuklear->ctx, "Build", nk_rect(0, 0, w, h), NK_WINDOW_TITLE|NK_WINDOW_BORDER);

	struct nk_vec2 content = nk_window_get_content_region_size(&nuklear->ctx);
	struct nk_vec2 spacing = nuklear->ctx.style.window.spacing;

	float tilePix = 128.0f;

	nk_layout_row_static(&nuklear->ctx, tilePix, tilePix, std::floor(content.x/(tilePix+spacing.x)));

	for (auto pair: Spec::all) {
		Spec* spec = pair.second;

		if (images.count(spec->name) == 0) {
			struct nk_image img = nk_image_ptr(spec->image.data);
			img.w = spec->image.width;
			img.h = spec->image.height;
			img.region[2] = spec->image.width;
			img.region[3] = spec->image.height;
			images[spec->name] = img;
		}

		if (nk_button_image_label(&nuklear->ctx, images[spec->name], spec->name.c_str(), NK_TEXT_CENTERED)) {
			camera->build(spec);
			camera->popup = NULL;
		}
	}
	nk_end(&nuklear->ctx);
}

EntityPopup::EntityPopup(MainCamera *cam, int w, int h) : Panel(cam, w, h) {
	ge = NULL;
}

void EntityPopup::useEntity(GuiEntity *uge) {
	if (ge) {
		delete ge;
	}
	ge = uge;
}

void EntityPopup::build() {
	if (!ge) {
		camera->popup = NULL;
		return;
	}

	nk_begin(&nuklear->ctx, ge->spec->name.c_str(), nk_rect(0, 0, w, h), NK_WINDOW_TITLE|NK_WINDOW_BORDER);

	nk_layout_row_dynamic(&nuklear->ctx, 0, 1);
	for (auto [x,y]: Chunk::walk(ge->box())) {
		auto sx = std::to_string(x);
		auto sy = std::to_string(y);
		nk_label(&nuklear->ctx, (sx + "," + sy).c_str(), NK_TEXT_LEFT);

		for (int id: Entity::grid[(Chunk::XY){x,y}]) {
			auto sid = std::to_string(id);
			nk_label(&nuklear->ctx, sid.c_str(), NK_TEXT_LEFT);
		}
	}

	nk_end(&nuklear->ctx);
}
