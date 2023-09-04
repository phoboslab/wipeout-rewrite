#include "../mem.h"
#include "../utils.h"
#include "../system.h"

#include "track.h"
#include "ship.h"
#include "weapon.h"
#include "object.h"
#include "game.h"
#include "image.h"
#include "particle.h"

extern int32_t ctrlNeedTargetIcon;
extern int ctrlnearShip;
int16_t Shielded = 0;

typedef struct weapon_t {
	float timer;
	ship_t *owner;
	ship_t *target;
	section_t *section;
	Object *model;
	bool active;

	int16_t trail_particle;
	int16_t track_hit_particle;
	int16_t ship_hit_particle;
	float trail_spawn_timer;

	int16_t type;
	vec3_t acceleration;
	vec3_t velocity;
	vec3_t position;
	vec3_t angle;
	float drag;

	void (*update_func)(struct weapon_t *);
} weapon_t;


weapon_t *weapons;
int weapons_active = 0;

struct {
	uint16_t reticle;
	Object *rocket;
	Object *mine;
	Object *missile;
	Object *shield;
	Object *shield_internal;
	Object *ebolt;
} weapon_assets;

void weapon_update_wait_for_delay(weapon_t *self);

void weapon_fire_mine(ship_t *ship);
void weapon_update_mine_wait_for_release(weapon_t *self);
void weapon_update_mine(weapon_t *self);
void weapon_update_mine_lights(weapon_t *self, int index);

void weapon_fire_missile(ship_t *ship);
void weapon_update_missile(weapon_t *self);

void weapon_fire_rocket(ship_t *ship);
void weapon_update_rocket(weapon_t *self);

void weapon_fire_ebolt(ship_t *ship);
void weapon_update_ebolt(weapon_t *self);

void weapon_fire_shield(ship_t *ship);
void weapon_update_shield(weapon_t *self);

void weapon_fire_turbo(ship_t *ship);

void invert_shield_polys(Object *shield);

void weapons_load(void) {
	weapons = mem_bump(sizeof(weapon_t) * WEAPONS_MAX);
	weapon_assets.reticle = image_get_texture("wipeout/textures/target2.tim");

	texture_list_t weapon_textures = image_get_compressed_textures("wipeout/common/mine.cmp");
	weapon_assets.rocket = objects_load("wipeout/common/rock.prm", weapon_textures);
	weapon_assets.mine = objects_load("wipeout/common/mine.prm", weapon_textures);
	weapon_assets.missile = objects_load("wipeout/common/miss.prm", weapon_textures);
	weapon_assets.shield = objects_load("wipeout/common/shld.prm", weapon_textures);
	weapon_assets.shield_internal = objects_load("wipeout/common/shld.prm", weapon_textures);
	weapon_assets.ebolt = objects_load("wipeout/common/ebolt.prm", weapon_textures);

	// Invert shield polys for internal view
	Prm poly = {.primitive = weapon_assets.shield_internal->primitives};
	int primitives_len = weapon_assets.shield_internal->primitives_len;
	for (int k = 0; k < primitives_len; k++) {
		switch (poly.primitive->type) {
		case PRM_TYPE_G3 :
			swap(poly.g3->coords[0], poly.g3->coords[2]);
			poly.g3 += 1;
			break;

		case PRM_TYPE_G4 :
			swap(poly.g4->coords[0], poly.g4->coords[3]);
			poly.g4 += 1;
			break;
		}
	}

	weapons_init();
}

void weapons_init(void) {
	weapons_active = 0;
}

weapon_t *weapon_init(ship_t *ship) {
	if (weapons_active == WEAPONS_MAX) {
		return NULL;
	}

	weapon_t *weapon = &weapons[weapons_active++];
	weapon->timer = 0;
	weapon->owner = ship;
	weapon->section = ship->section;
	weapon->position = ship->position;
	weapon->angle = ship->angle;	
	weapon->acceleration = vec3(0, 0, 0);
	weapon->velocity = vec3(0, 0, 0);
	weapon->acceleration = vec3(0, 0, 0);
	weapon->target = NULL;
	weapon->model = NULL;
	weapon->active = true;
	weapon->trail_particle = PARTICLE_TYPE_NONE;
	weapon->track_hit_particle = PARTICLE_TYPE_NONE;
	weapon->ship_hit_particle = PARTICLE_TYPE_NONE;
	weapon->trail_spawn_timer = 0;
	weapon->drag = 0;
	return weapon;
}

