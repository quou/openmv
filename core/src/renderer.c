#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "video.h"
#include "res.h"
#include "util/stb_rect_pack.h"
#include "util/stb_truetype.h"
#include "util/gl.h"

#define batch_size 100
#define els_per_vert 10
#define verts_per_quad 4
#define indices_per_quad 6

#define MAX_GLYPHSET 256

struct color make_color(u32 rgb, u8 alpha) {
	return (struct color) { (rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xff, alpha };
}

struct rect make_rect(i32 x, i32 y, i32 w, i32 h) {
	return (struct rect) { x, y, w, h };
}

struct renderer* new_renderer(const struct shader* shader, v2i dimentions) {
	struct renderer* renderer = calloc(1, sizeof(struct renderer));

	renderer->quad_count = 0;
	renderer->texture_count = 0;
	renderer->transform_count = 0;

	init_vb(&renderer->vb, VB_DYNAMIC | VB_TRIS);
	bind_vb_for_edit(&renderer->vb);
	push_vertices(&renderer->vb, null, els_per_vert * verts_per_quad * batch_size);
	push_indices(&renderer->vb, null, indices_per_quad * batch_size);
	configure_vb(&renderer->vb, 0, 2, els_per_vert, 0); /* vec2 position */
	configure_vb(&renderer->vb, 1, 2, els_per_vert, 2); /* vec2 uv */
	configure_vb(&renderer->vb, 2, 4, els_per_vert, 4); /* vec4 color */
	configure_vb(&renderer->vb, 3, 1, els_per_vert, 8); /* float texture_id */
	configure_vb(&renderer->vb, 4, 1, els_per_vert, 9); /* float transform_id */
	bind_vb_for_edit(null);

	renderer->clip_enable = false;
	renderer->camera_enable = false;

	renderer->shader = shader;
	bind_shader(renderer->shader);

	renderer->camera = m4f_orth(0.0f, (float)dimentions.x, (float)dimentions.y, 0.0f, -1.0f, 1.0f);
	shader_set_m4f(renderer->shader, "camera", renderer->camera);
	renderer->dimentions = dimentions;

	bind_shader(null);

