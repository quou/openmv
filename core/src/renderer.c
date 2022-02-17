#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "core.h"
#include "res.h"
#include "util/glad.h"
#include "util/stb_rect_pack.h"
#include "util/stb_truetype.h"
#include "video.h"

#define batch_size 100
#define els_per_vert 12
#define verts_per_quad 4
#define indices_per_quad 6

#define MAX_GLYPHSET 256

struct color make_color(u32 rgb, u8 alpha) {
	return (struct color) { (rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xff, alpha };
}

struct rect make_rect(i32 x, i32 y, i32 w, i32 h) {
	return (struct rect) { x, y, w, h };
}

struct renderer* new_renderer(struct shader shader, v2i dimentions) {
	struct renderer* renderer = core_calloc(1, sizeof(struct renderer));

	renderer->quad_count = 0;
	renderer->texture_count = 0;

	renderer->ambient_light = 1.0f;

	init_vb(&renderer->vb, VB_DYNAMIC | VB_TRIS);
	bind_vb_for_edit(&renderer->vb);
	push_vertices(&renderer->vb, null, els_per_vert * verts_per_quad * batch_size);
	push_indices(&renderer->vb, null, indices_per_quad * batch_size);
	configure_vb(&renderer->vb, 0, 2, els_per_vert, 0);  /* vec2 position */
	configure_vb(&renderer->vb, 1, 2, els_per_vert, 2);  /* vec2 uv */
	configure_vb(&renderer->vb, 2, 4, els_per_vert, 4);  /* vec4 color */
	configure_vb(&renderer->vb, 3, 1, els_per_vert, 8);  /* float texture_id */
	configure_vb(&renderer->vb, 4, 1, els_per_vert, 9);  /* float inverted */
	configure_vb(&renderer->vb, 5, 1, els_per_vert, 10); /* float unlit */
	configure_vb(&renderer->vb, 6, 1, els_per_vert, 11); /* float trans_id */
	bind_vb_for_edit(null);

	renderer->clip_enable = false;
	renderer->camera_enable = false;

	renderer->shader = shader;
	bind_shader(&renderer->shader);

	renderer->camera = m4f_orth(0.0f, (float)dimentions.x, (float)dimentions.y, 0.0f, -1.0f, 1.0f);
	shader_set_m4f(&renderer->shader, "camera", renderer->camera);
	renderer->dimentions = dimentions;

	bind_shader(null);

