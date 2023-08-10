#include "../render.h"
#include "../utils.h"

#include "ui.h"
#include "image.h"

typedef struct {
	vec2i_t offset;
	uint16_t width;
} glyph_t;

typedef struct {
	uint16_t texture;
	uint16_t height;
	glyph_t glyphs[40];
} char_set_t;

int ui_scale = 2;

char_set_t char_set[UI_SIZE_MAX] = {
	[UI_SIZE_16] = {
		.texture = 0,
		.height = 16,
		.glyphs = {
			{{  0,   0}, 25}, {{ 25,   0}, 24}, {{ 49,   0}, 17}, {{ 66,   0}, 24}, {{ 90,   0}, 24}, {{114,   0}, 17}, {{131,   0}, 25}, {{156,   0}, 18},
			{{174,   0},  7}, {{181,   0}, 17}, {{  0,  16}, 17}, {{ 17,  16}, 17}, {{ 34,  16}, 28}, {{ 62,  16}, 17}, {{ 79,  16}, 24}, {{103,  16}, 24},
			{{127,  16}, 26}, {{153,  16}, 24}, {{177,  16}, 18}, {{195,  16}, 17}, {{  0,  32}, 17}, {{ 17,  32}, 17}, {{ 34,  32}, 29}, {{ 63,  32}, 24},
			{{ 87,  32}, 17}, {{104,  32}, 18}, {{122,  32}, 24}, {{146,  32}, 10}, {{156,  32}, 18}, {{174,  32}, 17}, {{191,  32}, 18}, {{  0,  48}, 18},
			{{ 18,  48}, 18}, {{ 36,  48}, 18}, {{ 54,  48}, 22}, {{ 76,  48}, 25}, {{101,  48},  7}, {{108,  48},  7}, {{198,   0},  0}, {{198,   0},  0}
		}
	},
	[UI_SIZE_12] = {
		.texture = 0,
		.height = 12,
		.glyphs = {
			{{  0,   0}, 19}, {{ 19,   0}, 19}, {{ 38,   0}, 14}, {{ 52,   0}, 19}, {{ 71,   0}, 19}, {{ 90,   0}, 13}, {{103,   0}, 19}, {{122,   0}, 14},
			{{136,   0},  6}, {{142,   0}, 13}, {{155,   0}, 14}, {{169,   0}, 14}, {{  0,  12}, 22}, {{ 22,  12}, 14}, {{ 36,  12}, 19}, {{ 55,  12}, 18},
			{{ 73,  12}, 20}, {{ 93,  12}, 19}, {{112,  12}, 15}, {{127,  12}, 14}, {{141,  12}, 13}, {{154,  12}, 13}, {{167,  12}, 22}, {{  0,  24}, 19},
			{{ 19,  24}, 13}, {{ 32,  24}, 14}, {{ 46,  24}, 19}, {{ 65,  24},  8}, {{ 73,  24}, 15}, {{ 88,  24}, 13}, {{101,  24}, 14}, {{115,  24}, 15},
			{{130,  24}, 14}, {{144,  24}, 15}, {{159,  24}, 18}, {{177,  24}, 19}, {{196,  24},  5}, {{201,  24},  5}, {{183,   0},  0}, {{183,   0},  0}
		}
	},
	[UI_SIZE_8] = {
		.texture = 0,
		.height = 8,
		.glyphs = {
			{{  0,   0}, 13}, {{ 13,   0}, 13}, {{ 26,   0}, 10}, {{ 36,   0}, 13}, {{ 49,   0}, 13}, {{ 62,   0},  9}, {{ 71,   0}, 13}, {{ 84,   0}, 10},
			{{ 94,   0},  4}, {{ 98,   0},  9}, {{107,   0}, 10}, {{117,   0}, 10}, {{127,   0}, 16}, {{143,   0}, 10}, {{153,   0}, 13}, {{166,   0}, 13},
			{{179,   0}, 14}, {{  0,   8}, 13}, {{ 13,   8}, 10}, {{ 23,   8},  9}, {{ 32,   8},  9}, {{ 41,   8},  9}, {{ 50,   8}, 16}, {{ 66,   8}, 14},
			{{ 80,   8},  9}, {{ 89,   8}, 10}, {{ 99,   8}, 13}, {{112,   8},  6}, {{118,   8}, 11}, {{129,   8}, 10}, {{139,   8}, 10}, {{149,   8}, 11},
			{{160,   8}, 10}, {{170,   8}, 10}, {{180,   8}, 12}, {{192,   8}, 14}, {{206,   8},  4}, {{210,   8},  4}, {{193,   0},  0}, {{193,   0},  0}
		}
	},
};

uint16_t icon_textures[UI_ICON_MAX];

void ui_load() {
	texture_list_t tl = image_get_compressed_textures("wipeout/textures/drfonts.cmp");
	char_set[UI_SIZE_16].texture   = texture_from_list(tl, 0);
	char_set[UI_SIZE_12].texture   = texture_from_list(tl, 1);
	char_set[UI_SIZE_8 ].texture   = texture_from_list(tl, 2);
	icon_textures[UI_ICON_HAND]    = texture_from_list(tl, 3);
	icon_textures[UI_ICON_CONFIRM] = texture_from_list(tl, 5);
	icon_textures[UI_ICON_CANCEL]  = texture_from_list(tl, 6);
	icon_textures[UI_ICON_END]     = texture_from_list(tl, 7);
	icon_textures[UI_ICON_DEL]     = texture_from_list(tl, 8);
	icon_textures[UI_ICON_STAR]    = texture_from_list(tl, 9);
}