void weapons_fire(ship_t *ship, int weapon_type) {
	switch (weapon_type) {
		case WEAPON_TYPE_MINE:      weapon_fire_mine(ship); break;
		case WEAPON_TYPE_MISSILE:   weapon_fire_missile(ship); break;
		case WEAPON_TYPE_ROCKET:    weapon_fire_rocket(ship); break;
		case WEAPON_TYPE_EBOLT:     weapon_fire_ebolt(ship); break;
		case WEAPON_TYPE_SHIELD:    weapon_fire_shield(ship); break;
		case WEAPON_TYPE_TURBO:     weapon_fire_turbo(ship); break;
		default: die("Inavlid weapon type %d", weapon_type);
	}
	ship->weapon_type = WEAPON_TYPE_NONE;
}

void weapons_fire_delayed(ship_t *ship, int weapon_type) {
	weapon_t *weapon = weapon_init(ship);
	if (!weapon) {
		return;
	}
	weapon->type = weapon_type;
	weapon->timer = WEAPON_AI_DELAY;
	weapon->update_func = weapon_update_wait_for_delay;
}

bool weapon_collides_with_track(weapon_t *self);

void weapons_update(void) {
	for (int i = 0; i < weapons_active; i++) {
		weapon_t *weapon = &weapons[i];
		
		weapon->timer -= system_tick();
		(weapon->update_func)(weapon);

		// Handle projectiles
		if (weapon->acceleration.x != 0 || weapon->acceleration.z != 0) {
			weapon->velocity = vec3_add(weapon->velocity, vec3_mulf(weapon->acceleration, 30 * system_tick()));
			weapon->velocity = vec3_sub(weapon->velocity, vec3_mulf(weapon->velocity, weapon->drag * 30 * system_tick()));
			weapon->position = vec3_add(weapon->position, vec3_mulf(weapon->velocity, 30 * system_tick()));

			// Move along track normal
			track_face_t *face = track_section_get_base_face(weapon->section);
			vec3_t face_point = face->tris[0].vertices[0].pos;
			vec3_t face_normal = face->normal;
			float height = vec3_distance_to_plane(weapon->position, face_point, face_normal);

			if (height < 2000) {
				weapon->position = vec3_add(weapon->position, vec3_mulf(face_normal, (200 - height) * 30 * system_tick()));
			}

			// Trail
			if (weapon->trail_particle != PARTICLE_TYPE_NONE) {
				weapon->trail_spawn_timer += system_tick();
				while (weapon->trail_spawn_timer > 0) {
					vec3_t pos = vec3_sub(weapon->position, vec3_mulf(weapon->velocity, 30 * system_tick() * weapon->trail_spawn_timer));
					vec3_t velocity = vec3(rand_float(-128, 128), rand_float(-128, 128), rand_float(-128, 128));
					particles_spawn(pos, weapon->trail_particle, velocity, 128);
					weapon->trail_spawn_timer -= WEAPON_PARTICLE_SPAWN_RATE;
				}
			}

			// Track collision
			weapon->section = track_nearest_section(weapon->position, weapon->section, NULL);
			if (weapon_collides_with_track(weapon)) {
				for (int p = 0; p < 32; p++) {
					vec3_t velocity = vec3(rand_float(-512, 512), rand_float(-512, 512), rand_float(-512, 512));
					particles_spawn(weapon->position, weapon->track_hit_particle, velocity, 256);
				}
				sfx_play_at(SFX_EXPLOSION_2, weapon->position, vec3(0,0,0), 1);
				weapon->active = false;
			}
		}

		// If this weapon is released, we have to rewind one step
		if (!weapon->active) {
			weapons[i--] = weapons[--weapons_active];
			continue;
		}
	}
}

