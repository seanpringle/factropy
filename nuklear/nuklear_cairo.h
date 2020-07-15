/*

Copyright 2020 Sean Pringle <sean.pringle@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

A frame buffer backend for Nuklear using Cairo software rendering.

Example:

int main(...) {

	nk_cairo_init();
	struct nk_user_font *font = nk_cairo_ttf("path/to/font.ttf", 24);

	struct nk_context ctx;
	nk_init_default(&ctx, font);

	int width = 1000;
	int height = 1000;
	int pitch = width * sizeof(uint32_t);
	void *frame = malloc(height * pitch);

	nk_begin(&ctx, nk_rect(0, 0, width, height), ...);
		// usual Nuklear stuff
	nk_end(&ctx);

	nk_cairo_render(&ctx, frame, width, height, pitch);
}

*/

/*
 * ==============================================================
 *
 *                              API
 *
 * ===============================================================
 */

#ifndef NK_CAIRO_H_
#define NK_CAIRO_H_

NK_API void nk_cairo_init();
NK_API struct nk_user_font *nk_cairo_ttf(const char *ttf, int ttf_size);
NK_API void nk_cairo_render(struct nk_context *ctx, void *fb, int w, int h, int pitch);

#endif

/*
 * ==============================================================
 *
 *                          IMPLEMENTATION
 *
 * ===============================================================
 */

#ifdef NK_CAIRO_IMPLEMENTATION

#include <cairo.h>
#include <ft2build.h>
#include <cairo-ft.h>
#include FT_SFNT_NAMES_H
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_BBOX_H
#include FT_TYPE1_TABLES_H

typedef struct {
	const char *ttf;
	FT_Face ft;
	cairo_font_face_t *ca;
} nk_cairo_face_t;

typedef struct {
	struct nk_user_font font;
	int face;
	int size;
} nk_cairo_font_t;

#define MAXFONTS 16

/* Fonts are shared between Nuklear contexts */
FT_Library nk_cairo_ft;
int nk_cairo_face;
nk_cairo_face_t nk_cairo_faces[MAXFONTS];
int nk_cairo_font;
nk_cairo_font_t nk_cairo_fonts[MAXFONTS];
/* A cairo_t with a tiny surface used only for calculating font extents */
cairo_t *nk_cairo;

static void
nk_cairo_set_color(cairo_t *cairo, struct nk_color col)
{
	cairo_set_source_rgba(
		cairo,
		(double)col.r/255.0,
		(double)col.g/255.0,
		(double)col.b/255.0,
		(double)col.a/255.0
	);
}

static void
nk_cairo_scissor(cairo_t *cairo, float x, float y, float w, float h)
{
	cairo_reset_clip(cairo);
	cairo_new_path(cairo);
	cairo_move_to(cairo, x, y);
	cairo_line_to(cairo, x+w, y);
	cairo_line_to(cairo, x+w, y+h);
	cairo_line_to(cairo, x, y+h);
	cairo_line_to(cairo, x, y);
	cairo_clip(cairo);
}

static void
nk_cairo_stroke_line(cairo_t *cairo, int x0, int y0, int x1, int y1, int line_thickness, struct nk_color col)
{
	nk_cairo_set_color(cairo, col);
	cairo_set_line_width(cairo, line_thickness);
	cairo_move_to(cairo, x0, y0);
	cairo_line_to(cairo, x1, y1);
	cairo_stroke(cairo);
}

static void
nk_cairo_stroke_polygon(cairo_t *cairo, struct nk_vec2i *pnts, int count, int line_thickness, struct nk_color col)
{
	int i;
	nk_cairo_set_color(cairo, col);
	cairo_set_line_width(cairo, line_thickness);
	cairo_new_sub_path(cairo);
	cairo_move_to(cairo, pnts[0].x, pnts[0].y);
	for (i = 1; i < count; i++) {
		cairo_line_to(cairo, pnts[i].x, pnts[i].y);
	}
	cairo_line_to(cairo, pnts[0].x, pnts[0].y);
	cairo_close_path(cairo);
	cairo_stroke(cairo);
}