	return renderer;
}

void free_renderer(struct renderer* renderer) {
	deinit_vb(&renderer->vb);

	free(renderer);
}

void renderer_flush(struct renderer* renderer) {
	if (renderer->quad_count == 0) { return; }

	if (renderer->clip_enable) {
		glEnable(GL_SCISSOR_TEST);
		glScissor(renderer->clip.x, renderer->dimentions.y - (renderer->clip.y + renderer->clip.h),
				renderer->clip.w, renderer->clip.h);
	} else {
		glDisable(GL_SCISSOR_TEST);
	}

	bind_shader(renderer->shader);

	for (u32 i = 0; i < renderer->texture_count; i++) {
		bind_texture(renderer->textures[i], i);

		char name[32];
		sprintf(name, "textures[%u]", i);

		shader_set_i(renderer->shader, name, i);
	}

	for (u32 i = 0; i < renderer->transform_count; i++) {
		char name[32];
		sprintf(name, "transforms[%u]", i);

		shader_set_m4f(renderer->shader, name, renderer->transforms[i]);
	}

	shader_set_m4f(renderer->shader, "camera", renderer->camera);

	if (renderer->camera_enable) {
		m4f view = m4f_translate(m4f_identity(), make_v3f(
			-(renderer->camera_pos.x) + (renderer->dimentions.x / 2),
			-(renderer->camera_pos.y) + (renderer->dimentions.y / 2),
			0.0f));
		shader_set_m4f(renderer->shader, "view", view);
	} else {
		shader_set_m4f(renderer->shader, "view", m4f_identity());
	}

	bind_vb_for_draw(&renderer->vb);
	draw_vb_n(&renderer->vb, renderer->quad_count * indices_per_quad);
	bind_vb_for_draw(null);
	bind_shader(null);

	renderer->quad_count = 0;
	renderer->texture_count = 0;
	renderer->transform_count = 0;
}

void renderer_push(struct renderer* renderer, struct textured_quad* quad) {
	bind_vb_for_edit(&renderer->vb);
	
	float tx = 0, ty = 0, tw = 0, th = 0;

	i32 tidx = -1;
	if (quad->texture) {
		for (u32 i = 0; i < renderer->texture_count; i++) {
			if (renderer->textures[i] == quad->texture) {
				tidx = (i32)i;
			}
		}

		if (tidx == -1) {
			renderer->textures[renderer->texture_count] = quad->texture;
			tidx = renderer->texture_count;

			renderer->texture_count++;

			if (renderer->texture_count >= 32) {
				renderer_flush(renderer);
				tidx = 0;
				renderer->textures[0] = quad->texture;
			}
		}

		tx = (float)quad->rect.x / (float)quad->texture->width;
		ty = (float)quad->rect.y / (float)quad->texture->height;
		tw = (float)quad->rect.w / (float)quad->texture->width;
		th = (float)quad->rect.h / (float)quad->texture->height;
	}

	const float r = (float)quad->color.r / 255.0f;
	const float g = (float)quad->color.g / 255.0f;
	const float b = (float)quad->color.b / 255.0f;
	const float a = (float)quad->color.a / 255.0f;

	m4f transform = m4f_translate(m4f_identity(), make_v3f(quad->position.x, quad->position.y, 0.0f));

	transform = m4f_translate(transform, make_v3f(quad->origin.x, quad->origin.y, 0.0f));
	transform = m4f_rotate(transform, (float)torad(quad->rotation), make_v3f(0.0f, 0.0f, 1.0f));
	transform = m4f_translate(transform, make_v3f(-quad->origin.x, -quad->origin.y, 0.0f));

	transform = m4f_scale(transform, make_v3f(quad->dimentions.x, quad->dimentions.y, 0.0f));

	float verts[] = {
		0.0f, 0.0f, tx, ty,           r, g, b, a, (float)tidx, (float)renderer->transform_count,
		1.0f, 0.0f, tx + tw, ty,      r, g, b, a, (float)tidx, (float)renderer->transform_count,
		1.0f, 1.0f, tx + tw, ty + th, r, g, b, a, (float)tidx, (float)renderer->transform_count,
		0.0f, 1.0f, tx, ty + th,      r, g, b, a, (float)tidx, (float)renderer->transform_count
	};

	renderer->transforms[renderer->transform_count++] = transform;

	const u32 idx_off = renderer->quad_count * verts_per_quad;

	u32 indices[] = {
		idx_off + 3, idx_off + 2, idx_off + 1,
		idx_off + 3, idx_off + 1, idx_off + 0
	};

	update_vertices(&renderer->vb, verts, renderer->quad_count * els_per_vert * verts_per_quad, els_per_vert * verts_per_quad);
	update_indices(&renderer->vb, indices, renderer->quad_count * indices_per_quad, indices_per_quad);

	bind_vb_for_edit(null);

	renderer->quad_count++;

	if (renderer->quad_count >= batch_size) {
		renderer_flush(renderer);
	}
}

void renderer_clip(struct renderer* renderer, struct rect clip) {
	if (renderer->clip.x != clip.x ||
		renderer->clip.y != clip.y ||
		renderer->clip.w != clip.w ||
		renderer->clip.h != clip.h) {
		renderer_flush(renderer);
		renderer->clip = clip;
	}
}

struct glyph_set {
	struct texture atlas;
	stbtt_bakedchar glyphs[256];
};

struct font {
	void* data;
	stbtt_fontinfo info;
	struct glyph_set* sets[MAX_GLYPHSET];
	float size;
	i32 height;
};

static const char* utf8_to_codepoint(const char* p, u32* dst) {
	u32 res, n;
	switch (*p & 0xf0) {
		case 0xf0 : res = *p & 0x07; n = 3; break;
		case 0xe0 : res = *p & 0x0f; n = 2; break;
		case 0xd0 :
		case 0xc0 : res = *p & 0x1f; n = 1; break;
		default   : res = *p;        n = 0; break;
	}
	while (n--) {
		res = (res << 6) | (*(++p) & 0x3f);
	}
	*dst = res;
	return p + 1;
}

static struct glyph_set* load_glyph_set(struct font* font, i32 idx) {
	i32 width, height, r, ascent, descent, linegap, scaled_ascent, i;
	unsigned char n;
	float scale, s;
	struct glyph_set* set;

	set = calloc(1, sizeof(struct glyph_set));

	width = 128;
	height = 128;

	struct color* pixels;

retry:
	pixels = malloc(width * height * 4);

	s = stbtt_ScaleForMappingEmToPixels(&font->info, 1) /
		stbtt_ScaleForPixelHeight(&font->info, 1);
	r = stbtt_BakeFontBitmap(font->data, 0, font->size * s,
			(void*)pixels, width, height, idx * 256, 256, set->glyphs);