void weapons_draw(void) {
	mat4_t mat = mat4_identity();
	for (int i = 0; i < weapons_active; i++) {
		weapon_t *weapon = &weapons[i];
		if (weapon->model) {
			mat4_set_translation(&mat, weapon->position);
			mat4_set_yaw_pitch_roll(&mat, weapon->angle);
			if (weapon->model == weapon_assets.mine) {
				weapon_update_mine_lights(weapon, i);
			}
			object_draw(weapon->model, &mat);
		}
	}
}



void weapon_set_trajectory(weapon_t *self) {
	ship_t *ship = self->owner;
	track_face_t *face = track_section_get_base_face(ship->section);

	vec3_t face_point = face->tris[0].vertices[0].pos;
	vec3_t target = vec3_add(ship->position, vec3_mulf(ship->dir_forward, 64));
	float target_height = vec3_distance_to_plane(target, face_point, face->normal);
	float ship_height = vec3_distance_to_plane(target, face_point, face->normal);

	float nudge = target_height * 0.95 - ship_height;

	self->acceleration = vec3_sub(vec3_sub(target, vec3_mulf(face->normal, nudge)), ship->position);
	self->velocity = vec3_mulf(ship->velocity, 0.015625);
	self->angle = ship->angle;
}

void weapon_follow_target(weapon_t *self) {
	vec3_t angular_velocity = vec3(0, 0, 0);
	if (self->target) {
		vec3_t dir = vec3_mulf(vec3_sub(self->target->position, self->position), 0.125 * 30 * system_tick());
		float height = sqrt(dir.x * dir.x + dir.z * dir.z);
		angular_velocity.y = -atan2(dir.x, dir.z) - self->angle.y;
		angular_velocity.x = -atan2(dir.y, height) - self->angle.x;
	}

	angular_velocity = vec3_wrap_angle(angular_velocity);
	self->angle = vec3_add(self->angle, vec3_mulf(angular_velocity, 30 * system_tick() * 0.25));
	self->angle = vec3_wrap_angle(self->angle);

	self->acceleration.x = -sin(self->angle.y) * cos(self->angle.x) * 256;
	self->acceleration.y = -sin(self->angle.x) * 256;
	self->acceleration.z = cos(self->angle.y) * cos(self->angle.x) * 256;
}

ship_t *weapon_collides_with_ship(weapon_t *self) {
	for (int i = 0; i < NUM_PILOTS; i++) {
		ship_t *ship = &g.ships[i];
		if (ship == self->owner) {
			continue;
		}

		float distance = vec3_len(vec3_sub(ship->position, self->position));
		if (distance < 512) {
			for (int p = 0; p < 32; p++) {
				vec3_t velocity = vec3(rand_float(-512, 512), rand_float(-512, 512), rand_float(-512, 512));
				velocity = vec3_add(velocity, vec3_mulf(ship->velocity, 0.25));
				particles_spawn(self->position, self->ship_hit_particle, velocity, 256);
			}
			return ship;
		}
	}

	return NULL;
}


bool weapon_collides_with_track(weapon_t *self) {
	if (flags_is(self->section->flags, SECTION_JUMP)) {
		return false;
	}

	track_face_t *face = g.track.faces + self->section->face_start;
	for (int i = 0; i < self->section->face_count; i++) {
		vec3_t face_point = face->tris[0].vertices[0].pos;
		float distance = vec3_distance_to_plane(self->position, face_point, face->normal);
		if (distance < 0) {
			return true;
		}
		face++;
	}

	return false;
}

void weapon_update_wait_for_delay(weapon_t *self) {
	if (self->timer <= 0) {
		weapons_fire(self->owner, self->type);
		self->active = false;
	}
}


void weapon_fire_mine(ship_t *ship) {
	float timer = 0;
	for (int i = 0; i < WEAPON_MINE_COUNT; i++) {
		weapon_t *self = weapon_init(ship);
		if (!self) {
			return;
		}
		timer += WEAPON_MINE_RELEASE_RATE;
		self->timer = timer;
		self->update_func = weapon_update_mine_wait_for_release;
	}
}