static void
nk_cairo_fill_polygon(cairo_t *cairo, struct nk_vec2i *pnts, int count, struct nk_color col)
{
	int i;
	nk_cairo_set_color(cairo, col);
	cairo_new_sub_path(cairo);
	cairo_move_to(cairo, pnts[0].x, pnts[0].y);
	for (i = 1; i < count; i++) {
		cairo_line_to(cairo, pnts[i].x, pnts[i].y);
	}
	cairo_line_to(cairo, pnts[0].x, pnts[0].y);
	cairo_close_path(cairo);
	cairo_fill(cairo);
}

static void
nk_cairo_stroke_arc(cairo_t *cairo, int x, int y, int r, float a0, float a1, int line_thickness, struct nk_color col)
{
	nk_cairo_set_color(cairo, col);
	cairo_set_line_width(cairo, line_thickness);
	cairo_new_sub_path(cairo);
	cairo_arc(cairo, x, y, r, a0, a1);
	cairo_close_path(cairo);
	cairo_stroke(cairo);
}

static void
nk_cairo_fill_arc(cairo_t *cairo, int x, int y, int r, float a0, float a1, struct nk_color col)
{
	nk_cairo_set_color(cairo, col);
	cairo_new_sub_path(cairo);
	cairo_arc(cairo, x, y, r, a0, a1);
	cairo_close_path(cairo);
	cairo_fill(cairo);
}

static void
nk_cairo_fill_circle(cairo_t *cairo, int x, int y, int r, struct nk_color col)
{
	nk_cairo_set_color(cairo, col);
	cairo_new_sub_path(cairo);
	cairo_arc(cairo, x, y, r, 0, 2*NK_PI);
	cairo_close_path(cairo);
	cairo_fill(cairo);
}

static void
nk_cairo_stroke_circle(cairo_t *cairo, int x, int y, int r, int line_thickness, struct nk_color col)
{
	nk_cairo_set_color(cairo, col);
	cairo_set_line_width(cairo, line_thickness);
	cairo_new_sub_path(cairo);
	cairo_arc(cairo, x, y, r, 0, 2*NK_PI);
	cairo_close_path(cairo);
	cairo_stroke(cairo);
}

static void
nk_cairo_stroke_rect(cairo_t *cairo, int x, int y, int w, int h, int r, int line_thickness, struct nk_color col)
{
	nk_cairo_set_color(cairo, col);
	cairo_set_line_width(cairo, line_thickness);

	if (r == 0) {
		cairo_rectangle(cairo, x, y, w, h);
	} else {
		double degrees = NK_PI / 180.0;
		cairo_new_sub_path(cairo);
		/* cairo_arc automatically adds the straight edges to the path */
		cairo_arc(cairo, x + w - r, y + r, r, -90 * degrees, 0 * degrees);
		cairo_arc(cairo, x + w - r, y + h - r, r, 0 * degrees, 90 * degrees);
		cairo_arc(cairo, x + r, y + h - r, r, 90 * degrees, 180 * degrees);
		cairo_arc(cairo, x + r, y + r, r, 180 * degrees, 270 * degrees);
		cairo_close_path(cairo);
	}
	cairo_stroke(cairo);
}

static void
nk_cairo_fill_rect(cairo_t *cairo, int x, int y, int w, int h, int r, struct nk_color col)
{
	nk_cairo_set_color(cairo, col);

	if (r == 0) {
		cairo_rectangle(cairo, x, y, w, h);
	} else {
		double degrees = NK_PI / 180.0;
		cairo_new_sub_path(cairo);
		/* cairo_arc automatically adds the straight edges to the path */
		cairo_arc(cairo, x + w - r, y + r, r, -90 * degrees, 0 * degrees);
		cairo_arc(cairo, x + w - r, y + h - r, r, 0 * degrees, 90 * degrees);
		cairo_arc(cairo, x + r, y + h - r, r, 90 * degrees, 180 * degrees);
		cairo_arc(cairo, x + r, y + r, r, 180 * degrees, 270 * degrees);
		cairo_close_path(cairo);
	}
	cairo_fill(cairo);
}

