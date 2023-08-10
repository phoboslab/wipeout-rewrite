#ifndef DROID_H
#define DROID_H

#include "../types.h"
#include "track.h"
#include "ship.h"
#include "sfx.h"

#define DROID_UPDATE_TIME_INITIAL (800 * (1.0/30.0))
#define DROID_UPDATE_TIME_INTRO_1 (770 * (1.0/30.0))
#define DROID_UPDATE_TIME_INTRO_2 (710 * (1.0/30.0))
#define DROID_UPDATE_TIME_INTRO_3 (400 * (1.0/30.0))

typedef struct droid_t {
	section_t *section;
	vec3_t position;
	vec3_t velocity;
	vec3_t acceleration;
	vec3_t angle;
	vec3_t angular_velocity;
	bool siren_started;
	float cycle_timer;
	float update_timer;
	void (*update_func)(struct droid_t *, ship_t *);
	mat4_t mat;
	Object *model;
	sfx_t *sfx_tractor;
} droid_t;

void droid_draw(droid_t *droid);

void droid_load();
void droid_init(droid_t *droid, ship_t *ship);
void droid_update(droid_t *droid, ship_t *ship);
void droid_update_intro(droid_t *droid, ship_t *ship);
void droid_update_idle(droid_t *droid, ship_t *ship);
void droid_update_rescue(droid_t *droid, ship_t *ship);

#endif