	if (r <= 0) {
		width *= 2;
		height *= 2;
		free(pixels);
		goto retry;
	}

	stbtt_GetFontVMetrics(&font->info, &ascent, &descent, &linegap);
	scale = stbtt_ScaleForMappingEmToPixels(&font->info, font->size);
	scaled_ascent = (i32)(ascent * scale + 0.5);
	for (i = 0; i < 256; i++) {
		set->glyphs[i].yoff += scaled_ascent;
		set->glyphs[i].xadvance = (float)floor(set->glyphs[i].xadvance);
	}

	for (i = width * height - 1; i >= 0; i--) {
		n = *((u8*)pixels + i);
		pixels[i] = (struct color) {255, 255, 255, n};
	}

	init_texture_no_bmp(&set->atlas, (u8*)pixels, width, height, false);

	free(pixels);

	return set;
}

static struct glyph_set* get_glyph_set(struct font* font, i32 code_poi32) {
	i32 idx;
	
	idx = (code_poi32 >> 8) % MAX_GLYPHSET;
	if (!font->sets[idx]) {
		font->sets[idx] = load_glyph_set(font, idx);
	}
	return font->sets[idx];
}

struct font* load_font_from_memory(void* data, i32 filesize, float size) {
	struct font* font;
	i32 r, ascent, descent, linegap;
	float scale;

	font = calloc(1, sizeof(struct font));
	font->data = data;

	font->size = size;

	r = stbtt_InitFont(&font->info, font->data, 0);
	if (!r) {
		goto fail;
	}

	stbtt_GetFontVMetrics(&font->info, &ascent, &descent, &linegap);
	scale = stbtt_ScaleForMappingEmToPixels(&font->info, size);
	font->height = (i32)((ascent - descent + linegap) * scale + 0.5);

	stbtt_bakedchar* g = get_glyph_set(font, '\n')->glyphs;
	g['\t'].x1 = g['\t'].x0;
	g['\n'].x1 = g['\n'].x0;

	set_font_tab_size(font, 8);

	return font;

fail:
	if (font->data) { free(font->data); }
	if (font) { free(font); }
	return NULL;
}

void free_font(struct font* font) {
	i32 i;
	struct glyph_set* set;

	for (i = 0; i < MAX_GLYPHSET; i++) {
		set = font->sets[i];
		if (set) {
			deinit_texture(&set->atlas);
			free(set);
		}
	}

	free(font->data);
	free(font);
}

void set_font_tab_size(struct font* font, i32 n) {
	struct glyph_set* set;

	set = get_glyph_set(font, '\t');
	set->glyphs['\t'].xadvance = n * set->glyphs[' '].xadvance;
}

i32 get_font_tab_size(struct font* font) {
	struct glyph_set* set;

	set = get_glyph_set(font, '\t');
	return (i32)(set->glyphs['\t'].xadvance / set->glyphs[' '].xadvance);
}

i32 text_width(struct font* font, const char* text) {
	i32 x;
	u32 codepoint;
	const char* p;
	struct glyph_set* set;
	stbtt_bakedchar* g;
	
	x = 0;
	p = text;
	while (*p) {
		p = utf8_to_codepoint(p, &codepoint);
		set = get_glyph_set(font, codepoint);
		g = &set->glyphs[codepoint & 0xff];
		x += (i32)g->xadvance;
	}
	return x;
}

i32 text_height(struct font* font) {
	return font->height;
}

i32 render_text(struct renderer* renderer, struct font* font,
		const char* text, i32 x, i32 y, struct color color) {
	const char* p;
	u32 codepoint;
	struct glyph_set* set;
	stbtt_bakedchar* g;

	p = text;
	while (*p) {
		p = utf8_to_codepoint(p, &codepoint);

		set = get_glyph_set(font, codepoint);
		g = &set->glyphs[codepoint & 0xff];

		i32 w = g->x1 - g->x0;
		i32 h = g->y1 - g->y0;
		
		struct textured_quad quad = {
			.texture = &set->atlas,
			.position = { x + g->xoff, y + g->yoff },
			.dimentions = { w, h },
			.rect = { g->x0, g->y0, w, h },
			.color = color
		};

		renderer_push(renderer, &quad);
		x += (i32)g->xadvance;
	}

	return x;
}
