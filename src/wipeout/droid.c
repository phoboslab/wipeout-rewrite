#include "../types.h"
#include "../mem.h"
#include "../system.h"
#include "../utils.h"

#include "object.h"
#include "track.h"
#include "ship.h"
#include "weapon.h"
#include "hud.h"
#include "droid.h"
#include "camera.h"
#include "image.h"
#include "scene.h"
#include "object.h"
#include "game.h"

static Object *droid_model;

void droid_load() {
	texture_list_t droid_textures = image_get_compressed_textures("wipeout/common/rescu.cmp");
	droid_model = objects_load("wipeout/common/rescu.prm", droid_textures);
}

void droid_init(droid_t *droid, ship_t *ship) {
	droid->section = g.track.sections;

	while (flags_not(droid->section->flags, SECTION_JUMP)) {
		droid->section = droid->section->next;
	}

	droid->position = vec3_add(ship->position, vec3(0, -200, 0));
	droid->velocity = vec3(0, 0, 0);
	droid->acceleration = vec3(0, 0, 0);
	droid->angle = vec3(0, 0, 0);
	droid->angular_velocity = vec3(0, 0, 0);
	droid->update_timer = DROID_UPDATE_TIME_INITIAL;
	droid->mat = mat4_identity();

	droid->cycle_timer = 0;
	droid->update_func = droid_update_intro;

	droid->sfx_tractor = sfx_reserve_loop(SFX_TRACTOR);
	flags_rm(droid->sfx_tractor->flags, SFX_PLAY);
}

void droid_draw(droid_t *droid) {
	droid->cycle_timer += system_tick() * M_PI * 2;

	Prm prm = {.primitive = droid_model->primitives};
	int rf = sin(droid->cycle_timer) * 127 + 128;
	int gf = sin(droid->cycle_timer + 0.2) * 127 + 128;
	int bf = sin(droid->cycle_timer * 0.5 + 0.1) * 127 + 128;

	int r, g, b;

	for (int i = 0; i < 11; i++) {
		if (i < 2) {
			r = 40;
			g = gf;
			b = 40;
		}
		else if (i < 6) {
			r = bf >> 1;
			b = bf;
			g = bf >> 1;
		}
		else {
			r = rf;
			b = 40;
			g = 40;
		}

		switch (prm.f3->type) {
			case PRM_TYPE_GT3:
				prm.gt3->color[0].r = r;
				prm.gt3->color[0].g = g;
				prm.gt3->color[0].b = b;

				prm.gt3->color[1].r = r;
				prm.gt3->color[1].g = g;
				prm.gt3->color[1].b = b;

				prm.gt3->color[2].r = r;
				prm.gt3->color[2].g = g;
				prm.gt3->color[2].b = b;
				prm.gt3++;
				break;

			case PRM_TYPE_GT4:
				prm.gt4->color[0].r = r;
				prm.gt4->color[0].g = g;
				prm.gt4->color[0].b = b;

				prm.gt4->color[1].r = r;
				prm.gt4->color[1].g = g;
				prm.gt4->color[1].b = b;

				prm.gt4->color[2].r = r;
				prm.gt4->color[2].g = g;
				prm.gt4->color[2].b = b;

				prm.gt4->color[3].r = 40;
				prm.gt4->color[3].g = 40;
				prm.gt4->color[3].b = 40;
				prm.gt4++;
				break;
		}
	}

	mat4_set_translation(&droid->mat, droid->position);
	mat4_set_yaw_pitch_roll(&droid->mat, droid->angle);
	object_draw(droid_model, &droid->mat);
}

void droid_update(droid_t *droid, ship_t *ship) {
	(droid->update_func)(droid, ship);

	droid->velocity = vec3_add(droid->velocity, vec3_mulf(droid->acceleration, 30 * system_tick()));
	droid->velocity = vec3_sub(droid->velocity, vec3_mulf(droid->velocity, 0.125 * 30 * system_tick()));
	droid->position = vec3_add(droid->position, vec3_mulf(droid->velocity, 0.015625 * 30 * system_tick()));
	droid->angle = vec3_add(droid->angle, vec3_mulf(droid->angular_velocity, system_tick()));
	droid->angle = vec3_wrap_angle(droid->angle);
	
	if (flags_is(droid->sfx_tractor->flags, SFX_PLAY)) {
		sfx_set_position(droid->sfx_tractor, droid->position, droid->velocity, 0.5);
	}
}