static void
nk_cairo_stroke_triangle(cairo_t *cairo, int x0, int y0, int x1, int y1, int x2, int y2, int line_thickness,	struct nk_color col)
{
	nk_cairo_set_color(cairo, col);
	cairo_set_line_width(cairo, line_thickness);
	cairo_new_sub_path(cairo);
	cairo_move_to(cairo, x0, y0);
	cairo_line_to(cairo, x1, y1);
	cairo_line_to(cairo, x2, y2);
	cairo_line_to(cairo, x0, y0);
	cairo_close_path(cairo);
	cairo_stroke(cairo);
}

static void
nk_cairo_fill_triangle(cairo_t *cairo, int x0, int y0, int x1, int y1, int x2, int y2, struct nk_color col)
{
	nk_cairo_set_color(cairo, col);
	cairo_new_sub_path(cairo);
	cairo_move_to(cairo, x0, y0);
	cairo_line_to(cairo, x1, y1);
	cairo_line_to(cairo, x2, y2);
	cairo_line_to(cairo, x0, y0);
	cairo_close_path(cairo);
	cairo_fill(cairo);
}

static void
nk_cairo_stroke_polyline(cairo_t *cairo, struct nk_vec2i *pnts, int count, int line_thickness, struct nk_color col)
{
	int i;
	nk_cairo_set_color(cairo, col);
	cairo_set_line_width(cairo, line_thickness);
	cairo_new_sub_path(cairo);
	cairo_move_to(cairo, pnts[0].x, pnts[0].y);
	for (i = 1; i < count; i++) {
		cairo_line_to(cairo, pnts[i].x, pnts[i].y);
	}
	cairo_close_path(cairo);
	cairo_stroke(cairo);
}

NK_API void
nk_cairo_drawimage(cairo_t *cairo, int x, int y, int w, int h, struct nk_image *img, struct nk_color *col)
{
	double xs = (double)w/(double)img->w;
	double ys = (double)h/(double)img->h;
	cairo_matrix_t matrix;
	cairo_get_matrix(cairo, &matrix);
	cairo_scale(cairo, xs, ys);

	cairo_surface_t *surface = cairo_image_surface_create_for_data(img->handle.ptr, CAIRO_FORMAT_ARGB32, img->w, img->h, img->w*sizeof(uint32_t));
	cairo_set_source_surface(cairo, surface, x/xs, y/ys);
	cairo_paint(cairo);
	cairo_surface_destroy(surface);

	cairo_set_matrix(cairo, &matrix);
}

NK_API void
nk_cairo_draw_text(cairo_t *cairo, struct nk_user_font *font,	struct nk_rect rect, const char *text, int len, float font_height, struct nk_color fg)
{
	nk_cairo_font_t *cfont = font->userdata.ptr;

	cairo_set_font_face(cairo, nk_cairo_faces[cfont->face].ca);
	cairo_set_font_size(cairo, cfont->size);

	cairo_font_extents_t extents;
	cairo_font_extents(cairo, &extents);

	char *tmp = malloc(len+1);
	memmove(tmp, text, len);
	tmp[len] = 0;

	nk_cairo_set_color(cairo, fg);
	cairo_move_to(cairo, rect.x, rect.y+extents.ascent);
	cairo_show_text(cairo, tmp);

	free(tmp);
}

