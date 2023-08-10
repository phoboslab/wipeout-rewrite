#ifndef SFX_H
#define SFX_H

#include "../types.h"

typedef enum {
	SFX_CRUNCH,
	SFX_EBOLT,
	SFX_ENGINE_INTAKE,
	SFX_ENGINE_RUMBLE,
	SFX_ENGINE_THRUST,
	SFX_EXPLOSION_1,
	SFX_EXPLOSION_2,
	SFX_IMPACT,
	SFX_MENU_MOVE,
	SFX_MENU_SELECT,
	SFX_MENU_TRANSITION,
	SFX_MINE_DROP,
	SFX_MISSILE_FIRE,
	SFX_ENGINE_REMOTE,
	SFX_POWERUP,
	SFX_SHIELD,
	SFX_SIREN,
	SFX_TRACTOR,
	SFX_TURBULENCE,
	SFX_CROWD,
	SFX_VOICE_MINES,
	SFX_VOICE_MISSILE,
	SFX_VOICE_ROCKETS,
	SFX_VOICE_REVCON,
	SFX_VOICE_SHOCKWAVE,
	SFX_VOICE_SPECIAL,
	SFX_VOICE_COUNT_3,
	SFX_VOICE_COUNT_2,
	SFX_VOICE_COUNT_1,
	SFX_VOICE_COUNT_GO,
} sfx_source_t;

typedef enum {
	SFX_NONE       = 0,
	SFX_PLAY       = (1<<0),
	SFX_RESERVE    = (1<<1),
	SFX_LOOP       = (1<<2),
	SFX_LOOP_PAUSE = (1<<3),
} sfx_flags_t;

typedef struct {
	sfx_source_t source;
	sfx_flags_t flags;
	float pan;
	float current_pan;
	float volume;
	float current_volume;
	float pitch;
	float position;
} sfx_t;

#define SFX_MAX 64
#define SFX_MAX_ACTIVE 16

void sfx_load();
void sfx_stero_mix(float *buffer, uint32_t len);
void sfx_set_external_mix_cb(void (*cb)(float *, uint32_t len));
void sfx_reset();
void sfx_pause();
void sfx_unpause();

sfx_t *sfx_play(sfx_source_t source_index);
sfx_t *sfx_play_at(sfx_source_t source_index, vec3_t pos, vec3_t vel, float volume);
sfx_t *sfx_reserve_loop(sfx_source_t source_index);
void sfx_set_position(sfx_t *sfx, vec3_t pos, vec3_t vel, float volume);

typedef enum {
	SFX_MUSIC_PAUSED,
	SFX_MUSIC_RANDOM,
	SFX_MUSIC_SEQUENTIAL,
	SFX_MUSIC_LOOP
} sfx_music_mode_t;

void sfx_music_next();
void sfx_music_play(uint32_t index);
void sfx_music_mode(sfx_music_mode_t);
void sfx_music_pause();

#endif