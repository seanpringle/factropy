
#include "common.h"
#include "panel.h"
#include "spec.h"
#include "chunk.h"
#include "sim.h"
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
	struct nk_user_font *symbol;
};

namespace {

	static enum nk_buttons buttonsRaylibNuklear[3];
	static int keysRaylibNuklear[512];

	static std::map<std::string,struct nk_image> images;

	struct nk_image nk_img(std::string name, Image image) {
		if (images.count(name) == 0) {
			struct nk_image img = nk_image_ptr(image.data);
			img.w = image.width;
			img.h = image.height;
			img.region[2] = image.width;
			img.region[3] = image.height;
			images[name] = img;
		}
		return images[name];
	}
}

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
	nuklear->symbol = nk_cairo_ttf("font/dejavu/DejaVuSans.ttf", 18);
	nk_init_default(&nuklear->ctx, NULL);
	nk_style_set_font(&nuklear->ctx, nuklear->font);
	changed = true;
	refresh = 0;
	mx = 0;
	my = 0;
	place();
	ZERO(buttons);
	ZERO(keys);
}

Panel::~Panel() {
	UnloadImage(canvas);
	UnloadTexture(texture);
	nk_free(&nuklear->ctx);
	delete nuklear;
}

void Panel::place() {
	x = (GetScreenWidth()-w)/2;
	y = (GetScreenHeight()-h)/2;
}

bool Panel::contains(int px, int py) {
	return px >= x && px < x+w && py >= y && py < y+h;
}

void Panel::render() {
	nk_cairo_render(&nuklear->ctx, canvas.data, w, h, w*4);
	UpdateTexture(texture, canvas.data);
}

void Panel::draw() {
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
	place();

	if (changed) {
		build();
		render();
		changed = false;
	}
}

void Panel::input() {
	//nk_clear(&nuklear->ctx);
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
		wy += GetMouseWheelMove();
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

	nk_input_end(&nuklear->ctx);

	if (IsKeyReleased(KEY_ESCAPE)) {
		camera->popup = NULL;
	}
}

MessagePopup::MessagePopup(int w, int h) : Panel(NULL, w, h) {
	text = "hello world";
}

void MessagePopup::build() {
	nk_begin(&nuklear->ctx, "message", nk_rect(0, 0, w, h), NK_WINDOW_BORDER);

	struct nk_vec2 content = nk_window_get_content_region_size(&nuklear->ctx);
	struct nk_vec2 spacing = nuklear->ctx.style.window.spacing;

	nk_layout_row_dynamic(&nuklear->ctx, content.y-spacing.y*2, 1);
	nk_label(&nuklear->ctx, text.c_str(), NK_TEXT_CENTERED);
	nk_end(&nuklear->ctx);
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
		if (nk_button_image(&nuklear->ctx, nk_img("spec"+spec->name, spec->image))) {
			camera->build(spec);
			camera->popup = NULL;
		}
	}
	nk_end(&nuklear->ctx);
}

EntityPopup::EntityPopup(MainCamera *cam, int w, int h) : Panel(cam, w, h) {
	eid = 0;
}

void EntityPopup::useEntity(uint ueid) {
	eid = ueid;
}