static float nk_cairo_text_width(nk_handle handle, float h, const char *text, int len) {
	char *tmp = malloc(len+1);
	memmove(tmp, text, len);
	tmp[len] = 0;

	nk_cairo_font_t *cfont = handle.ptr;

	cairo_set_font_face(nk_cairo, nk_cairo_faces[cfont->face].ca);
	cairo_set_font_size(nk_cairo, cfont->size);

	cairo_text_extents_t extents;
	cairo_text_extents(nk_cairo, tmp, &extents);
	free(tmp);

	return NK_MAX(extents.width + extents.x_bearing, extents.x_advance);
}

NK_API struct nk_user_font* nk_cairo_ttf(const char *ttf, int ttf_size)
{
	int i, face = -1, font = -1;

	/* is the font face is already loaded? */
	for (i = 0; i < nk_cairo_face; i++) {
		if (strcmp(ttf, nk_cairo_faces[i].ttf) == 0) {
			face = i;
			break;
		}
	}

	if (face < 0) {
		face = nk_cairo_face++;
		assert(face < MAXFONTS);
		nk_cairo_faces[face].ttf = ttf;
		assert(0 == FT_New_Face(nk_cairo_ft, ttf, 0, &nk_cairo_faces[face].ft));
		nk_cairo_faces[face].ca = cairo_ft_font_face_create_for_ft_face(nk_cairo_faces[face].ft, 0);
	}

	/* is the font size is already created? */
	for (i = 0; i < nk_cairo_font; i++) {
		if (nk_cairo_fonts[i].face == face && nk_cairo_fonts[i].size == ttf_size) {
			font = i;
			break;
		}
	}

	if (font < 0) {
		font = nk_cairo_font++;
		assert(font < MAXFONTS);

		cairo_set_font_face(nk_cairo, nk_cairo_faces[face].ca);
		cairo_set_font_size(nk_cairo, ttf_size);

		cairo_font_extents_t extents;
		cairo_font_extents(nk_cairo, &extents);

		nk_cairo_fonts[font].face = face;
		nk_cairo_fonts[font].size = ttf_size;
		nk_cairo_fonts[font].font.userdata.ptr = &nk_cairo_fonts[font];
		nk_cairo_fonts[font].font.height = extents.height;
		nk_cairo_fonts[font].font.width = nk_cairo_text_width;
	}

	return &nk_cairo_fonts[font].font;
}

NK_API void
nk_cairo_init()
{
	assert(0 == FT_Init_FreeType(&nk_cairo_ft));
	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
	nk_cairo = cairo_create(surface);
	cairo_surface_destroy(surface);
}