	return renderer;
}

void free_renderer(struct renderer* renderer) {
	deinit_vb(&renderer->vb);

	core_free(renderer);
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

	bind_shader(&renderer->shader);

	for (u32 i = 0; i < renderer->texture_count; i++) {
		bind_texture(renderer->textures[i], i);

		char name[32];
		sprintf(name, "textures[%u]", i);

		shader_set_i(&renderer->shader, name, i);
	}

	for (u32 i = 0; i < renderer->transform_count; i++) {
		char name[32];
		sprintf(name, "transforms[%u]", i);

		shader_set_m4f(&renderer->shader, name, renderer->transforms[i]);
	}

	for (u32 i = 0; i < renderer->light_count; i++) {
		char name[64];

		sprintf(name, "lights[%u].position", i);
		shader_set_v2f(&renderer->shader, name, renderer->lights[i].position);

		sprintf(name, "lights[%u].intensity", i);
		shader_set_f(&renderer->shader, name, renderer->lights[i].intensity);

		sprintf(name, "lights[%u].range", i);
		shader_set_f(&renderer->shader, name, renderer->lights[i].range);
	}

	shader_set_i(&renderer->shader, "light_count", renderer->light_count);

	shader_set_m4f(&renderer->shader, "camera", renderer->camera);
	shader_set_f(&renderer->shader, "ambient_light", renderer->ambient_light);

	if (renderer->camera_enable) {
		m4f view = m4f_translate(m4f_identity(), make_v3f(
			-((float)renderer->camera_pos.x) + ((float)renderer->dimentions.x / 2),
			-((float)renderer->camera_pos.y) + ((float)renderer->dimentions.y / 2),
			0.0f));
		shader_set_m4f(&renderer->shader, "view", view);
	} else {
		shader_set_m4f(&renderer->shader, "view", m4f_identity());
	}

	bind_vb_for_draw(&renderer->vb);
	draw_vb_n(&renderer->vb, renderer->quad_count * indices_per_quad);
	bind_vb_for_draw(null);
	bind_shader(null);

	renderer->quad_count = 0;
	renderer->texture_count = 0;
	renderer->transform_count = 0;
}

void renderer_end_frame(struct renderer* renderer) {
	renderer_flush(renderer);
	renderer->light_count = 0;
}

void renderer_push_light(struct renderer* renderer, struct light light) {
	if (renderer->light_count > max_lights) {
		fprintf(stderr, "Too many lights! Max: %d\n", max_lights);
		return;
	}

	renderer->lights[renderer->light_count++] = light;
}

void renderer_push(struct renderer* renderer, struct textured_quad* quad) {
	bind_vb_for_edit(&renderer->vb);
	
	float tx = 0, ty = 0, tw = 0, th = 0;

	i32 tidx = -1;
	if (quad->texture) {
		for (u32 i = 0; i < renderer->texture_count; i++) {
			if (renderer->textures[i] == quad->texture) {
				tidx = (i32)i;
				break;
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

		tx = (float)quad->rect.x/ (float)quad->texture->width;
		ty = (float)quad->rect.y/ (float)quad->texture->height;
		tw = (float)quad->rect.w/ (float)quad->texture->width;
		th = (float)quad->rect.h/ (float)quad->texture->height;
	}

	const float r = (float)quad->color.r / 255.0f;
	const float g = (float)quad->color.g / 255.0f;
	const float b = (float)quad->color.b / 255.0f;
	const float a = (float)quad->color.a / 255.0f;

	const float w = (float)quad->dimentions.x;
	const float h = (float)quad->dimentions.y;
	const float x = (float)quad->position.x;
	const float y = (float)quad->position.y;

	/* I thought this was causing the lack of performance, but removing it had no effect. */
	m4f transform = m4f_translate(m4f_identity(), make_v3f((float)quad->position.x, (float)quad->position.y, 0.0f));
	transform = m4f_translate(transform, make_v3f((float)quad->origin.x, (float)quad->origin.y, 0.0f));
	transform = m4f_rotate(transform, (float)torad((float)quad->rotation), make_v3f(0.0f, 0.0f, 1.0f));
	transform = m4f_scale(transform, make_v3f((float)quad->dimentions.x, (float)quad->dimentions.y, 0.0f));
	transform = m4f_translate(transform, make_v3f((float)-quad->origin.x, (float)-quad->origin.y, 0.0f));

	float trans_id = (float)renderer->transform_count;

	float verts[] = {
		0.0f, 0.0f, tx, ty,           r, g, b, a, (float)tidx, (float)quad->inverted, (float)quad->unlit, trans_id,
		1.0f, 0.0f, tx + tw, ty,      r, g, b, a, (float)tidx, (float)quad->inverted, (float)quad->unlit, trans_id,
		1.0f, 1.0f, tx + tw, ty + th, r, g, b, a, (float)tidx, (float)quad->inverted, (float)quad->unlit, trans_id,
		0.0f, 1.0f, tx, ty + th,      r, g, b, a, (float)tidx, (float)quad->inverted, (float)quad->unlit, trans_id
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

	set = core_calloc(1, sizeof(struct glyph_set));

	width = 128;
	height = 128;

	struct color* pixels;

retry:
	pixels = core_alloc(width * height * 4);

	s = stbtt_ScaleForMappingEmToPixels(&font->info, 1) /
		stbtt_ScaleForPixelHeight(&font->info, 1);
	r = stbtt_BakeFontBitmap(font->data, 0, font->size * s,
			(void*)pixels, width, height, idx * 256, 256, set->glyphs);

	if (r <= 0) {
		width *= 2;
		height *= 2;
		core_free(pixels);
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

	core_free(pixels);

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

struct font* load_font_from_memory(void* data, u64 filesize, float size) {
	struct font* font;
	i32 r, ascent, descent, linegap;
	float scale;

	font = core_calloc(1, sizeof(struct font));
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
	if (font) {
		if (font->data) {
			core_free(font->data);
		}
		core_free(font);
	}
	return null;
}

void free_font(struct font* font) {
	i32 i;
	struct glyph_set* set;

	for (i = 0; i < MAX_GLYPHSET; i++) {
		set = font->sets[i];
		if (set) {
			deinit_texture(&set->atlas);
			core_free(set);
		}
	}

	core_free(font->data);
	core_free(font);
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

float get_font_size(struct font* font) {
	return font->size;
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
			.position = { x + (i32)g->xoff, y + (i32)g->yoff },
			.dimentions = { w, h },
			.rect = { g->x0, g->y0, w, h },
			.color = color,
			.unlit = true
		};

		renderer_push(renderer, &quad);
		x += (i32)g->xadvance;
	}

	return x;
}

i32 render_text_n(struct renderer* renderer, struct font* font,
		const char* text, u32 n, i32 x, i32 y, struct color color) {
	const char* p;
	u32 codepoint;
	struct glyph_set* set;
	stbtt_bakedchar* g;

	p = text;
	for (u32 i = 0; i < n && *p; i++) {
		p = utf8_to_codepoint(p, &codepoint);

		set = get_glyph_set(font, codepoint);
		g = &set->glyphs[codepoint & 0xff];

		i32 w = g->x1 - g->x0;
		i32 h = g->y1 - g->y0;
		
		struct textured_quad quad = {
			.texture = &set->atlas,
			.position = { x + (i32)g->xoff, y + (i32)g->yoff },
			.dimentions = { w, h },
			.rect = { g->x0, g->y0, w, h },
			.color = color,
			.unlit = true
		};

		renderer_push(renderer, &quad);
		x += (i32)g->xadvance;
	}

	return x;
}

i32 render_text_fancy(struct renderer* renderer, struct font* font,
		const char* text, u32 n, i32 x, i32 y, struct color color,
		struct textured_quad* coin) {
	const char* p;
	u32 codepoint;
	struct glyph_set* set;
	stbtt_bakedchar* g;

	p = text;
	for (u32 i = 0; i < n && *p; i++) {
		if (*p == '%' && *(p + 1) == 'c') {
			p += 2;

			coin->position.x = x;
			coin->position.y = y;
			coin->color = color;
			renderer_push(renderer, coin);

			x += coin->dimentions.x;
		} else {
			p = utf8_to_codepoint(p, &codepoint);

			set = get_glyph_set(font, codepoint);
			g = &set->glyphs[codepoint & 0xff];

			i32 w = g->x1 - g->x0;
			i32 h = g->y1 - g->y0;
			
			struct textured_quad quad = {
				.texture = &set->atlas,
				.position = { x + (i32)g->xoff, y + (i32)g->yoff },
				.dimentions = { w, h },
				.rect = { g->x0, g->y0, w, h },
				.color = color
			};

			renderer_push(renderer, &quad);
			x += (i32)g->xadvance;
		}
	}

	return x;
}