void EntityPopup::build() {
	Sim::locked([&]() {
		if (!eid || !Entity::exists(eid) || Entity::get(eid).isGhost()) {
			eid = 0;
			camera->popup = NULL;
			return;
		}

		Entity &en = Entity::get(eid);

		nk_begin(&nuklear->ctx, en.spec->name.c_str(), nk_rect(0, 0, w, h), NK_WINDOW_TITLE|NK_WINDOW_BORDER);

		float tilePix = 64.0f;
		struct nk_vec2 content = nk_window_get_content_region_size(&nuklear->ctx);
		struct nk_vec2 spacing = nuklear->ctx.style.window.spacing;

		if (en.spec->consumeChemical) {
			Burner& burner = en.burner();
			nk_layout_row_dynamic(&nuklear->ctx, 0, 2);
			nk_label(&nuklear->ctx, "Fuel", NK_TEXT_LEFT);
			nk_prog(&nuklear->ctx, (int)(burner.energy.portion(burner.buffer)*100), 100, NK_FIXED);
			for (Stack stack: burner.store.stacks) {
				Item* item = Item::get(stack.iid);
				nk_layout_row_template_begin(&nuklear->ctx, tilePix);
					nk_layout_row_template_push_static(&nuklear->ctx, tilePix);
					nk_layout_row_template_push_variable(&nuklear->ctx, tilePix);
				nk_layout_row_template_end(&nuklear->ctx);
				nk_image(&nuklear->ctx, nk_img("item"+item->name, item->image));
				nk_label(&nuklear->ctx, fmtc("%u", stack.size), NK_TEXT_LEFT);
			}
		}

		if (en.spec->crafter) {
			Crafter& crafter = en.crafter();
			nk_layout_row_dynamic(&nuklear->ctx, 0, 1);
			if (nk_button_label(&nuklear->ctx, crafter.recipe ? crafter.recipe->name.c_str(): "(no recipe)")) {
				camera->popup = camera->recipePopup;
				camera->recipePopup->useEntity(eid);
			}
			nk_prog(&nuklear->ctx, (int)(crafter.progress*100), 100, NK_FIXED);

			if (en.spec->recipeTags.count("mining")) {
				auto minables = Chunk::minables(en.box());
				nk_layout_row_dynamic(&nuklear->ctx, 0, minables.size());
				for (Stack stack: minables) {
					nk_label(&nuklear->ctx, fmtc("%s(%d)", Item::get(stack.iid)->name, stack.size), NK_TEXT_LEFT);
				}
			}
		}

		if (en.spec->arm) {
			Arm& arm = en.arm();
			nk_layout_row_dynamic(&nuklear->ctx, 0, 1);
			nk_prog(&nuklear->ctx, (int)(arm.orientation*100), 100, NK_FIXED);

			nk_layout_row_static(&nuklear->ctx, tilePix, tilePix, std::floor(content.x/(tilePix+spacing.x)));

			for (uint iid: arm.filter) {
				Item* item = Item::get(iid);
				if (nk_button_image(&nuklear->ctx, nk_img("item"+item->name, item->image))) {
					arm.filter.erase(iid);
				}
			}

			if (nk_button_label(&nuklear->ctx, "+")) {
				camera->itemPopup->revert = camera->popup;
				camera->popup = camera->itemPopup;

				camera->itemPopup->callback = [&](uint iid) {
					Sim::locked([&]() {
						if (eid && Entity::exists(eid)) {
							arm.filter.insert(iid);
						}
					});
				};
			}
		}

		if (en.spec->lift) {
			Lift& lift = en.lift();
			nk_layout_row_dynamic(&nuklear->ctx, 0, 1);
			nk_label(&nuklear->ctx, fmtc("%s", lift.mode == Lift::Raise ? "raise": "lower"), NK_TEXT_LEFT);
			nk_label(&nuklear->ctx, fmtc("%s",
				lift.stage == Lift::Lowered ? "lowered":
					lift.stage == Lift::Lowering ? "lowering":
						lift.stage == Lift::Raised ? "raised":
							lift.stage == Lift::Raising ? "raising": "wtf"), NK_TEXT_LEFT);
		}

		if (en.spec->store) {
			Store& store = en.store();

			Mass usage = store.usage();
			Mass limit = store.limit();

			if (limit > Mass(0)) {
				nk_layout_row_dynamic(&nuklear->ctx, 0, 2);
				nk_label(&nuklear->ctx, fmtc("Storage Capacity %s", limit.format()), NK_TEXT_LEFT);
				nk_prog(&nuklear->ctx, usage.portion(limit) * 100, 100, NK_FIXED);
			}

			// requester/provider
			if (en.spec->enableSetLower && en.spec->enableSetUpper) {

				if (store.levels.size() > 0 || !store.isEmpty()) {

					nk_layout_row_template_begin(&nuklear->ctx, tilePix);
						nk_layout_row_template_push_static(&nuklear->ctx, tilePix);
						nk_layout_row_template_push_static(&nuklear->ctx, tilePix);
						nk_layout_row_template_push_variable(&nuklear->ctx, tilePix);
						nk_layout_row_template_push_static(&nuklear->ctx, tilePix);
						nk_layout_row_template_push_variable(&nuklear->ctx, tilePix);
						nk_layout_row_template_push_static(&nuklear->ctx, tilePix);
						nk_layout_row_template_push_static(&nuklear->ctx, tilePix);
						nk_layout_row_template_push_static(&nuklear->ctx, tilePix);
						nk_layout_row_template_push_static(&nuklear->ctx, tilePix);
					nk_layout_row_template_end(&nuklear->ctx);

					uint clear = 0;
					uint down = 0;

					for (Store::Level level: store.levels) {
						Item *item = Item::get(level.iid);

						uint limit = std::max(level.upper, (uint)store.limit().value/item->mass.value);
						uint step  = std::max(1U, (uint)limit/100);

						nk_image(&nuklear->ctx, nk_img("item"+item->name, item->image));
						nk_label(&nuklear->ctx, fmtc("%d", store.count(level.iid)), NK_TEXT_CENTERED);

						int lower = (int)level.lower;
						nk_slider_int(&nuklear->ctx, 0, &lower, limit, step);
						nk_label(&nuklear->ctx, fmtc("%d", lower), NK_TEXT_LEFT);

						int upper = (int)level.upper;
						nk_slider_int(&nuklear->ctx, 0, &upper, limit, step);
						nk_label(&nuklear->ctx, fmtc("%d", upper), NK_TEXT_LEFT);

						store.levelSet(level.iid, (uint)lower, (uint)upper);

						nk_style_push_font(&nuklear->ctx, nuklear->symbol);

						struct nk_style_button style = nuklear->ctx.style.button;
						struct nk_color color = nk_rgb(0,128,0);
						if (store.isRequesting(level.iid) && store.count(level.iid) == 0) color = nk_rgb(128,0,0);
						if (store.isRequesting(level.iid) && store.count(level.iid)  > 0) color = nk_rgb(128,64,0);
						if (store.isActiveProviding(level.iid)) color = nk_rgb(128,128,0);
						style.text_normal = color;
						style.text_hover = color;
						nk_button_label_styled(&nuklear->ctx, &style, "●");

						if (nk_button_label(&nuklear->ctx, "↓")) {
							down = level.iid;
						}
						if (nk_button_label(&nuklear->ctx, "×")) {
							clear = level.iid;
						}
						nk_style_pop_font(&nuklear->ctx);
					}

					for (Stack stack: store.stacks) {
						if (store.level(stack.iid) != NULL) continue;
						Item *item = Item::get(stack.iid);

						nk_image(&nuklear->ctx, nk_img("item"+item->name, item->image));
						nk_label(&nuklear->ctx, fmtc("%d", store.count(stack.iid)), NK_TEXT_CENTERED);

						nk_label(&nuklear->ctx, "(not limited)", NK_TEXT_CENTERED);
						nk_label(&nuklear->ctx, "--", NK_TEXT_CENTERED);

						nk_label(&nuklear->ctx, "(not limited)", NK_TEXT_CENTERED);
						nk_label(&nuklear->ctx, "--", NK_TEXT_CENTERED);

						if (nk_button_label(&nuklear->ctx, "+")) {
							store.levelSet(stack.iid, 0, (uint)store.limit().value/item->mass.value);
						}

						nk_label(&nuklear->ctx, "--", NK_TEXT_CENTERED);
						nk_label(&nuklear->ctx, "--", NK_TEXT_CENTERED);
					}

					if (clear) {
						store.levelClear(clear);
					}

					if (down) {
						auto level = store.level(down);
						uint lower = level->lower;
						uint upper = level->upper;
						store.levelClear(down);
						store.levelSet(down, lower, upper);
					}
				}

				nk_layout_row_static(&nuklear->ctx, tilePix, tilePix, 1);
				if (nk_button_label(&nuklear->ctx, "+")) {
					camera->itemPopup->revert = camera->popup;
					camera->popup = camera->itemPopup;

					camera->itemPopup->callback = [&](uint iid) {
						Sim::locked([&]() {
							if (eid && Entity::exists(eid)) {
								Entity& en = Entity::get(eid);
								Store& store = en.store();
								uint lower = store.count(iid);
								uint upper = store.limit().value/Item::get(iid)->mass.value;
								lower += (lower%10 != 0) ? 10-(lower%10): 0;
								lower = std::min(lower, upper);
								store.levelSet(iid, lower, upper);
							}
						});
					};
				}
			}

			// requester
			if (en.spec->enableSetLower && !en.spec->enableSetUpper) {

				if (store.levels.size() > 0 || !store.isEmpty()) {

					nk_layout_row_template_begin(&nuklear->ctx, tilePix);
						nk_layout_row_template_push_static(&nuklear->ctx, tilePix);
						nk_layout_row_template_push_static(&nuklear->ctx, tilePix);
						nk_layout_row_template_push_variable(&nuklear->ctx, tilePix);
						nk_layout_row_template_push_static(&nuklear->ctx, tilePix);
						nk_layout_row_template_push_static(&nuklear->ctx, tilePix);
						nk_layout_row_template_push_static(&nuklear->ctx, tilePix);
						nk_layout_row_template_push_static(&nuklear->ctx, tilePix);
					nk_layout_row_template_end(&nuklear->ctx);

					uint clear = 0;
					uint down = 0;

					for (Store::Level level: store.levels) {
						Item *item = Item::get(level.iid);

						uint limit = std::max(level.upper, (uint)store.limit().value/item->mass.value);
						uint step  = std::max(1U, (uint)limit/100);

						nk_image(&nuklear->ctx, nk_img("item"+item->name, item->image));
						nk_label(&nuklear->ctx, fmtc("%d", store.count(level.iid)), NK_TEXT_CENTERED);

						int lower = (int)level.lower;
						nk_slider_int(&nuklear->ctx, 0, &lower, limit, step);
						nk_label(&nuklear->ctx, fmtc("%d", lower), NK_TEXT_LEFT);

						store.levelSet(level.iid, lower, lower);

						nk_style_push_font(&nuklear->ctx, nuklear->symbol);

						struct nk_style_button style = nuklear->ctx.style.button;
						struct nk_color color = nk_rgb(0,128,0);
						if (store.isRequesting(level.iid) && store.count(level.iid) == 0) color = nk_rgb(128,0,0);
						if (store.isRequesting(level.iid) && store.count(level.iid)  > 0) color = nk_rgb(128,64,0);
						if (store.isActiveProviding(level.iid)) color = nk_rgb(128,128,0);
						style.text_normal = color;
						style.text_hover = color;
						nk_button_label_styled(&nuklear->ctx, &style, "●");

						if (nk_button_label(&nuklear->ctx, "↓")) {
							down = level.iid;
						}
						if (nk_button_label(&nuklear->ctx, "×")) {
							clear = level.iid;
						}

						nk_style_pop_font(&nuklear->ctx);
					}

					for (Stack stack: store.stacks) {
						if (store.level(stack.iid) != NULL) continue;
						Item *item = Item::get(stack.iid);

						nk_image(&nuklear->ctx, nk_img("item"+item->name, item->image));
						nk_label(&nuklear->ctx, fmtc("%d", store.count(stack.iid)), NK_TEXT_CENTERED);

						nk_label(&nuklear->ctx, "(not limited)", NK_TEXT_CENTERED);
						nk_label(&nuklear->ctx, "--", NK_TEXT_CENTERED);

						if (nk_button_label(&nuklear->ctx, "+")) {
							store.levelSet(stack.iid, 0, (uint)store.limit().value/item->mass.value);
						}

						nk_label(&nuklear->ctx, "--", NK_TEXT_CENTERED);
						nk_label(&nuklear->ctx, "--", NK_TEXT_CENTERED);
					}

					if (clear) {
						store.levelClear(clear);
					}

					if (down) {
						auto level = store.level(down);
						uint lower = level->lower;
						uint upper = level->upper;
						store.levelClear(down);
						store.levelSet(down, lower, upper);
					}
				}

				nk_layout_row_static(&nuklear->ctx, tilePix, tilePix, 1);
				if (nk_button_label(&nuklear->ctx, "+")) {
					camera->itemPopup->revert = camera->popup;
					camera->popup = camera->itemPopup;

					camera->itemPopup->callback = [&](uint iid) {
						Sim::locked([&]() {
							if (eid && Entity::exists(eid)) {
								Entity& en = Entity::get(eid);
								Store& store = en.store();
								uint lower = store.count(iid);
								uint cap = store.limit().value/Item::get(iid)->mass.value;
								lower += (lower%10 != 0) ? 10-(lower%10): 0;
								lower = std::min(lower, cap);
								store.levelSet(iid, lower, lower);
							}
						});
					};
				}
			}

			// provider
			if (!en.spec->enableSetLower && en.spec->enableSetUpper) {

				if (store.levels.size() > 0 || !store.isEmpty()) {

					nk_layout_row_template_begin(&nuklear->ctx, tilePix);
						nk_layout_row_template_push_static(&nuklear->ctx, tilePix);
						nk_layout_row_template_push_static(&nuklear->ctx, tilePix);
						nk_layout_row_template_push_variable(&nuklear->ctx, tilePix);
						nk_layout_row_template_push_static(&nuklear->ctx, tilePix);
						nk_layout_row_template_push_static(&nuklear->ctx, tilePix);
						nk_layout_row_template_push_static(&nuklear->ctx, tilePix);
						nk_layout_row_template_push_static(&nuklear->ctx, tilePix);
					nk_layout_row_template_end(&nuklear->ctx);

					uint clear = 0;
					uint down = 0;

					for (Store::Level level: store.levels) {
						Item *item = Item::get(level.iid);

						uint limit = std::max(level.upper, (uint)store.limit().value/item->mass.value);
						uint step  = std::max(1U, (uint)limit/100);

						nk_image(&nuklear->ctx, nk_img("item"+item->name, item->image));
						nk_label(&nuklear->ctx, fmtc("%d", store.count(level.iid)), NK_TEXT_CENTERED);

						int upper = (int)level.upper;
						nk_slider_int(&nuklear->ctx, 0, &upper, limit, step);
						nk_label(&nuklear->ctx, fmtc("%d", upper), NK_TEXT_LEFT);

						store.levelSet(level.iid, 0, upper);

						nk_style_push_font(&nuklear->ctx, nuklear->symbol);

						struct nk_style_button style = nuklear->ctx.style.button;
						struct nk_color color = nk_rgb(0,128,0);
						if (store.isAccepting(level.iid) && store.count(level.iid) == 0) color = nk_rgb(128,0,0);
						if (store.isAccepting(level.iid) && store.count(level.iid) < level.upper) color = nk_rgb(128,64,0);
						if (store.isActiveProviding(level.iid)) color = nk_rgb(128,128,0);
						style.text_normal = color;
						style.text_hover = color;
						nk_button_label_styled(&nuklear->ctx, &style, "●");

						if (nk_button_label(&nuklear->ctx, "↓")) {
							down = level.iid;
						}
						if (nk_button_label(&nuklear->ctx, "×")) {
							clear = level.iid;
						}

						nk_style_pop_font(&nuklear->ctx);
					}

					for (Stack stack: store.stacks) {
						if (store.level(stack.iid) != NULL) continue;
						Item *item = Item::get(stack.iid);

						nk_image(&nuklear->ctx, nk_img("item"+item->name, item->image));
						nk_label(&nuklear->ctx, fmtc("%d", store.count(stack.iid)), NK_TEXT_CENTERED);

						nk_label(&nuklear->ctx, "(not limited)", NK_TEXT_CENTERED);
						nk_label(&nuklear->ctx, "--", NK_TEXT_CENTERED);

						nk_label(&nuklear->ctx, "--", NK_TEXT_CENTERED);

						if (nk_button_label(&nuklear->ctx, "+")) {
							store.levelSet(stack.iid, 0, (uint)store.limit().value/item->mass.value);
						}

						nk_label(&nuklear->ctx, "--", NK_TEXT_CENTERED);
						nk_label(&nuklear->ctx, "--", NK_TEXT_CENTERED);
					}

					if (clear) {
						store.levelClear(clear);
					}

					if (down) {
						auto level = store.level(down);
						uint lower = level->lower;
						uint upper = level->upper;
						store.levelClear(down);
						store.levelSet(down, lower, upper);
					}
				}

				nk_layout_row_static(&nuklear->ctx, tilePix, tilePix, 1);
				if (nk_button_label(&nuklear->ctx, "+")) {
					camera->itemPopup->revert = camera->popup;
					camera->popup = camera->itemPopup;

					camera->itemPopup->callback = [&](uint iid) {
						Sim::locked([&]() {
							if (eid && Entity::exists(eid)) {
								Entity& en = Entity::get(eid);
								Store& store = en.store();
								uint lower = 0;
								uint upper = store.limit().value/Item::get(iid)->mass.value;
								store.levelSet(iid, lower, upper);
							}
						});
					};
				}
			}

			// generic
			if (!en.spec->enableSetLower && !en.spec->enableSetUpper) {

				if (store.levels.size() > 0 || !store.isEmpty()) {

					nk_layout_row_template_begin(&nuklear->ctx, tilePix);
						nk_layout_row_template_push_static(&nuklear->ctx, tilePix);
						nk_layout_row_template_push_static(&nuklear->ctx, tilePix);
						nk_layout_row_template_push_static(&nuklear->ctx, tilePix);
						nk_layout_row_template_push_static(&nuklear->ctx, tilePix);
						nk_layout_row_template_push_static(&nuklear->ctx, tilePix);
						nk_layout_row_template_push_static(&nuklear->ctx, tilePix);
					nk_layout_row_template_end(&nuklear->ctx);

					for (Store::Level level: store.levels) {
						Item *item = Item::get(level.iid);

						nk_image(&nuklear->ctx, nk_img("item"+item->name, item->image));
						nk_label(&nuklear->ctx, fmtc("%d", store.count(level.iid)), NK_TEXT_CENTERED);
						nk_label(&nuklear->ctx, fmtc("%d", level.lower), NK_TEXT_CENTERED);
						nk_label(&nuklear->ctx, fmtc("%d", level.upper), NK_TEXT_CENTERED);
						nk_label(&nuklear->ctx, fmtc("%d", level.reserved), NK_TEXT_CENTERED);
						nk_label(&nuklear->ctx, fmtc("%d", level.promised), NK_TEXT_CENTERED);
					}

					for (Stack stack: store.stacks) {
						if (store.level(stack.iid) != NULL) continue;
						Item *item = Item::get(stack.iid);

						nk_image(&nuklear->ctx, nk_img("item"+item->name, item->image));
						nk_label(&nuklear->ctx, "--", NK_TEXT_CENTERED);
						nk_label(&nuklear->ctx, fmtc("%d", store.count(stack.iid)), NK_TEXT_CENTERED);
						nk_label(&nuklear->ctx, "--", NK_TEXT_CENTERED);
						nk_label(&nuklear->ctx, "--", NK_TEXT_CENTERED);
						nk_label(&nuklear->ctx, "--", NK_TEXT_CENTERED);
					}
				}
			}

			//nk_layout_row_dynamic(&nuklear->ctx, 0, 1);
			//nk_label(&nuklear->ctx, fmtc("logistic %d", en.spec->logistic), NK_TEXT_LEFT);
			//nk_layout_row_dynamic(&nuklear->ctx, 0, 1);
			//nk_label(&nuklear->ctx, fmtc("supplyPriority %d", en.spec->supplyPriority), NK_TEXT_LEFT);
			//nk_layout_row_dynamic(&nuklear->ctx, 0, 1);
			//nk_label(&nuklear->ctx, fmtc("loadPriority %d", en.spec->loadPriority), NK_TEXT_LEFT);
			//nk_layout_row_dynamic(&nuklear->ctx, 0, 1);
			//nk_label(&nuklear->ctx, fmtc("defaultOverflow %d", en.spec->defaultOverflow), NK_TEXT_LEFT);
			//nk_layout_row_dynamic(&nuklear->ctx, 0, 1);
			//nk_label(&nuklear->ctx, fmtc("loadAnything %d", en.spec->loadAnything), NK_TEXT_LEFT);
			//nk_layout_row_dynamic(&nuklear->ctx, 0, 1);
			//nk_label(&nuklear->ctx, fmtc("unloadAnything %d", en.spec->unloadAnything), NK_TEXT_LEFT);
		}

		nk_end(&nuklear->ctx);
		refresh = 1;
	});
}