int ui_get_scale() {
	return ui_scale;
}

void ui_set_scale(int scale) {
	ui_scale = scale;
}


vec2i_t ui_scaled(vec2i_t v) {
	return vec2i(v.x * ui_scale, v.y * ui_scale);
}

vec2i_t ui_scaled_screen() {
	return vec2i_mulf(render_size(), ui_scale);
}

vec2i_t ui_scaled_pos(ui_pos_t anchor, vec2i_t offset) {
	vec2i_t pos;
	vec2i_t screen_size = render_size();

	if (flags_is(anchor, UI_POS_LEFT)) {
		pos.x = offset.x * ui_scale;
	}
	else if (flags_is(anchor, UI_POS_CENTER)) {
		pos.x = (screen_size.x >> 1) + offset.x * ui_scale;
	}
	else if (flags_is(anchor, UI_POS_RIGHT)) {
		pos.x = screen_size.x + offset.x * ui_scale;
	}

	if (flags_is(anchor, UI_POS_TOP)) {
		pos.y = offset.y * ui_scale;
	}
	else if (flags_is(anchor, UI_POS_MIDDLE)) {
		pos.y = (screen_size.y >> 1) + offset.y * ui_scale;
	}
	else if (flags_is(anchor, UI_POS_BOTTOM)) {
		pos.y = screen_size.y + offset.y * ui_scale;
	}

	return pos;
}

#define char_to_glyph_index(C) (C >= '0' && C <= '9' ? (C - '0' + 26) : C - 'A')

int ui_char_width(char c, ui_text_size_t size) {
	if (c == ' ') {
		return 8;
	}
	return char_set[size].glyphs[char_to_glyph_index(c)].width;
}

int ui_text_width(const char *text, ui_text_size_t size) {
	int width = 0;
	char_set_t *cs = &char_set[size];

	for (int i = 0; text[i] != 0; i++) {
		width += text[i] != ' '
			? cs->glyphs[char_to_glyph_index(text[i])].width
			: 8;
	}

	return width;
}

int ui_number_width(int num, ui_text_size_t size) {
	char text_buffer[16];
	text_buffer[15] = '\0';

	int i;
	for (i = 14; i > 0; i--) {
		text_buffer[i] = '0' + (num % 10);
		num = num / 10;
		if (num == 0) {
			break;
		}
	}
	return ui_text_width(text_buffer + i, size);
}

void ui_draw_time(float time, vec2i_t pos, ui_text_size_t size, rgba_t color) {
	int msec = time * 1000;
	int tenths = (msec / 100) % 10;
	int secs = (msec / 1000) % 60;
	int mins = msec / (60 * 1000);

	char text_buffer[8];
	text_buffer[0] = '0' + (mins / 10) % 10;
	text_buffer[1] = '0' + mins % 10;
	text_buffer[2] = 'e'; // ":"
	text_buffer[3] = '0' + secs / 10;
	text_buffer[4] = '0' + secs % 10;
	text_buffer[5] = 'f'; // "."
	text_buffer[6] = '0' + tenths;
	text_buffer[7] = '\0';
	ui_draw_text(text_buffer, pos, size, color);
}

void ui_draw_number(int num, vec2i_t pos, ui_text_size_t size, rgba_t color) {
	char text_buffer[16];
	text_buffer[15] = '\0';

	int i;
	for (i = 14; i > 0; i--) {
		text_buffer[i] = '0' + (num % 10);
		num = num / 10;
		if (num == 0) {
			break;
		}
	}
	ui_draw_text(text_buffer + i, pos, size, color);
}

void ui_draw_text(const char *text, vec2i_t pos, ui_text_size_t size, rgba_t color) {
	char_set_t *cs = &char_set[size];

	for (int i = 0; text[i] != 0; i++) {
		if (text[i] != ' ') {
			glyph_t *glyph = &cs->glyphs[char_to_glyph_index(text[i])];
			vec2i_t size = vec2i(glyph->width, cs->height);
			render_push_2d_tile(pos, glyph->offset, size, ui_scaled(size), color, cs->texture);
			pos.x += glyph->width * ui_scale;
		}
		else {
			pos.x += 8 * ui_scale;
		}
	}
}

void ui_draw_image(vec2i_t pos, uint16_t texture) {
	vec2i_t scaled_size = ui_scaled(render_texture_size(texture));
	render_push_2d(pos, scaled_size, rgba(128, 128, 128, 255), texture);
}

void ui_draw_icon(ui_icon_type_t icon, vec2i_t pos, rgba_t color) {
	render_push_2d(pos, ui_scaled(render_texture_size(icon_textures[icon])), color, icon_textures[icon]);
}

void ui_draw_text_centered(const char *text, vec2i_t pos, ui_text_size_t size, rgba_t color) {
	pos.x -= (ui_text_width(text, size) * ui_scale) >> 1;
	ui_draw_text(text, pos, size, color);
}
