#include "../system.h"
#include "../input.h"
#include "../utils.h"
#include "../types.h"
#include "../mem.h"

#include "intro.h"
#include "ui.h"
#include "image.h"
#include "game.h"

void free_dummmy(void *p) {}
void *realloc_dummmy(void *p, size_t sz) {
	die("pl_mpeg needed to realloc. Not implemented. Maybe increase PLM_BUFFER_DEFAULT_SIZE");
	return NULL;
}

#define PL_MPEG_IMPLEMENTATION
#define PLM_MALLOC mem_bump
#define PLM_FREE free_dummmy
#define PLM_REALLOC realloc_dummmy
#include "../libs/pl_mpeg.h"

#define INTRO_AUDIO_BUFFER_LEN (64 * 1024)

static plm_t *plm;
static rgba_t *frame_buffer;
static int16_t texture;
static float *audio_buffer;
static int audio_buffer_read_pos;
static int audio_buffer_write_pos;

static void video_cb(plm_t *plm, plm_frame_t *frame, void *user);
static void audio_cb(plm_t *plm, plm_samples_t *samples, void *user);
static void audio_mix(float *samples, uint32_t len);
static void intro_end();

void intro_init() {
	plm = plm_create_with_filename("wipeout/intro.mpeg");
	if (!plm) {
		intro_end();
		return;
	}
	plm_set_video_decode_callback(plm, video_cb, NULL);
	plm_set_audio_decode_callback(plm, audio_cb, NULL);
	
	plm_set_loop(plm, false);
	plm_set_audio_enabled(plm, true);
	plm_set_audio_stream(plm, 0);

	int w = plm_get_width(plm);
	int h = plm_get_height(plm);
	frame_buffer = mem_bump(w * h * sizeof(rgba_t));
	for (int i = 0; i < w * h; i++) {
		frame_buffer[i] = rgba(0, 0, 0, 255);
	}
	texture = render_texture_create(w, h, frame_buffer);

	sfx_set_external_mix_cb(audio_mix);
	audio_buffer = mem_bump(INTRO_AUDIO_BUFFER_LEN * sizeof(float) * 2);
	audio_buffer_read_pos = 0;
	audio_buffer_write_pos = 0;
}

static void intro_end() {
	sfx_set_external_mix_cb(NULL);
	game_set_scene(GAME_SCENE_TITLE);
}

void intro_update() {
	if (!plm) {
		return;
	}
	plm_decode(plm, system_tick());
	render_set_view_2d();
	render_push_2d(vec2i(0,0), render_size(), rgba(128, 128, 128, 255), texture);
	if (plm_has_ended(plm) || input_pressed(A_MENU_SELECT) || input_pressed(A_MENU_START)) {
		intro_end();
	}
}

static void audio_cb(plm_t *plm, plm_samples_t *samples, void *user) {
	int len = samples->count * 2;
	for (int i = 0; i < len; i++) {
		audio_buffer[audio_buffer_write_pos % INTRO_AUDIO_BUFFER_LEN] = samples->interleaved[i];
		audio_buffer_write_pos++;
	}
}

static void audio_mix(float *samples, uint32_t len) {
	int i;
	for (i = 0; i < len && audio_buffer_read_pos < audio_buffer_write_pos; i++) {
		samples[i] = audio_buffer[audio_buffer_read_pos % INTRO_AUDIO_BUFFER_LEN];
		audio_buffer_read_pos++;
	}
	for (; i < len; i++) {
		samples[i] = 0;
	}
}

static void video_cb(plm_t *plm, plm_frame_t *frame, void *user) {
	plm_frame_to_rgba(frame, (uint8_t *)frame_buffer, plm_get_width(plm) * sizeof(rgba_t));
	render_texture_replace_pixels(texture, frame_buffer);
}

