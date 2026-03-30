#include "render.h"

static render_stats_t dummy_stats = {0};

uint16_t RENDER_NO_TEXTURE;

void render_init(vec2i_t screen_size) {
	(void) screen_size;
}
void render_cleanup(void) {}

void render_set_screen_size(vec2i_t size) {
	(void) size;
}
void render_set_resolution(render_resolution_t res) {
	(void) res;
}
void render_set_post_effect(render_post_effect_t post) {
	(void) post;
}
vec2i_t render_size(void) {
	return vec2i(0, 0);
}

void render_frame_prepare(void) {}
void render_frame_end(void) {}
const render_stats_t* render_frame_get_stats(void) {
	return &dummy_stats;
}

void render_set_view(vec3_t pos, vec3_t angles) {
	(void) pos; (void) angles;
}
void render_set_view_2d(void) {}
void render_set_model_mat(mat4_t *m) {
	(void) m;
}
void render_set_depth_write(bool enabled) {
	(void) enabled;
}
void render_set_depth_test(bool enabled) {
	(void) enabled;
}
void render_set_depth_offset(float offset) {
	(void) offset;
}
void render_set_screen_position(vec2_t pos) {
	(void) pos;
}
void render_set_blend_mode(render_blend_mode_t mode) {
	(void) mode;
}
void render_set_cull_backface(bool enabled) {
	(void) enabled;
}

vec3_t render_transform(vec3_t pos) {
	return pos;
}
void render_push_tris(tris_t tris, uint16_t texture) {
	(void) tris; (void) texture;
}
void render_push_sprite(vec3_t pos, vec2i_t size, rgba_t color, uint16_t texture) {
	(void) pos; (void) size; (void) color; (void) texture;
}
void render_push_2d(vec2i_t pos, vec2i_t size, rgba_t color, uint16_t texture) {
	(void) pos; (void) size; (void) color; (void) texture;
}
void render_push_2d_tile(vec2i_t pos, vec2i_t uv_offset, vec2i_t uv_size, vec2i_t size, rgba_t color, uint16_t texture_index) {
	(void) pos; (void) uv_offset; (void) uv_size; (void) size; (void) color; (void) texture_index;
}

uint16_t render_texture_create(uint32_t width, uint32_t height, rgba_t *pixels) {
	(void) width; (void) height; (void) pixels;
	return 0;
}
vec2i_t render_texture_size(uint16_t texture_index) {
	(void) texture_index;
	return vec2i(0, 0);
}
void render_texture_replace_pixels(int16_t texture_index, rgba_t *pixels) {
	(void) texture_index; (void) pixels;
}
uint16_t render_textures_len(void) {
	return 0;
}
void render_textures_reset(uint16_t len) {
	(void) len;
}
void render_textures_dump(const char *path) {
	(void) path;
}
