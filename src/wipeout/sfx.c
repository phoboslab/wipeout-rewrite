#include "../utils.h"
#include "../mem.h"
#include "../platform.h"

#include "sfx.h"
#include "game.h"

#define QOA_IMPLEMENTATION
#define QOA_NO_STDIO
#include <qoa.h>

typedef struct {
	int16_t *samples;
	uint32_t len;
} sfx_data_t;

typedef struct {
	qoa_desc qoa;
	FILE *file;

	uint32_t track_index;
	uint32_t first_frame_pos;

	uint32_t buffer_len;
	uint8_t *buffer;

	uint32_t sample_data_pos;
	uint32_t sample_data_len;
	short *sample_data;
	sfx_music_mode_t mode;
} music_decoder_t;

enum {
	VAG_REGION_START = 1,
	VAG_REGION = 2,
	VAG_REGION_END = 4
};

static const int32_t vag_tab[5][2] = {
	{    0,      0}, // {         0.0,          0.0}, << 14
	{15360,      0}, // { 60.0 / 64.0,          0.0}, << 14
	{29440, -13312}, // {115.0 / 64.0, -52.0 / 64.0}, << 14
	{25088, -14080}, // { 98.0 / 64.0, -55.0 / 64.0}, << 14
	{31232, -15360}, // {122.0 / 64.0, -60.0 / 64.0}, << 14
};

static sfx_data_t *sources;
static uint32_t num_sources;
static sfx_t *nodes;
static music_decoder_t *music;
static void (*external_mix_cb)(float *, uint32_t len) = NULL;

void sfx_load(void) {
	// Init decode buffer for music
	uint32_t channels = 2;
	music = mem_bump(sizeof(music_decoder_t));
	music->buffer = mem_bump(QOA_FRAME_SIZE(channels, QOA_SLICES_PER_FRAME));
	music->sample_data = mem_bump(channels * QOA_FRAME_LEN * sizeof(short) * 2);
	music->qoa.channels = channels;
	music->mode = SFX_MUSIC_RANDOM;
	music->file = NULL;
	music->track_index = -1;


	// Load SFX samples
	nodes = mem_bump(SFX_MAX * sizeof(sfx_t));

	// 16 byte blocks: 2 byte header, 14 bytes with 2x4bit samples each
	uint32_t vb_size;
	uint8_t *vb = platform_load_asset("wipeout/sound/wipeout.vb", &vb_size);
	uint32_t num_samples = (vb_size / 16) * 28;

	int16_t *sample_buffer = mem_bump(num_samples * sizeof(int16_t));
	sources = mem_mark();
	num_sources = 0;

	uint32_t sample_index = 0;
	int32_t history[2] = {0, 0};
	for (unsigned int p = 0; p < vb_size;) {
		uint8_t header = vb[p++];
		uint8_t flags = vb[p++];
		uint8_t shift = header & 0x0f;
		uint8_t predictor = clamp(header >> 4, 0, 4);

		if (flags_is(flags, VAG_REGION_END)) {
			mem_bump(sizeof(sfx_data_t));
			sources[num_sources].samples = &sample_buffer[sample_index];
		}

		for (uint32_t bs = 0; bs < 14; bs++) {
			int32_t nibbles[2] = {
				(vb[p] & 0x0f) << 12,
				(vb[p] & 0xf0) <<  8
			};
			p++;

			for (int ni = 0; ni < 2; ni++) {
				int32_t sample = nibbles[ni];
				if (sample & 0x8000) {
					sample |= 0xffff0000;
				}
				sample >>= shift;
				sample += (history[0] * vag_tab[predictor][0] + history[1] * vag_tab[predictor][1]) >> 14;
				history[1] = history[0];
				history[0] = sample;
				sample_buffer[sample_index++] = clamp(sample, -32768, 32767);
			}
		}

		if (flags_is(flags, VAG_REGION_START)) {
			error_if(sources[num_sources].samples == NULL, "VAG_REGION_START without VAG_REGION_END");
			sources[num_sources].len = &sample_buffer[sample_index] - sources[num_sources].samples;
			num_sources++;
		}
	}

	mem_temp_free(vb);
	platform_set_audio_mix_cb(sfx_stero_mix);
}

void sfx_reset(void) {
	for (int i = 0; i < SFX_MAX; i++) {
		if (flags_is(nodes[i].flags, SFX_LOOP)) {
			flags_set(nodes[i].flags, SFX_NONE);
		}
	}
}

