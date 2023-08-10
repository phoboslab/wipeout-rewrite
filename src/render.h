#ifndef RENDER_H
#define RENDER_H

#include "types.h"

typedef enum {
	RENDER_BLEND_NORMAL,
	RENDER_BLEND_LIGHTER
} render_blend_mode_t;

#define RENDER_USE_MIPMAPS 1

#define RENDER_FADEOUT_NEAR 48000.0
#define RENDER_FADEOUT_FAR 64000.0

extern uint16_t RENDER_NO_TEXTURE;

void render_init(vec2i_t size);
void render_cleanup();

void render_resize(vec2i_t size);
vec2i_t render_size();

void render_frame_prepare();
void render_frame_end();

void render_set_view(vec3_t pos, vec3_t angles);
void render_set_view_2d();
void render_set_model_mat(mat4_t *m);
void render_set_depth_write(bool enabled);
void render_set_depth_test(bool enabled);
void render_set_depth_offset(float offset);
void render_set_screen_position(vec2_t pos);
void render_set_blend_mode(render_blend_mode_t mode);
void render_set_cull_backface(bool enabled);

vec3_t render_transform(vec3_t pos);
void render_push_tris(tris_t tris, uint16_t texture);
void render_push_sprite(vec3_t pos, vec2i_t size, rgba_t color, uint16_t texture);
void render_push_2d(vec2i_t pos, vec2i_t size, rgba_t color, uint16_t texture);
void render_push_2d_tile(vec2i_t pos, vec2i_t uv_offset, vec2i_t uv_size, vec2i_t size, rgba_t color, uint16_t texture_index);

uint16_t render_texture_create(uint32_t width, uint32_t height, rgba_t *pixels);
vec2i_t render_texture_size(uint16_t texture_index);
void render_texture_replace_pixels(int16_t texture_index, rgba_t *pixels);
uint16_t render_textures_len();
void render_textures_reset(uint16_t len);
void render_textures_dump(const char *path);

#endif