NK_API void
nk_cairo_render(struct nk_context *ctx, void *fb, int w, int h, int pitch)
{
	cairo_surface_t *surface = cairo_image_surface_create_for_data(fb, CAIRO_FORMAT_ARGB32, w, h, pitch);
	cairo_t *cairo = cairo_create(surface);

	nk_cairo_scissor(cairo, 0, 0, w, h);

	const struct nk_command *cmd;

	nk_foreach(cmd, ctx) {
		switch (cmd->type) {
			case NK_COMMAND_NOP: break;
			case NK_COMMAND_SCISSOR: {
				struct nk_command_scissor *s =(struct nk_command_scissor*)cmd;
				nk_cairo_scissor(cairo, s->x, s->y, s->w, s->h);
				break;
			}
			case NK_COMMAND_LINE: {
				struct nk_command_line *l = (struct nk_command_line *)cmd;
				nk_cairo_stroke_line(cairo, l->begin.x, l->begin.y, l->end.x, l->end.y, l->line_thickness, l->color);
				break;
			}
			case NK_COMMAND_RECT: {
				struct nk_command_rect *r = (struct nk_command_rect *)cmd;
				nk_cairo_stroke_rect(cairo, r->x, r->y, r->w, r->h,
					(int)r->rounding, r->line_thickness, r->color);
				break;
			}
			case NK_COMMAND_RECT_FILLED: {
				struct nk_command_rect_filled *r = (struct nk_command_rect_filled *)cmd;
				nk_cairo_fill_rect(cairo, r->x, r->y, r->w, r->h, (int)r->rounding, r->color);
				break;
			}
			case NK_COMMAND_CIRCLE: {
				struct nk_command_circle *c = (struct nk_command_circle *)cmd;
				int r = (c->w < c->h ? c->w: c->h)/2;
				nk_cairo_stroke_circle(cairo, c->x+r, c->y+r, r, c->line_thickness, c->color);
				break;
			}
			case NK_COMMAND_CIRCLE_FILLED: {
				struct nk_command_circle_filled *c = (struct nk_command_circle_filled *)cmd;
				int r = (c->w < c->h ? c->w: c->h)/2;
				nk_cairo_fill_circle(cairo, c->x+r, c->y+r, r, c->color);
				break;
			}
			case NK_COMMAND_TRIANGLE: {
				struct nk_command_triangle*t = (struct nk_command_triangle*)cmd;
				nk_cairo_stroke_triangle(cairo, t->a.x, t->a.y, t->b.x, t->b.y, t->c.x, t->c.y, t->line_thickness, t->color);
				break;
			}
			case NK_COMMAND_TRIANGLE_FILLED: {
				struct nk_command_triangle_filled *t = (struct nk_command_triangle_filled *)cmd;
				nk_cairo_fill_triangle(cairo, t->a.x, t->a.y, t->b.x, t->b.y, t->c.x, t->c.y, t->color);
				break;
			}
			case NK_COMMAND_POLYGON: {
				struct nk_command_polygon *p =(struct nk_command_polygon*)cmd;
				nk_cairo_stroke_polygon(cairo, p->points, p->point_count, p->line_thickness, p->color);
				break;
			}
			case NK_COMMAND_POLYGON_FILLED: {
				struct nk_command_polygon_filled *p = (struct nk_command_polygon_filled *)cmd;
				nk_cairo_fill_polygon(cairo, p->points, p->point_count, p->color);
				break;
			}
			case NK_COMMAND_POLYLINE: {
				struct nk_command_polyline *p = (struct nk_command_polyline *)cmd;
				nk_cairo_stroke_polyline(cairo, p->points, p->point_count, p->line_thickness, p->color);
				break;
			}
			case NK_COMMAND_TEXT: {
				struct nk_command_text *t = (struct nk_command_text*)cmd;
				nk_cairo_draw_text(cairo, (struct nk_user_font*)t->font, nk_rect(t->x, t->y, t->w, t->h), t->string, t->length, t->height, t->foreground);
				break;
			}
			case NK_COMMAND_CURVE: {
				fprintf(stderr, "NK_COMMAND_CURVE not implemented\n");
				break;
			}
			case NK_COMMAND_RECT_MULTI_COLOR: {
				fprintf(stderr, "NK_COMMAND_RECT_MULTI_COLOR not implemented\n");
				break;
			}
			case NK_COMMAND_IMAGE: {
				struct nk_command_image *q = (struct nk_command_image *)cmd;
				nk_cairo_drawimage(cairo, q->x, q->y, q->w, q->h, &q->img, &q->col);
				break;
			}
			case NK_COMMAND_ARC: {
				fprintf(stderr, "NK_COMMAND_ARC\n");
				struct nk_command_arc *q = (struct nk_command_arc *)cmd;
				nk_cairo_stroke_arc(cairo, q->cx, q->cy, q->r, q->a[0], q->a[1], q->line_thickness, q->color);
				break;
			}
			case NK_COMMAND_ARC_FILLED: {
				fprintf(stderr, "NK_COMMAND_ARC_FILLED\n");
				struct nk_command_arc_filled *q = (struct nk_command_arc_filled *)cmd;
				nk_cairo_fill_arc(cairo, q->cx, q->cy, q->r, q->a[0], q->a[1], q->color);
				break;
			}
			default: break;
		}
	}

	cairo_surface_flush(surface);
	cairo_surface_destroy(surface);
	cairo_destroy(cairo);
	nk_clear(ctx);
}
#endif
