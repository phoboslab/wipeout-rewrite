#include "../utils.h"
#include "../types.h"
#include "../system.h"

#include "track.h"
#include "ship.h"
#include "droid.h"
#include "camera.h"
#include "game.h"

void camera_init(camera_t *camera, section_t *section) {
	camera->section = section;
	for (int i = 0; i < 10; i++) {
		camera->section = camera->section->next;
	}

	camera->position = camera->section->center;
	camera->velocity = vec3(0, 0, 0);
	camera->angle = vec3(0, 0, 0);
	camera->angular_velocity = vec3(0, 0, 0);
	camera->has_initial_section = false;
}

vec3_t camera_forward(camera_t *camera) {
	mat4_t rotation_matrix;
	mat4_set_yaw_pitch_roll(&rotation_matrix, camera->angle);
	return rotation_matrix.basis.forward.vec3;
}

static void camera_look_at_point(camera_t *camera, vec3_t point) {
	vec3_t target = vec3_sub(point, camera->position);
	float height = vec3_len(vec3_mul(target, vec3(1,0,1)));

	camera->angle = vec3(-atan2f(target.y, height), -atan2f(target.x, target.z), 0);
}

void camera_update(camera_t *camera, ship_t *ship, droid_t *droid) {
	camera->last_position = camera->position;
	camera->update_timer -= system_tick();

	(camera->update_func)(camera, ship, droid);

	camera->real_velocity = vec3_mulf(vec3_sub(camera->position, camera->last_position), 1.0/system_tick());
	camera_update_shake(camera);
}

void camera_update_race_external(camera_t *camera, ship_t *ship, droid_t *droid) {
	vec3_t pos = vec3_transform(vec3(0,0,-1024), &ship->mat);
	pos.y -= 200;

	camera->section = track_nearest_section(pos, vec3(1,1,1), camera->section, NULL);
	section_t *next = camera->section->next;

	vec3_t target = vec3_project_to_ray(pos, next->center, camera->section->center);

	vec3_t diff_from_center = vec3_sub(pos, target);
	vec3_t acc = diff_from_center;
	acc.y += vec3_len(diff_from_center) * 0.5;

	camera->velocity = vec3_sub(camera->velocity, vec3_mulf(acc, 0.015625 * 30 * system_tick()));
	camera->velocity = vec3_sub(camera->velocity, vec3_mulf(camera->velocity, 0.125 * 30 * system_tick()));
	pos = vec3_add(pos, camera->velocity);

	camera->position = pos;
	camera->angle = vec3_mul(ship->angle, vec3(1,1,0));
}

void camera_update_race_internal(camera_t *camera, ship_t *ship, droid_t *droid) {
	camera->section = ship->section;
	camera->position = ship_cockpit(ship);
	camera->angle = vec3_mul(ship->angle, vec3(1,1,save.internal_roll));
}

void camera_update_race_intro(camera_t *camera, ship_t *ship, droid_t *droid) {
	// Set to final position
	vec3_t pos = vec3_transform(vec3(0,0,-1024), &ship->mat);
	pos.y -= 200;

	camera->position = vec3_add(pos, vec3(
		sinf(( (ship->update_timer - UPDATE_TIME_RACE_VIEW) * 30 * 3.0 * M_PI * 2) / 4096.0) * 4096,
		-((2 * (ship->update_timer - UPDATE_TIME_RACE_VIEW) * 30)),
		sinf(( (ship->update_timer - UPDATE_TIME_RACE_VIEW) * 30 * 3.0 * M_PI * 2) / 4096.0) * 4096
	));

	if (!camera->has_initial_section) {
		camera->section = ship->section;
		camera->has_initial_section = true;
	}
	else {
		camera->section = track_nearest_section(camera->position, vec3(1,1,1), camera->section, NULL);
	}

	camera_look_at_point(camera, ship->position);
	camera->angle.x = ship->angle.x * 0.5f;

	if (ship->update_timer <= UPDATE_TIME_RACE_VIEW) {
		flags_add(ship->flags, SHIP_VIEW_INTERNAL);
		camera->update_func = camera_update_race_internal;
	}
}

void camera_update_rescue(camera_t *camera, ship_t *ship, droid_t *droid) {
	camera->position = vec3_add(camera->section->center, vec3(300, -1500, 300));
	camera_look_at_point(camera, droid->position);
}

void camera_update_attract_circle(camera_t *camera, ship_t *ship, droid_t *droid) {
	if (camera->update_timer <= 0) {
		camera->update_func = camera_update_attract_random;
	}
	// FIXME: not exactly sure what I'm doing here. The PSX version behaves
	// differently.
	camera->section = ship->section;

	camera->position = vec3_add(ship->position, vec3(
		sinf(ship->angle.y) * 512,
		((ship->angle.x * 512 / (M_PI * 2)) - 200),
		-cosf(ship->angle.y) * 512
	));
	camera->position = vec3_add(camera->position, vec3(
		sinf(camera->update_timer * 0.25) * 512,
		-400,
		cosf(camera->update_timer * 0.25) * 512
	));
	camera->position = vec3_add(camera->position, vec3_mulf(ship->mat.basis.down.vec3, 256));
	
	camera_look_at_point(camera, ship->position);
}

void camera_update_attract_internal(camera_t *camera, ship_t *ship, droid_t *droid) {
	if (camera->update_timer <= 0) {
		camera->update_func = camera_update_attract_random;
	}

	camera->section = ship->section;
	camera->position = ship_cockpit(ship);
	camera->angle = vec3_mul(ship->angle, vec3(1,1,0)); // No roll
}

void camera_update_static_follow(camera_t *camera, ship_t *ship, droid_t *droid) {
	if (camera->update_timer <= 0) {
		camera->update_func = camera_update_attract_random;
	}

	camera_look_at_point(camera, ship->position);
}

void camera_update_attract_static_follow(camera_t *camera, ship_t *ship, droid_t *droid) {
	section_t *section = ship->section->next;
	for (int i = 0; i < 10; i++) {
		section = section->next;
	}

	camera->section = section;
	camera->position = section->center;
	camera->position.y -= 500;
	camera->update_func = camera_update_static_follow;
	(camera->update_func)(camera, ship, droid);
}

void (* const attract_options[3])(camera_t *camera, ship_t *ship, droid_t *droid) = {
	camera_update_attract_static_follow,
	camera_update_attract_circle,
	camera_update_attract_internal,
};

void camera_update_attract_random(camera_t *camera, ship_t *ship, droid_t *droid) {
	flags_rm(ship->flags, SHIP_VIEW_INTERNAL);
	camera->update_timer = 5;
	camera->update_func = attract_options[rand() % len(attract_options)];

	(camera->update_func)(camera, ship, droid);
}

void camera_set_shake(camera_t *camera, float duration) {
	camera->shake_timer = duration;
}

void camera_update_shake(camera_t *camera) {
	if (camera->shake_timer > 0.0f) {
		float s = 0.25 * save.screen_shake * camera->shake_timer;
		camera->shake = vec2(rand_float(-s, s), rand_float(-s, s));
		camera->shake_timer -= system_tick();
	}
	else {
		camera->shake = vec2(0.0f, 0.0f);
		camera->shake_timer = 0.0f;
	}
}
