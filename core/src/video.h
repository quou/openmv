#pragma once

#include "common.h"
#include "maths.h"

#define max_lights 100

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
API void shader_set_f(const struct shader* shader, const char* name, const f32 v);
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
API void push_vertices(const struct vertex_buffer* vb, f32* vertices, u32 count);
API void push_indices(struct vertex_buffer* vb, u32* indices, u32 count);
API void update_vertices(const struct vertex_buffer* vb, f32* vertices, u32 offset, u32 count);
API void update_indices(struct vertex_buffer* vb, u32* indices, u32 offset, u32 count);
API void configure_vb(const struct vertex_buffer* vb, u32 index, u32 component_count,
 	u32 stride, u32 offset);
API void draw_vb(const struct vertex_buffer* vb);
API void draw_vb_n(const struct vertex_buffer* vb, u32 count);
 
struct texture {
	u32 id;
	u32 width, height;
};

API void init_texture(struct texture* texture, u8* src, u64 size);
API void init_texture_no_bmp(struct texture* texture, u8* src, u32 w, u32 h, bool flip);
API void update_texture(struct texture* texture, u8* data, u64 size);
API void update_texture_no_bmp(struct texture* texture, u8* src, u32 w, u32 h, bool flip);
API void deinit_texture(struct texture* texture);
API void bind_texture(const struct texture* texture, u32 unit);

struct render_target {
	u32 id;
	u32 width, height;

	u32 output;
};

API void init_render_target(struct render_target* target, u32 width, u32 height);
API void deinit_render_target(struct render_target* target);
API void resize_render_target(struct render_target* target, u32 width, u32 height);
API void bind_render_target(struct render_target* target);
API void bind_render_target_output(struct render_target* target, u32 unit);

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

	bool inverted;
	bool unlit;

	v2f origin;
	f32 rotation;
};

struct light {
	v2f position;
	f32 range;
	f32 intensity;
};

/* For some reason the renderer is really slow, and I do not know
 * why. It *should* be fast; It doesn't cause too many draw calls
 * and it isn't doing too much maths. At first I thought the matrix
 * transformation was slowing it down, but removing rotations had
 * zero effect on ther performance.
 *
 * Fix this, somehow. */
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

	v2f light_pos;

	f32 ambient_light;

	struct light lights[max_lights];
	u32 light_count;

	m4f transforms[100];
	u32 transform_count;
};

API struct renderer* new_renderer(struct shader shader, v2i dimentions);
API void free_renderer(struct renderer* renderer);
API void renderer_flush(struct renderer* renderer);
API void renderer_end_frame(struct renderer* renderer);
API void renderer_push(struct renderer* renderer, struct textured_quad* quad);
API void renderer_push_light(struct renderer* renderer, struct light light);
API void renderer_clip(struct renderer* renderer, struct rect clip);
API void renderer_resize(struct renderer* renderer, v2i size);
API void renderer_fit_to_main_window(struct renderer* renderer);

struct post_processor {
	struct render_target target;

	struct shader shader;
	struct vertex_buffer vb;

	v2i dimentions;
};

API struct post_processor* new_post_processor(struct shader shader);
API void free_post_processor(struct post_processor* p);
API void use_post_processor(struct post_processor* p);
API void resize_post_processor(struct post_processor* p, v2i dimentions);
API void post_processor_fit_to_main_window(struct post_processor* p);
API void flush_post_processor(struct post_processor* p, bool default_rt);

struct font;

API i32 render_text(struct renderer* renderer, struct font* font,
		const char* text, i32 x, i32 y, struct color color);

/* Draw text with a specified number of characters. */
API i32 render_text_n(struct renderer* renderer, struct font* font,
		const char* text, u32 n, i32 x, i32 y, struct color color);

/* This function draws text with icons. There is only one icon
 * at the moment, that being a coin. This shouldn't really be in
 * the core, rather it should be in the logic since it is specific
 * to the game... */
API i32 render_text_fancy(struct renderer* renderer, struct font* font,
		const char* text, u32 n, i32 x, i32 y, struct color color,
		struct textured_quad* coin);

API struct font* load_font_from_memory(void* data, u64 filesize, f32 size);
API void free_font(struct font* font);

API void set_font_tab_size(struct font* font, i32 n);
API i32 get_font_tab_size(struct font* font);

API f32 get_font_size(struct font* font);

API i32 font_height(struct font* font);

API i32 text_width(struct font* font, const char* text);
API i32 text_height(struct font* font, const char* text);
API i32 text_width_n(struct font* font, const char* text, u32 n);
API i32 text_height_n(struct font* font, const char* text, u32 n);
