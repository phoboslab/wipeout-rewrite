#include "../types.h"
#include "../mem.h"
#include "../utils.h"
#include "../platform.h"

#include "object.h"
#include "track.h"
#include "ship.h"
#include "weapon.h"
#include "droid.h"
#include "camera.h"
#include "object.h"
#include "scene.h"
#include "game.h"
#include "hud.h"
#include "image.h"


#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>


#define TIM_TYPE_PALETTED_4_BPP 0x08
#define TIM_TYPE_PALETTED_8_BPP 0x09
#define TIM_TYPE_TRUE_COLOR_16_BPP 0x02

static inline rgba_t tim_16bit_to_rgba(uint16_t c, bool transparent_bit) {
	return rgba(
		((c >>  0) & 0x1f) << 3,
		((c >>  5) & 0x1f) << 3,
		((c >> 10) & 0x1f) << 3,
		(c == 0 
			? 0x00
			: transparent_bit && (c & 0x7fff) == 0 ? 0x00 : 0xff
		)
	);
}

image_t *image_alloc(uint32_t width, uint32_t height) {
	image_t *image = mem_temp_alloc(sizeof(image_t) + width * height * sizeof(rgba_t));
	image->width = width;
	image->height = height;
	image->pixels = (rgba_t *)(((uint8_t *)image) + sizeof(image_t));
	return image;
}

image_t *image_load_from_bytes(uint8_t *bytes, bool transparent) {
	uint32_t p = 0;

	uint32_t magic = get_i32_le(bytes, &p);
	uint32_t type = get_i32_le(bytes, &p);
	rgba_t palette[256];

	if (
		type == TIM_TYPE_PALETTED_4_BPP ||
		type == TIM_TYPE_PALETTED_8_BPP
	) {
		uint32_t header_length = get_i32_le(bytes, &p);
		uint16_t palette_x = get_i16_le(bytes, &p);
		uint16_t palette_y = get_i16_le(bytes, &p);
		uint16_t palette_colors = get_i16_le(bytes, &p);
		uint16_t palettes = get_i16_le(bytes, &p);
		for (int i = 0; i < palette_colors; i++) {
			palette[i] = tim_16bit_to_rgba(get_u16_le(bytes, &p), transparent);
		}
	}

	uint32_t data_size = get_i32_le(bytes, &p);

	int32_t pixels_per_16bit = 1;
	if (type == TIM_TYPE_PALETTED_8_BPP) {
		pixels_per_16bit = 2;
	}
	else if (type == TIM_TYPE_PALETTED_4_BPP) {
		pixels_per_16bit = 4;
	}

	uint16_t skip_x = get_i16_le(bytes, &p);
	uint16_t skip_y = get_i16_le(bytes, &p);
	uint16_t entries_per_row  = get_i16_le(bytes, &p);
	uint16_t rows = get_i16_le(bytes, &p);

	int32_t width = entries_per_row * pixels_per_16bit;
	int32_t height = rows;
	int32_t entries = entries_per_row * rows;

	image_t *image = image_alloc(width, height);
	int32_t pixel_pos = 0;

	if (type == TIM_TYPE_TRUE_COLOR_16_BPP) {
		for (int i = 0; i < entries; i++) {
			image->pixels[pixel_pos++] = tim_16bit_to_rgba(get_u16_le(bytes, &p), transparent);
		}
	}
	else if (type == TIM_TYPE_PALETTED_8_BPP) {
		for (int i = 0; i < entries; i++) {
			int32_t palette_pos = get_i16_le(bytes, &p);
			image->pixels[pixel_pos++] = palette[(palette_pos >> 0) & 0xff];
			image->pixels[pixel_pos++] = palette[(palette_pos >> 8) & 0xff];
		}
	}
	else if (type == TIM_TYPE_PALETTED_4_BPP) {
		for (int i = 0; i < entries; i++) {
			int32_t palette_pos = get_i16_le(bytes, &p);
			image->pixels[pixel_pos++] = palette[(palette_pos >>  0) & 0xf];
			image->pixels[pixel_pos++] = palette[(palette_pos >>  4) & 0xf];
			image->pixels[pixel_pos++] = palette[(palette_pos >>  8) & 0xf];
			image->pixels[pixel_pos++] = palette[(palette_pos >> 12) & 0xf];
		}
	}

	return image;
}

#define LZSS_INDEX_BIT_COUNT  13
#define LZSS_LENGTH_BIT_COUNT 4
#define LZSS_WINDOW_SIZE      (1 << LZSS_INDEX_BIT_COUNT)
#define LZSS_BREAK_EVEN       ((1 + LZSS_INDEX_BIT_COUNT + LZSS_LENGTH_BIT_COUNT) / 9)
#define LZSS_END_OF_STREAM    0
#define LZSS_MOD_WINDOW(a)    ((a) & (LZSS_WINDOW_SIZE - 1))

