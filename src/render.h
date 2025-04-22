#ifndef RENDER_H
#define RENDER_H

#include "types.h"

typedef enum {
	RENDER_BLEND_NORMAL,
	RENDER_BLEND_LIGHTER
} render_blend_mode_t;

typedef enum {
	RENDER_RES_NATIVE,
	RENDER_RES_240P,
	RENDER_RES_480P,
} render_resolution_t;

typedef enum {
	RENDER_POST_NONE,
	RENDER_POST_CRT,
	NUM_RENDER_POST_EFFCTS,
} render_post_effect_t;

#define RENDER_USE_MIPMAPS 1

#define RENDER_FADEOUT_NEAR 48000.0
#define RENDER_FADEOUT_FAR 64000.0

extern uint16_t RENDER_NO_TEXTURE;

void render_init(vec2i_t screen_size);
void render_cleanup(void);

void render_set_screen_size(vec2i_t size);
void render_set_resolution(render_resolution_t res);
void render_set_post_effect(render_post_effect_t post);
vec2i_t render_size(void);

void render_frame_prepare(void);
void render_frame_end(void);

void render_set_view(vec3_t pos, vec3_t angles);
void render_set_view_2d(void);
void render_set_model_mat(mat4_t *m);
void render_set_depth_write(bool enabled);
void render_set_depth_test(bool enabled);
void render_set_depth_offset(float offset);
void render_set_screen_position(vec2_t pos);
void render_set_blend_mode(render_blend_mode_t mode);

vec3_t render_transform(vec3_t pos);
void render_push_tris(tris_t tris, uint16_t texture);
void render_push_sprite(vec3_t pos, vec2i_t size, rgba_t color, uint16_t texture, int16_t prm_flag);
void render_push_2d(vec2i_t pos, vec2i_t size, rgba_t color, uint16_t texture);
void render_push_2d_tile(vec2i_t pos, vec2i_t uv_offset, vec2i_t uv_size, vec2i_t size, rgba_t color, uint16_t texture_index);

uint16_t render_texture_create(uint32_t width, uint32_t height, rgba_t *pixels);
vec2i_t render_texture_size(uint16_t texture_index);
void render_texture_replace_pixels(int16_t texture_index, rgba_t *pixels);
uint16_t render_textures_len(void);
void render_textures_reset(uint16_t len);
void render_textures_dump(const char *path);

#endif