void weapon_update_mine_wait_for_release(weapon_t *self) {
	if (self->timer <= 0) {
		self->timer = WEAPON_MINE_DURATION;
		self->update_func = weapon_update_mine;
		self->model = weapon_assets.mine;
		self->position = self->owner->position;
		self->angle.y = rand_float(0, M_PI * 2);

		self->trail_particle = PARTICLE_TYPE_NONE;
		self->track_hit_particle = PARTICLE_TYPE_NONE;
		self->ship_hit_particle = PARTICLE_TYPE_FIRE;

		if (self->owner->pilot == g.pilot) {
			sfx_play(SFX_MINE_DROP);
		}
	}
}

void weapon_update_mine_lights(weapon_t *self, int index) {
	Prm prm = {.primitive = self->model->primitives};

	uint8_t r = sin(system_cycle_time() * M_PI * 2 + index * 0.66) * 128 + 128;
	for (int i = 0; i < 8; i++) {
		switch (prm.primitive->type) {
		case PRM_TYPE_GT3:
			prm.gt3->color[0].r = 230;
			prm.gt3->color[1].r = r;
			prm.gt3->color[2].r = r;
			prm.gt3->color[0].g = 0;
			prm.gt3->color[1].g = 0x40;
			prm.gt3->color[2].g = 0x40;
			prm.gt3->color[0].b = 0;
			prm.gt3->color[1].b = 0;
			prm.gt3->color[2].b = 0;
			prm.gt3 += 1;
			break;
		}
	}
}

void weapon_update_mine(weapon_t *self) {
	if (self->timer <= 0) {
		self->active = false;
		return;
	}

	// TODO: oscilate perpendicular to track!?
	self->angle.y += system_tick();

	ship_t *ship = weapon_collides_with_ship(self);
	if (ship) {
		sfx_play_at(SFX_EXPLOSION_1, self->position, vec3(0,0,0), 1);
		self->active = false;
		if (flags_not(ship->flags, SHIP_SHIELDED)) {
			if (ship->pilot == g.pilot) {
				ship->velocity = vec3_sub(ship->velocity, vec3_mulf(ship->velocity, 0.125));
				// SetShake(20); // FIXME
			}
			else {
				ship->speed = ship->speed * 0.125;
			}
		}
	}	
}


void weapon_fire_missile(ship_t *ship) {
	weapon_t *self = weapon_init(ship);
	if (!self) {
		return;
	}

	self->timer = WEAPON_MISSILE_DURATION;
	self->model = weapon_assets.missile;
	self->update_func = weapon_update_missile;
	self->trail_particle = PARTICLE_TYPE_SMOKE;
	self->track_hit_particle = PARTICLE_TYPE_FIRE_WHITE;
	self->ship_hit_particle = PARTICLE_TYPE_FIRE;
	self->target = ship->weapon_target;
	self->drag = 0.25;
	weapon_set_trajectory(self);

	if (self->owner->pilot == g.pilot) {
		sfx_play(SFX_MISSILE_FIRE);
	}
}

void weapon_update_missile(weapon_t *self) {
	if (self->timer <= 0) {
		self->active = false;
		return;
	}

	weapon_follow_target(self);

	// Collision with other ships
	ship_t *ship = weapon_collides_with_ship(self);
	if (ship) {
		sfx_play_at(SFX_EXPLOSION_1, self->position, vec3(0,0,0), 1);
		self->active = false;

		if (flags_not(ship->flags, SHIP_SHIELDED)) {
			if (ship->pilot == g.pilot) {
				ship->velocity = vec3_sub(ship->velocity, vec3_mulf(ship->velocity, 0.75));
				ship->angular_velocity.z += rand_float(-0.1, 0.1);
				ship->turn_rate_from_hit = rand_float(-0.1, 0.1);
				// SetShake(20);  // FIXME
			}
			else {
				ship->speed = ship->speed * 0.03125;
				ship->angular_velocity.z += 10 * M_PI;
				ship->turn_rate_from_hit = rand_float(-M_PI, M_PI);
			}
		}
	}
}