EntityInfo::EntityInfo(MainCamera *cam, int w, int h) : Panel(cam, w, h) {
	eid = 0;
}

void EntityInfo::useEntity(uint ueid) {
	eid = ueid;
}

void EntityInfo::place() {
	x = GetScreenWidth()-w;
	y = 0;
}

void EntityInfo::build() {
	Sim::locked([&](){
		if (!eid) return;
		if (!Entity::exists(eid)) return;

		Entity& en = Entity::get(eid);

		nk_begin(&nuklear->ctx, fmtc("%s", en.spec->name), nk_rect(0, 0, w, h), NK_WINDOW_TITLE|NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR);

		//float tilePix = 128.0f;
		//struct nk_vec2 content = nk_window_get_content_region_size(&nuklear->ctx);
		//struct nk_vec2 spacing = nuklear->ctx.style.window.spacing;

		if (en.spec->consumeChemical) {
			Burner& burner = en.burner();
			nk_layout_row_dynamic(&nuklear->ctx, 0, 2);
			nk_label(&nuklear->ctx, fmtc("Fuel"), NK_TEXT_LEFT);
			nk_prog(&nuklear->ctx, (int)(burner.energy.portion(burner.buffer)*100), 100, NK_FIXED);
		}

		if (en.spec->consumeElectricity) {
			nk_layout_row_dynamic(&nuklear->ctx, 0, 2);
			nk_label(&nuklear->ctx, fmtc("Electricity"), NK_TEXT_LEFT);
			nk_prog(&nuklear->ctx, (int)(Entity::electricitySatisfaction*100), 100, NK_FIXED);
		}

		if (en.spec->generateElectricity) {
			nk_layout_row_dynamic(&nuklear->ctx, 0, 2);
			nk_label(&nuklear->ctx, fmtc("Generator"), NK_TEXT_LEFT);
			nk_prog(&nuklear->ctx, (int)(Entity::electricityLoad*100), 100, NK_FIXED);
		}

		if (en.spec->crafter) {
			Crafter& crafter = en.crafter();
			nk_layout_row_dynamic(&nuklear->ctx, 0, 2);

			nk_label(&nuklear->ctx, crafter.recipe ? crafter.recipe->name.c_str(): "(no recipe)", NK_TEXT_LEFT);
			nk_prog(&nuklear->ctx, (int)(crafter.progress*100), 100, NK_FIXED);

			if (en.spec->recipeTags.count("mining")) {
				auto minables = Chunk::minables(en.box());
				nk_layout_row_dynamic(&nuklear->ctx, 0, 1);
				for (Stack stack: minables) {
					nk_label(&nuklear->ctx, fmtc("%s(%d)", Item::get(stack.iid)->name, stack.size), NK_TEXT_LEFT);
				}
			}
		}

		nk_end(&nuklear->ctx);
		refresh = 1;
	});
}