void sfx_unpause(void) {
	for (int i = 0; i < SFX_MAX; i++) {
		if (flags_is(nodes[i].flags, SFX_LOOP_PAUSE)) {
			flags_rm(nodes[i].flags, SFX_LOOP_PAUSE);
			flags_add(nodes[i].flags, SFX_PLAY);
		}
	}
}

void sfx_pause(void) {
	for (int i = 0; i < SFX_MAX; i++) {
		if (flags_is(nodes[i].flags, SFX_PLAY | SFX_LOOP)) {
			flags_rm(nodes[i].flags, SFX_PLAY);
			flags_add(nodes[i].flags, SFX_LOOP_PAUSE);
		}
	}
}



// Sound effects

sfx_t *sfx_get_node(sfx_source_t source_index) {
	error_if(source_index < 0 || source_index > num_sources, "Invalid audio source");

	sfx_t *sfx = NULL;
	for (int i = 0; i < SFX_MAX; i++) {
		if (flags_none(nodes[i].flags, SFX_PLAY | SFX_RESERVE)){
			sfx = &nodes[i];
			break;
		}
	}
	if (!sfx) {
		for (int i = 0; i < SFX_MAX; i++) {
			if (flags_not(nodes[i].flags, SFX_RESERVE)) {
				sfx = &nodes[i];
				break;
			}
		}
	}

	error_if(!sfx, "All audio nodes reserved");

	flags_set(sfx->flags, SFX_NONE);
	sfx->source = source_index;
	sfx->volume = 1;
	sfx->current_volume = 1;
	sfx->pan = 0;
	sfx->current_pan = 0;
	sfx->position = 0;

	// Set default pitch. All voice samples are 44khz, 
	// other effects 22khz
	sfx->pitch = source_index >= SFX_VOICE_MINES ? 1.0 : 0.5;

	return sfx;
}

sfx_t *sfx_play(sfx_source_t source_index) {
	sfx_t *sfx = sfx_get_node(source_index);
	flags_set(sfx->flags, SFX_PLAY);
	return sfx;
}

sfx_t *sfx_play_at(sfx_source_t source_index, vec3_t pos, vec3_t vel, float volume) {
	sfx_t *sfx = sfx_get_node(source_index);
	sfx_set_position(sfx, pos, vel, volume);
	if (sfx->volume > 0) {
		flags_set(sfx->flags, SFX_PLAY);
	}
	return sfx;
}

sfx_t *sfx_reserve_loop(sfx_source_t source_index) {
	sfx_t *sfx = sfx_get_node(source_index);
	flags_set(sfx->flags, SFX_RESERVE | SFX_LOOP | SFX_PLAY);
	sfx->volume = 0;
	sfx->current_volume = 0;
	sfx->current_pan = 0;
	sfx->position = rand_float(0, sources[source_index].len);
	return sfx;
}

void sfx_set_position(sfx_t *sfx, vec3_t pos, vec3_t vel, float volume) {
	vec3_t relative_position = vec3_sub(g.camera.position, pos);
	vec3_t relative_velocity = vec3_sub(g.camera.real_velocity, vel);
	float distance = vec3_len(relative_position);

	sfx->volume = clamp(scale(distance, 512, 32768, 1, 0), 0, 1) * volume;
	sfx->pan = -sin(atan2(g.camera.position.x - pos.x, g.camera.position.z - pos.z)+g.camera.angle.y);

	// Doppler effect
	float away = vec3_dot(relative_velocity, relative_position) / distance;
	sfx->pitch = (262144.0 - away) / 524288.0;
}




// Music

uint32_t sfx_music_decode_frame(void) {
	if (!music->file) {
		return 0;
	}
	music->buffer_len = fread(music->buffer, 1, qoa_max_frame_size(&music->qoa), music->file);

	uint32_t frame_len;
	qoa_decode_frame(music->buffer, music->buffer_len, &music->qoa, music->sample_data, &frame_len);
	music->sample_data_pos = 0;
	music->sample_data_len = frame_len;
	return frame_len;
}

void sfx_music_rewind(void) {
	fseek(music->file, music->first_frame_pos, SEEK_SET);
	music->sample_data_len = 0;
	music->sample_data_pos = 0;
}

