#ifndef CAMERA_H
#define CAMERA_H

#include "../types.h"
#include "droid.h"

#define CAMERA_SHAKE_LONG (20.0f * (1.0f / 30.0f))
#define CAMERA_SHAKE_SHORT (2.0f * (1.0f / 30.0f))

typedef struct camera_t {
	vec3_t position;
	vec3_t velocity;
	vec3_t angle;
	vec3_t angular_velocity;
	vec3_t last_position;
	vec3_t real_velocity;
	section_t *section;
	bool has_initial_section;
	float update_timer;
	void (*update_func)(struct camera_t *, ship_t *, droid_t *);
	vec2_t shake;
	float shake_timer;
} camera_t;

void camera_init(camera_t *camera, section_t *section);
vec3_t camera_forward(camera_t *camera);
void camera_update(camera_t *camera, ship_t *ship, droid_t *droid);
void camera_update_race_external(camera_t *, ship_t *camShip, droid_t *);
void camera_update_race_internal(camera_t *, ship_t *camShip, droid_t *);
void camera_update_race_intro(camera_t *, ship_t *camShip, droid_t *);
void camera_update_attract_circle(camera_t *, ship_t *camShip, droid_t *);
void camera_update_attract_internal(camera_t *, ship_t *camShip, droid_t *);
void camera_update_static_follow(camera_t *, ship_t *camShip, droid_t *);
void camera_update_attract_random(camera_t *, ship_t *camShip, droid_t *);
void camera_update_rescue(camera_t *, ship_t *camShip, droid_t *);
void camera_set_shake(camera_t *, float duration);
void camera_update_shake(camera_t *);

#endif