GhostInfo::GhostInfo(MainCamera *cam, int w, int h) : Panel(cam, w, h) {
	ge = NULL;
}

void GhostInfo::useGhost(GuiFakeEntity* gge) {
	ge = gge;
}

void GhostInfo::place() {
	x = GetScreenWidth()-w;
	y = 0;
}

void GhostInfo::build() {
	Sim::locked([&](){
		if (!ge) return;

		nk_begin(&nuklear->ctx, fmtc("%s", ge->spec->name), nk_rect(0, 0, w, h), NK_WINDOW_TITLE|NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR);

		//float tilePix = 128.0f;
		//struct nk_vec2 content = nk_window_get_content_region_size(&nuklear->ctx);
		//struct nk_vec2 spacing = nuklear->ctx.style.window.spacing;

		if (ge->spec->crafter) {
			if (ge->spec->recipeTags.count("mining")) {
				auto minables = Chunk::minables(ge->box());
				nk_layout_row_dynamic(&nuklear->ctx, 0, minables.size());
				for (Stack stack: minables) {
					nk_label(&nuklear->ctx, fmtc("%s(%d)", Item::get(stack.iid)->name, stack.size), NK_TEXT_LEFT);
				}
			}
		}

		nk_end(&nuklear->ctx);
		refresh = 1;
	});
}