void lzss_decompress(uint8_t *in_data, uint8_t *out_data) {
	int16_t i;
	int16_t current_position;
	uint8_t cc;
	int16_t match_length;
	int16_t match_position;
	uint32_t mask;
	uint32_t return_value;
	uint8_t in_bfile_mask;
	int16_t in_bfile_rack;
	int16_t value;
	uint8_t window[LZSS_WINDOW_SIZE];

	in_bfile_rack = 0;
	in_bfile_mask = 0x80;

	current_position = 1;
	while (true) {
		if (in_bfile_mask == 0x80) {
			in_bfile_rack = (int16_t) * in_data++;
		}

		value = in_bfile_rack & in_bfile_mask;
		in_bfile_mask >>= 1;
		if (in_bfile_mask == 0) {
			in_bfile_mask = 0x80;
		}

		if (value) {
			mask = 1L << (8 - 1);
			return_value = 0;
			while (mask != 0) {
				if (in_bfile_mask == 0x80) {
					in_bfile_rack = (int16_t) * in_data++;
				}

				if (in_bfile_rack & in_bfile_mask) {
					return_value |= mask;
				}
				mask >>= 1;
				in_bfile_mask >>= 1;

				if (in_bfile_mask == 0) {
					in_bfile_mask = 0x80;
				}
			}
			cc = (uint8_t) return_value;
			*out_data++ = cc;
			window[ current_position ] = cc;
			current_position = LZSS_MOD_WINDOW(current_position + 1);
		}
		else {
			mask = 1L << (LZSS_INDEX_BIT_COUNT - 1);
			return_value = 0;
			while (mask != 0) {
				if (in_bfile_mask == 0x80) {
					in_bfile_rack = (int16_t) * in_data++;
				}

				if (in_bfile_rack & in_bfile_mask) {
					return_value |= mask;
				}
				mask >>= 1;
				in_bfile_mask >>= 1;

				if (in_bfile_mask == 0) {
					in_bfile_mask = 0x80;
				}
			}
			match_position = (int16_t) return_value;

			if (match_position == LZSS_END_OF_STREAM) {
				break;
			}

			mask = 1L << (LZSS_LENGTH_BIT_COUNT - 1);
			return_value = 0;
			while (mask != 0) {
				if (in_bfile_mask == 0x80) {
					in_bfile_rack = (int16_t) * in_data++;
				}

				if (in_bfile_rack & in_bfile_mask) {
					return_value |= mask;
				}
				mask >>= 1;
				in_bfile_mask >>= 1;

				if (in_bfile_mask == 0) {
					in_bfile_mask = 0x80;
				}
			}
			match_length = (int16_t) return_value;

			match_length += LZSS_BREAK_EVEN;

			for (i = 0 ; i <= match_length ; i++) {
				cc = window[LZSS_MOD_WINDOW(match_position + i)];
				*out_data++ = cc;
				window[current_position] = cc;
				current_position = LZSS_MOD_WINDOW(current_position + 1);
			}
		}
	}
}

cmp_t *image_load_compressed(char *name) {
	printf("load cmp %s\n", name);
	uint32_t compressed_size;
	uint8_t *compressed_bytes = platform_load_asset(name, &compressed_size);

	uint32_t p = 0;
	int32_t decompressed_size = 0;
	int32_t image_count = get_i32_le(compressed_bytes, &p);

	// Calculate the total uncompressed size
	for (int i = 0; i < image_count; i++) {
		decompressed_size += get_i32_le(compressed_bytes, &p);
	}

	uint32_t struct_size = sizeof(cmp_t) + sizeof(uint8_t *) * image_count;
	cmp_t *cmp = mem_temp_alloc(struct_size + decompressed_size);
	cmp->len = image_count;

	uint8_t *decompressed_bytes = ((uint8_t *)cmp) + struct_size;

	// Rewind and load all offsets
	p = 4;
	uint32_t offset = 0;
	for (int i = 0; i < image_count; i++) {
		cmp->entries[i] = decompressed_bytes + offset;
		offset += get_i32_le(compressed_bytes, &p);
	}

	lzss_decompress(compressed_bytes + p, decompressed_bytes);
	mem_temp_free(compressed_bytes);

	return cmp;
}

uint16_t image_get_texture(char *name) {
	printf("load: %s\n", name);
	uint32_t size;
	uint8_t *bytes = platform_load_asset(name, &size);
	image_t *image = image_load_from_bytes(bytes, false);
	uint32_t texture_index = render_texture_create(image->width, image->height, image->pixels);
	mem_temp_free(image);
	mem_temp_free(bytes);

	return texture_index;
}

uint16_t image_get_texture_semi_trans(char *name) {
	printf("load: %s\n", name);
	uint32_t size;
	uint8_t *bytes = platform_load_asset(name, &size);
	image_t *image = image_load_from_bytes(bytes, true);
	uint32_t texture_index = render_texture_create(image->width, image->height, image->pixels);
	mem_temp_free(image);
	mem_temp_free(bytes);

	return texture_index;
}

texture_list_t image_get_compressed_textures(char *name) {
	cmp_t *cmp = image_load_compressed(name);
	texture_list_t list = {.start = render_textures_len(), .len = cmp->len};

	for (int i = 0; i < cmp->len; i++) {
		int32_t width, height;
		image_t *image = image_load_from_bytes(cmp->entries[i], false);

		// char png_name[1024] = {0};
		// sprintf(png_name, "%s.%d.png", name, i);
		// stbi_write_png(png_name, image->width, image->height, 4, image->pixels, 0);

		render_texture_create(image->width, image->height, image->pixels);
		mem_temp_free(image);
	}

	mem_temp_free(cmp);
	return list;
}

uint16_t texture_from_list(texture_list_t tl, uint16_t index) {
	error_if(index >= tl.len, "Texture %d not in list of len %d", index, tl.len);
	return tl.start + index;
}

void image_copy(image_t *src, image_t *dst, uint32_t sx, uint32_t sy, uint32_t sw, uint32_t sh, uint32_t dx, uint32_t dy) {
	rgba_t *src_pixels = src->pixels + sy * src->width + sx;
	rgba_t *dst_pixels = dst->pixels + dy * dst->width + dx;
	for (uint32_t y = 0; y < sh; y++) {
		for (uint32_t x = 0; x < sw; x++) {
			*(dst_pixels++) = *(src_pixels++);
		}
		src_pixels += src->width - sw;
		dst_pixels += dst->width - sw;
	}
}