void weapon_fire_rocket(ship_t *ship) {
	weapon_t *self = weapon_init(ship);
	if (!self) {
		return;
	}

	self->timer = WEAPON_ROCKET_DURATION;
	self->model = weapon_assets.rocket;
	self->update_func = weapon_update_rocket;
	self->trail_particle = PARTICLE_TYPE_SMOKE;
	self->track_hit_particle = PARTICLE_TYPE_FIRE_WHITE;
	self->ship_hit_particle = PARTICLE_TYPE_FIRE;
	self->drag = 0.03125;
	weapon_set_trajectory(self);

	if (self->owner->pilot == g.pilot) {
		sfx_play(SFX_MISSILE_FIRE);
	}
}

void weapon_update_rocket(weapon_t *self) {
	if (self->timer <= 0) {
		self->active = false;
		return;
	}

	// Collision with other ships
	ship_t *ship = weapon_collides_with_ship(self);
	if (ship) {
		sfx_play_at(SFX_EXPLOSION_1, self->position, vec3(0,0,0), 1);
		self->active = false;

		if (flags_not(ship->flags, SHIP_SHIELDED)) {
			if (ship->pilot == g.pilot) {
				ship->velocity = vec3_sub(ship->velocity, vec3_mulf(ship->velocity, 0.75));
				ship->angular_velocity.z += rand_float(-0.1, 0.1);;
				ship->turn_rate_from_hit = rand_float(-0.1, 0.1);;
				// SetShake(20);  // FIXME
			}
			else {
				ship->speed = ship->speed * 0.03125;
				ship->angular_velocity.z += 10 * M_PI;
				ship->turn_rate_from_hit = rand_float(-M_PI, M_PI);
			}
		}
	}
}


void weapon_fire_ebolt(ship_t *ship) {
	weapon_t *self = weapon_init(ship);
	if (!self) {
		return;
	}

	self->timer = WEAPON_EBOLT_DURATION;
	self->model = weapon_assets.ebolt;
	self->update_func = weapon_update_ebolt;
	self->trail_particle = PARTICLE_TYPE_EBOLT;
	self->track_hit_particle = PARTICLE_TYPE_EBOLT;
	self->ship_hit_particle = PARTICLE_TYPE_GREENY;
	self->target = ship->weapon_target;
	self->drag = 0.25;
	weapon_set_trajectory(self);

	if (self->owner->pilot == g.pilot) {
		sfx_play(SFX_EBOLT);
	}
}

void weapon_update_ebolt(weapon_t *self) {
	if (self->timer <= 0) {
		self->active = false;
		return;
	}

	weapon_follow_target(self);

	// Collision with other ships
	ship_t *ship = weapon_collides_with_ship(self);
	if (ship) {
		sfx_play_at(SFX_EXPLOSION_1, self->position, vec3(0,0,0), 1);
		self->active = false;

		if (flags_not(ship->flags, SHIP_SHIELDED)) {
			flags_add(ship->flags, SHIP_ELECTROED);
			ship->ebolt_timer = WEAPON_EBOLT_DURATION;
		}
	}
}

void weapon_fire_shield(ship_t *ship) {
	weapon_t *self = weapon_init(ship);
	if (!self) {
		return;
	}

	self->timer = WEAPON_SHIELD_DURATION;
	self->model = weapon_assets.shield;
	self->update_func = weapon_update_shield;

	flags_add(self->owner->flags, SHIP_SHIELDED);
}