RecipePopup::RecipePopup(MainCamera *cam, int w, int h) : EntityPopup(cam, w, h) {
}

void RecipePopup::build() {
	Sim::locked([&](){
		if (!eid || !Entity::exists(eid)) {
			eid = 0;
			camera->popup = NULL;
			return;
		}

		Entity &en = Entity::get(eid);

		nk_begin(&nuklear->ctx, "Recipe", nk_rect(0, 0, w, h), NK_WINDOW_TITLE|NK_WINDOW_BORDER);

		float tilePix = 128.0f;
		struct nk_vec2 content = nk_window_get_content_region_size(&nuklear->ctx);
		struct nk_vec2 spacing = nuklear->ctx.style.window.spacing;

		nk_layout_row_static(&nuklear->ctx, tilePix, tilePix, std::floor(content.x/(tilePix+spacing.x)));

		for (auto [name,recipe]: Recipe::names) {
			bool show = true;
			if (en.spec->recipeTags.size()) {
				show = false;
				for (auto tag: en.spec->recipeTags) {
					show = show || recipe->tags.count(tag);
				}
			}
			if (show) {
				if (nk_button_image(&nuklear->ctx, nk_img("recipe"+name, recipe->image))) {
					en.crafter().nextRecipe = recipe;
					camera->popup = camera->entityPopup;
				}
			}
		}

		nk_end(&nuklear->ctx);
	});
}

