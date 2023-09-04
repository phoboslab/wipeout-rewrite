#ifndef INIT_H
#define INIT_H

#include "../types.h"

typedef struct {
	uint16_t start;
	uint16_t len;
} texture_list_t;

#define texture_list_empty() ((texture_list_t){0, 0})

typedef struct {
	uint32_t width;
	uint32_t height;
	rgba_t *pixels;
} image_t;

typedef struct {
	uint32_t len;
	uint8_t *entries[];
} cmp_t;

image_t *image_alloc(uint32_t width, uint32_t height);
void image_copy(image_t *src, image_t *dst, uint32_t sx, uint32_t sy, uint32_t sw, uint32_t sh, uint32_t dx, uint32_t dy);
image_t *image_load_from_bytes(uint8_t *bytes, bool transparent);
cmp_t *image_load_compressed(char *name);

uint16_t image_get_texture(char *name);
uint16_t image_get_texture_semi_trans(char *name);
texture_list_t image_get_compressed_textures(char *name);
uint16_t texture_from_list(texture_list_t tl, uint16_t index);

#endif