void sfx_music_open(char *path) {
	if (music->file) {
		fclose(music->file);
		music->file = NULL;
	}
	
	FILE *file = platform_open_asset(path, "rb");
	if (!file) {
		return;
	}

	uint8_t header[QOA_MIN_FILESIZE];
	int read = fread(header, QOA_MIN_FILESIZE, 1, file);
	if (!read) {
		fclose(file);
		return;
	}

	qoa_desc qoa;
	uint32_t first_frame_pos = qoa_decode_header(header, QOA_MIN_FILESIZE, &qoa);
	if (!first_frame_pos) {
		fclose(file);
		return;
	}

	fseek(file, first_frame_pos, SEEK_SET);

	if (qoa.channels != music->qoa.channels) {
		fclose(file);
		return;
	}
	music->qoa = qoa;
	music->first_frame_pos = first_frame_pos;
	music->file = file;
	music->sample_data_len = 0;
	music->sample_data_pos = 0;
}

void sfx_music_play(uint32_t index) {
	error_if(index >= len(def.music), "Invalid music index");
	if (index == music->track_index) {
		sfx_music_rewind();
		return;
	}

	printf("open music track %d\n", index);

	music->track_index = index;
	sfx_music_open(def.music[index].path);
}

void sfx_music_mode(sfx_music_mode_t mode) {
	music->mode = mode;
}





// Mixing

void sfx_set_external_mix_cb(void (*cb)(float *, uint32_t len)) {
	external_mix_cb = cb;
}

void sfx_stero_mix(float *buffer, uint32_t len) {
	if (external_mix_cb) {
		external_mix_cb(buffer, len);
		return;
	}

	// Find currently active nodes: those that play and have volume > 0
	sfx_t *active_nodes[SFX_MAX_ACTIVE];
	int active_nodes_len = 0;
	for (int n = 0; n < SFX_MAX; n++) {
		sfx_t *sfx = &nodes[n];
		if (flags_is(sfx->flags, SFX_PLAY) && (sfx->volume > 0 || sfx->current_volume > 0.01)) {
			active_nodes[active_nodes_len++] = sfx;
			if (active_nodes_len >= SFX_MAX_ACTIVE) {
				break;
			}
		}
	}

	uint32_t music_src_index = music->sample_data_pos * music->qoa.channels;

	for (uint32_t i = 0; i < len; i += 2) {
		float left = 0;
		float right = 0;

		// Fill buffer with all active nodes
		for (int n = 0; n < active_nodes_len; n++) {
			sfx_t *sfx = active_nodes[n];
			if (flags_not(sfx->flags, SFX_PLAY)) {
				continue;
			}

			sfx->current_volume = sfx->current_volume * 0.999 + sfx->volume * 0.001;
			sfx->current_pan = sfx->current_pan * 0.999 + sfx->pan * 0.001;

			sfx_data_t *source = &sources[sfx->source];
			float sample = (float)source->samples[(int)sfx->position] / 32768.0;
			left += sample * sfx->current_volume * clamp(1.0 - sfx->current_pan, 0, 1);
			right += sample * sfx->current_volume * clamp(1.0 + sfx->current_pan, 0, 1);

			sfx->position += sfx->pitch;
			if (sfx->position >= source->len) {
				if (flags_is(sfx->flags, SFX_LOOP)) {
					sfx->position = fmod(sfx->position, source->len);
				}
				else {
					flags_rm(sfx->flags, SFX_PLAY);
				}
			}
		}

		left *= save.sfx_volume;
		right *= save.sfx_volume;

		// Mix in music
		if (music->mode != SFX_MUSIC_PAUSED && music->file) {
			if (music->sample_data_len - music->sample_data_pos == 0) {
				if (!sfx_music_decode_frame()) {
					if (music->mode == SFX_MUSIC_RANDOM) {
						sfx_music_play(rand_int(0, len(def.music)));
					}
					else if (music->mode == SFX_MUSIC_SEQUENTIAL) {
						sfx_music_play((music->track_index + 1) % len(def.music));
					}
					else if (music->mode == SFX_MUSIC_LOOP) {
						sfx_music_rewind();
					}
					sfx_music_decode_frame();
				}
				music_src_index = 0;
			}
			left += (music->sample_data[music_src_index++] / 32768.0) * save.music_volume;
			right += (music->sample_data[music_src_index++] / 32768.0) * save.music_volume;
			music->sample_data_pos++;
		}

		buffer[i+0] = left;
		buffer[i+1] = right;
	}
}