ItemPopup::ItemPopup(MainCamera *cam, int w, int h) : Panel(cam, w, h) {
	revert = NULL;
	callback = NULL;
}

void ItemPopup::build() {
	nk_begin(&nuklear->ctx, "Item", nk_rect(0, 0, w, h), NK_WINDOW_TITLE|NK_WINDOW_BORDER);

	float tilePix = 64.0f;
	struct nk_vec2 content = nk_window_get_content_region_size(&nuklear->ctx);
	struct nk_vec2 spacing = nuklear->ctx.style.window.spacing;

	nk_layout_row_static(&nuklear->ctx, tilePix, tilePix, std::floor(content.x/(tilePix+spacing.x)));

	for (auto [name,item]: Item::names) {
		if (nk_button_image(&nuklear->ctx, nk_img("item"+name, item->image))) {
			camera->popup = revert;
			callback(item->id);
		}
	}

	nk_end(&nuklear->ctx);
}

StatsPopup::StatsPopup(MainCamera *cam, int w, int h) : Panel(cam, w, h) {
}

void StatsPopup::build() {

	std::function<void(TimeSeries*, std::string)> chart = [&](TimeSeries *ts, std::string title) {
		nk_layout_row_dynamic(&nuklear->ctx, 0, 1);
		nk_label(&nuklear->ctx, fmtc("%s : %fms", title, ts->secondMax), NK_TEXT_LEFT);
		nk_layout_row_dynamic(&nuklear->ctx, 100, 1);
		if (nk_chart_begin(&nuklear->ctx, NK_CHART_LINES, 59, 0, ts->secondMax)) {
			for (uint i = 59; i >= 1; i--) {
				nk_chart_push(&nuklear->ctx, i > camera->frame ? 0: ts->seconds[ts->second(camera->frame-i)]);
			}
			nk_chart_end(&nuklear->ctx);
		}
	};

	Sim::locked([&]() {
		nk_begin(&nuklear->ctx, "Stats", nk_rect(0, 0, w, h), NK_WINDOW_TITLE|NK_WINDOW_BORDER);

		nk_layout_row_dynamic(&nuklear->ctx, 0, 1);
		nk_label(&nuklear->ctx,
			fmtc("Electricity : %s/%s",
				Energy(Sim::statsElectricityDemand.minuteMax).formatRate(),
				Entity::electricityCapacity.formatRate()),
			NK_TEXT_LEFT);
		nk_layout_row_dynamic(&nuklear->ctx, 100, 1);
		if (nk_chart_begin(&nuklear->ctx, NK_CHART_LINES, 59, 0, (float)Entity::electricityCapacity.value)) {
			for (uint i = 59; i >= 1; i--) {
				nk_chart_push(&nuklear->ctx, (i*60) > Sim::tick ? 0: Sim::statsElectricityDemand.minutes[Sim::statsElectricityDemand.minute(Sim::tick-(i*60))]);
			}
			nk_chart_end(&nuklear->ctx);
		}

		chart(&camera->statsFrame, "frame");
		chart(&camera->statsUpdate, "MainCamera::update");
		chart(&camera->statsDraw, "MainCamera::draw");
		chart(&Sim::statsArm, "Arm::tick");
		chart(&Sim::statsCrafter, "Crafter::tick");
		chart(&Sim::statsPath, "Path::tick");
		chart(&Sim::statsVehicle, "Vehicle::tick");
		chart(&Sim::statsBelt, "Belt::tick");
		chart(&Sim::statsLift, "Lift::tick");
		chart(&Sim::statsDepot, "Depot::tick");
		chart(&Sim::statsDrone, "Drone::tick");

		nk_end(&nuklear->ctx);
		refresh = 1;
	});
}


