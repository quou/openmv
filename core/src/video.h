#pragma once

#include "common.h"
#include "maths.h"

/* Basic renderer; Graphics API abstraction. */

API void video_init();
API void video_clear();

struct shader {
	bool panic;

	u32 id;
};

API void init_shader(struct shader* shader, const char* source, const char* name);
API void deinit_shader(struct shader* shader);
API void bind_shader(const struct shader* shader);
API void shader_set_f(const struct shader* shader, const char* name, const float v);
API void shader_set_i(const struct shader* shader, const char* name, const i32 v);
API void shader_set_u(const struct shader* shader, const char* name, const u32 v);
API void shader_set_b(const struct shader* shader, const char* name, const bool v);
API void shader_set_v2f(const struct shader* shader, const char* name, const v2f v);
API void shader_set_v3f(const struct shader* shader, const char* name, const v3f v);
API void shader_set_v4f(const struct shader* shader, const char* name, const v4f v);
API void shader_set_m3f(const struct shader* shader, const char* name, const m3f v);
API void shader_set_m4f(const struct shader* shader, const char* name, const m4f v);

enum {
	VB_STATIC     = 1 << 0,
	VB_DYNAMIC    = 1 << 1,
	VB_LINES      = 1 << 2,
	VB_LINE_STRIP = 1 << 3,
	VB_TRIS       = 1 << 4
};

struct vertex_buffer {
	u32 va_id;
	u32 vb_id;
	u32 ib_id;
	u32 index_count;

	i32 flags;
};

API void init_vb(struct vertex_buffer* vb, const i32 flags);
API void deinit_vb(struct vertex_buffer* vb);
API void bind_vb_for_draw(const struct vertex_buffer* vb);
API void bind_vb_for_edit(const struct vertex_buffer* vb);
API void push_vertices(const struct vertex_buffer* vb, float* vertices, u32 count);
API void push_indices(struct vertex_buffer* vb, u32* indices, u32 count);
API void update_vertices(const struct vertex_buffer* vb, float* vertices, u32 offset, u32 count);
API void update_indices(struct vertex_buffer* vb, u32* indices, u32 offset, u32 count);
API void configure_vb(const struct vertex_buffer* vb, u32 index, u32 component_count,
 	u32 stride, u32 offset);
API void draw_vb(const struct vertex_buffer* vb);
API void draw_vb_n(const struct vertex_buffer* vb, u32 count);
 
struct texture {
	u32 id;
	u32 width, height;
};

API void init_texture(struct texture* texture, u8* src, u32 size);
API void init_texture_no_bmp(struct texture* texture, u8* src, u32 w, u32 h, bool flip);
API void deinit_texture(struct texture* texture);
API void bind_texture(const struct texture* texture, u32 unit);

struct rect {
	i32 x, y, w, h;
};

API struct rect make_rect(i32 x, i32 y, i32 w, i32 h);

struct color {
	u8 r, g, b, a;
};

API struct color make_color(u32 rgb, u8 alpha);

struct textured_quad {
	struct texture* texture;
	v2i position;
	v2i dimentions;
	struct rect rect;
	struct color color;

	float rotation;
	v2f origin;
};

struct renderer {
	struct shader shader;
	struct vertex_buffer vb;

	u32 quad_count;

	struct texture* textures[32];
	u32 texture_count;

	bool clip_enable;
	bool camera_enable;

	struct rect clip;
	v2i dimentions;

	v2i camera_pos;
	m4f camera;

	m4f transforms[100];
	u32 transform_count;
};

API struct renderer* new_renderer(struct shader shader, v2i dimentions);
API void free_renderer(struct renderer* renderer);
API void renderer_flush(struct renderer* renderer);
API void renderer_push(struct renderer* renderer, struct textured_quad* quad);
API void renderer_clip(struct renderer* renderer, struct rect clip);

struct font;

API i32 render_text(struct renderer* renderer, struct font* font,
		const char* text, i32 x, i32 y, struct color color);

API struct font* load_font_from_memory(void* data, i32 filesize, float size);
API void free_font(struct font* font);

API void set_font_tab_size(struct font* font, i32 n);
API i32 get_font_tab_size(struct font* font);

API float get_font_size(struct font* font);

API i32 text_width(struct font* font, const char* text);
API i32 text_height(struct font* font);