void droid_update_intro(droid_t *droid, ship_t *ship) {
	droid->update_timer -= system_tick();

	if (droid->update_timer < DROID_UPDATE_TIME_INTRO_3) {
		droid->acceleration.x = (-sin(droid->angle.y) * cos(droid->angle.x)) * 0.25 * 4096.0;
		droid->acceleration.y = 0;
		droid->acceleration.z = (cos(droid->angle.y) * cos(droid->angle.x)) * 0.25 * 4096.0;
		droid->angular_velocity.y = 0;
	}

	else if (droid->update_timer < DROID_UPDATE_TIME_INTRO_2) {
		droid->acceleration.x = (-sin(droid->angle.y) * cos(droid->angle.x)) * 0.125 * 4096.0;
		droid->acceleration.y = -140;
		droid->acceleration.z = (cos(droid->angle.y) * cos(droid->angle.x)) * 0.125 * 4096.0;
		droid->angular_velocity.y = (-8.0 / 4096.0) * M_PI * 2 * 30;
	}

	else if (droid->update_timer < DROID_UPDATE_TIME_INTRO_1) {
		droid->acceleration.y -= 90 * system_tick();
		droid->angular_velocity.y = (8.0 / 4096.0) * M_PI * 2 * 30;
	}

	if (droid->update_timer <= 0) {
		droid->update_timer = DROID_UPDATE_TIME_INITIAL;
		droid->update_func = droid_update_idle;
		droid->position.x = droid->section->center.x;
		droid->position.y = -3000;
		droid->position.z = droid->section->center.z;
	}
}

void droid_update_idle(droid_t *droid, ship_t *ship) {
	section_t *next = droid->section->next;

	vec3_t target = vec3(
		(droid->section->center.x + next->center.x) * 0.5,
		droid->section->center.y - 3000,
		(droid->section->center.z + next->center.z) * 0.5
	);

	vec3_t target_vector = vec3_sub(target, droid->position);

	float target_heading = -atan2(target_vector.x, target_vector.z);
	float quickest_turn = target_heading - droid->angle.y;
	float turn;
	if (droid->angle.y < 0) {
		turn = target_heading - (droid->angle.y + M_PI*2);
	}
	else {
		turn = target_heading - (droid->angle.y - M_PI*2);
	}

	if (fabsf(turn) < fabsf(quickest_turn)) {
		droid->angular_velocity.y = turn * 30 / 64.0;
	}
	else {
		droid->angular_velocity.y = quickest_turn * 30.0 / 64.0;
	}

	droid->acceleration.x = (-sin(droid->angle.y) * cos(droid->angle.x)) * 0.125 * 4096;
	droid->acceleration.y = target_vector.y / 64.0;
	droid->acceleration.z = (cos(droid->angle.y) * cos(droid->angle.x)) * 0.125 * 4096;

	if (flags_is(ship->flags, SHIP_IN_RESCUE)) {
		flags_add(droid->sfx_tractor->flags, SFX_PLAY);

		droid->update_func = droid_update_rescue;
		droid->update_timer = DROID_UPDATE_TIME_INITIAL;

		g.camera.update_func = camera_update_rescue;
		flags_add(ship->flags, SHIP_VIEW_REMOTE);
		if (flags_is(ship->section->flags, SECTION_JUMP)) {
			g.camera.section = ship->section->next;
		}
		else {
			g.camera.section = ship->section;
		}

		// If droid is not nearby the rescue position teleport it in!
		if (droid->section != ship->section && droid->section != ship->section->prev) {
			droid->section = ship->section;
			section_t *next = droid->section->next;

			droid->position.x = (droid->section->center.x + next->center.x) * 0.5;
			droid->position.y = droid->section->center.y - 3000;
			droid->position.z = (droid->section->center.z + next->center.z) * 0.5;
		}
		flags_rm(ship->flags, SHIP_IN_TOW);
		droid->velocity = vec3(0,0,0);
		droid->acceleration = vec3(0,0,0);
	}

	// AdjustDirectionalNote(START_SIREN, 0, 0, (VECTOR){droid->position.x, droid->position.y, droid->position.z});
}

void droid_update_rescue(droid_t *droid, ship_t *ship) {
	droid->angular_velocity.y = 0;
	droid->angle.y = ship->angle.y;

	vec3_t target = vec3(ship->position.x, ship->position.y - 350, ship->position.z);
	vec3_t distance = vec3_sub(target, droid->position);


	if (flags_is(ship->flags, SHIP_IN_TOW)) {
		droid->velocity = vec3(0,0,0);
		droid->acceleration = vec3(0,0,0);
		droid->position = target;
	}
	else if (vec3_len(distance) < 8) {
		flags_add(ship->flags, SHIP_IN_TOW);
		droid->velocity = vec3(0,0,0);
		droid->acceleration = vec3(0,0,0);
		droid->position = target;
	}
	else {
		droid->velocity = vec3_mulf(distance, 16);	
	}


	// Are we done rescuing?
	if (flags_not(ship->flags, SHIP_IN_RESCUE)) {
		flags_rm(droid->sfx_tractor->flags, SFX_PLAY);
		droid->siren_started = false;
		droid->update_func = droid_update_idle;
		droid->update_timer = DROID_UPDATE_TIME_INITIAL;

		while (flags_not(droid->section->flags, SECTION_JUMP)) {
			droid->section = droid->section->prev;
		}
	}
}