void weapon_update_shield(weapon_t *self) {
	if (self->timer <= 0) {
		self->active = false;
		flags_rm(self->owner->flags, SHIP_SHIELDED);
		return;
	}


	if (flags_is(self->owner->flags, SHIP_VIEW_INTERNAL)) {
		self->position = ship_cockpit(self->owner);
		self->model = weapon_assets.shield_internal;
	}
	else {
		self->position = self->owner->position;
		self->model = weapon_assets.shield;
	}
	self->angle = self->owner->angle;


	Prm poly = {.primitive = self->model->primitives};
	int primitives_len = self->model->primitives_len;
	uint8_t col0, col1, col2, col3;
	int16_t *coords;
	uint8_t shield_alpha = 48;

	// FIXME: this looks kinda close to the PSX original!?
	float color_timer = self->timer * 0.05;
	for (int k = 0; k < primitives_len; k++) {
		switch (poly.primitive->type) {
		case PRM_TYPE_G3 :
			coords = poly.g3->coords;

			col0 = sin(color_timer * coords[0]) * 127 + 128;
			col1 = sin(color_timer * coords[1]) * 127 + 128;
			col2 = sin(color_timer * coords[2]) * 127 + 128;

			poly.g3->color[0].r = col0;
			poly.g3->color[0].g = col0;
			poly.g3->color[0].b = 255;
			poly.g3->color[0].a = shield_alpha;

			poly.g3->color[1].r = col1;
			poly.g3->color[1].g = col1;
			poly.g3->color[1].b = 255;
			poly.g3->color[1].a = shield_alpha;

			poly.g3->color[2].r = col2;
			poly.g3->color[2].g = col2;
			poly.g3->color[2].b = 255;
			poly.g3->color[2].a = shield_alpha;
			poly.g3 += 1;
			break;

		case PRM_TYPE_G4 :
			coords = poly.g4->coords;

			col0 = sin(color_timer * coords[0]) * 127 + 128;
			col1 = sin(color_timer * coords[1]) * 127 + 128;
			col2 = sin(color_timer * coords[2]) * 127 + 128;
			col3 = sin(color_timer * coords[3]) * 127 + 128;

			poly.g4->color[0].r = col0;
			poly.g4->color[0].g = col0;
			poly.g4->color[0].b = 255;
			poly.g4->color[0].a = shield_alpha;

			poly.g4->color[1].r = col1;
			poly.g4->color[1].g = col1;
			poly.g4->color[1].b = 255;
			poly.g4->color[1].a = shield_alpha;

			poly.g4->color[2].r = col2;
			poly.g4->color[2].g = col2;
			poly.g4->color[2].b = 255;
			poly.g4->color[2].a = shield_alpha;

			poly.g4->color[3].r = col3;
			poly.g4->color[3].g = col3;
			poly.g4->color[3].b = 255;
			poly.g4->color[3].a = shield_alpha;
			poly.g4 += 1;
			break;
		}
	}
}


void weapon_fire_turbo(ship_t *ship) {
	ship->velocity = vec3_add(ship->velocity, vec3_mulf(ship->dir_forward, 39321)); // unitVecNose.vx) << 3) * FR60) / 50
	
	if (ship->pilot == g.pilot) {
		sfx_t *sfx = sfx_play(SFX_MISSILE_FIRE);
		sfx->pitch = 0.25;
	}
}

int weapon_get_random_type(int type_class) {
	if (type_class == WEAPON_CLASS_ANY) {
		int index = rand_int(0, 65);
		if (index < 17) {
			return WEAPON_TYPE_ROCKET;
		}
		else if (index < 35) {
			return WEAPON_TYPE_MINE;
		}
		else if (index < 45) {
			return WEAPON_TYPE_SHIELD;
		}
		else if (index < 53) {
			return WEAPON_TYPE_MISSILE;
		}
		else if (index < 59) {
			return WEAPON_TYPE_TURBO;
		}
		else {
			return WEAPON_TYPE_EBOLT;
		}
	}
	else if (type_class == WEAPON_CLASS_PROJECTILE) { 
		int index = rand_int(0, 60);
		if (index < 27) {
			return WEAPON_TYPE_ROCKET;
		}
		else if (index < 40) {
			return WEAPON_TYPE_MISSILE;
		}
		else if (index < 50) {
			return WEAPON_TYPE_TURBO;
		}
		else {
			return WEAPON_TYPE_EBOLT;
		}
	}
	else {
		die("Unknown WEAPON_CLASS_ %d", type_class);
	}
}

