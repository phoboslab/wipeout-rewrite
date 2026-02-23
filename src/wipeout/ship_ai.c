#include "../mem.h"
#include "../input.h"
#include "../system.h"
#include "../utils.h"

#include "object.h"
#include "track.h"
#include "ship.h"
#include "weapon.h"
#include "hud.h"
#include "droid.h"
#include "camera.h"
#include "ship_ai.h"
#include "game.h"

vec3_t ship_ai_strat_hold_center(ship_t *self, track_face_t *face);
vec3_t ship_ai_strat_hold_right(ship_t *self, track_face_t *face);
vec3_t ship_ai_strat_hold_left(ship_t *self, track_face_t *face);
vec3_t ship_ai_strat_block(ship_t *self, track_face_t *face);
vec3_t ship_ai_strat_avoid(ship_t *self, track_face_t *face);
vec3_t ship_ai_strat_avoid_other(ship_t *self, track_face_t *face);
vec3_t ship_ai_strat_zig_zag(ship_t *self, track_face_t *face);

void ship_ai_update_intro(ship_t *self) {
	self->temp_target = self->position;
	self->update_func = ship_ai_update_intro_await_go;

	self->sfx_engine_thrust = sfx_reserve_loop(SFX_ENGINE_REMOTE);
	sfx_set_position(self->sfx_engine_thrust, self->position, self->velocity, 0.1);
}

void ship_ai_update_intro_await_go(ship_t *self) {
	self->position.y = self->temp_target.y + sin(self->update_timer * (80.0 + self->pilot * 3.0) * 30.0 * M_PI * 2.0 / 4096.0) * 32;

	self->update_timer -= system_tick();
	if (self->update_timer <= UPDATE_TIME_GO) {
		self->update_func = ship_ai_update_race;
	}
}

vec3_t ship_ai_strat_hold_left(ship_t*, track_face_t *face) {
	vec3_t fv1 = face->tris[0].vertices[1].pos;
	vec3_t fv2 = face->tris[0].vertices[0].pos;

	return vec3_mulf(vec3_sub(fv1, fv2), 0.5);
}

vec3_t ship_ai_strat_hold_right(ship_t*, track_face_t *face) {
	vec3_t fv1 = face->tris[0].vertices[0].pos;
	vec3_t fv2 = face->tris[0].vertices[1].pos;

	return vec3_mulf(vec3_sub(fv1, fv2), 0.5);
}


vec3_t ship_ai_strat_hold_center(ship_t*, track_face_t*) {
	return vec3(0, 0, 0);
}

vec3_t ship_ai_strat_block(ship_t *self, track_face_t *face) {
	if (flags_is(g.ships[g.pilot].flags, SHIP_LEFT_SIDE)) {
		return ship_ai_strat_hold_left(self, face);
	}
	else {
		return ship_ai_strat_hold_right(self, face);
	}

}

vec3_t ship_ai_strat_avoid(ship_t *self, track_face_t *face) {
	if (flags_is(g.ships[g.pilot].flags, SHIP_LEFT_SIDE)) {
		return ship_ai_strat_hold_right(self, face);
	}
	else {
		return ship_ai_strat_hold_left(self, face);
	}
}



vec3_t ship_ai_strat_avoid_other(ship_t *self, track_face_t *face) {
	int min_section_num = 100;
	ship_t *avoid_ship;

	for (int i = 0; i < NUM_PILOTS; i++) {
		if (i != self->pilot) {
			int section_diff = g.ships[i].total_section_num - self->total_section_num;
			if (min_section_num < section_diff) {
				min_section_num = section_diff;
				avoid_ship = &g.ships[i];
			}
		}
	}

	if (avoid_ship && min_section_num < 10 && min_section_num > -2) {
		if (flags_is(avoid_ship->flags, SHIP_LEFT_SIDE)) {
			return ship_ai_strat_hold_right(self, face);
		}
		else {
			return ship_ai_strat_hold_left(self, face);
		}
	}
	return vec3(0, 0, 0);
}



vec3_t ship_ai_strat_zig_zag(ship_t *self, track_face_t *face) {
	int update_count = (self->update_timer * 30)/50;
	if (update_count % 2) {
		return ship_ai_strat_hold_right(self, face);
	}
	else {
		return ship_ai_strat_hold_left(self, face);
	}
}


void ship_ai_update_race(ship_t *self) {
	vec3_t offset_vector = vec3(0, 0, 0);

	ship_t *player = &(g.ships[g.pilot]);

	if (self->ebolt_timer > 0) {
		self->ebolt_timer -= system_tick();
	}
	if (self->ebolt_timer <= 0) {
		flags_rm(self->flags, SHIP_ELECTROED);
	}

	int behind_speed = def.circuts[g.circut].settings[g.race_class].behind_speed;


	if (flags_not(self->flags, SHIP_FLYING)) {
		// Find First track base section
		track_face_t *face = track_section_get_base_face(self->section);

		int section_diff = self->total_section_num - player->total_section_num;

		flags_rm(self->flags, SHIP_JUST_IN_FRONT);

		if (self == player) {
			self->update_strat_func = ship_ai_strat_avoid_other;
			if (self->remote_thrust_max > self->speed) {
				self->speed += self->remote_thrust_mag * 30 * system_tick();
			}
		}
		else {
			// Make global DPA decisions , these will effect the craft in
			// relation to your race position

			// Accelerate remote ships away at start, start_accelerate_count set in
			// InitShipData and is an exponential progression

			if (self->start_accelerate_timer > 0) {
				self->start_accelerate_timer -= system_tick();
				self->update_timer = 0;
				self->update_strat_func = ship_ai_strat_avoid;
				if ((self->remote_thrust_max + 1200) > self->speed) {
					self->speed += (self->remote_thrust_mag + 150) * 30 * system_tick();
				}
			}


			// Ship has been left WELL BEHIND; set it to avoid
			// other ships and update its speed as normal

			else if (section_diff < -10) { // Ship behind, AVOID
				self->update_timer = 0;
				self->update_strat_func = ship_ai_strat_avoid;

				// If ship has been well passed, increase its speed to allow
				// it to make a challenge when the player fouls up

				if (((self->remote_thrust_max + behind_speed) > self->speed)) {
					self->speed += self->remote_thrust_mag * 30 * system_tick();
				}
			}


			// Ship is JUST AHEAD

			else if ((section_diff <= 4) && (section_diff > 0)) { // Ship close by, beware does not account for lapped opponents yet
				flags_add(self->flags, SHIP_JUST_IN_FRONT);

				if (self->update_timer <= 0) { // Make New Decision
					int chance = rand_int(0, 64); // 12

					self->update_timer = UPDATE_TIME_JUST_FRONT;
					if (self->fight_back) { // Ship wants to make life difficult
						if ((chance < 40) || (self->weapon_type == WEAPON_TYPE_NONE)) { // Ship will try to block you
							self->update_strat_func = ship_ai_strat_block;
						}
						else if ((chance >= 40) && (chance < 52)) {	// Ship will attempt to drop mines in your path
							self->update_strat_func = ship_ai_strat_block;
							if (flags_not(self->flags, SHIP_SHIELDED) && flags_is(self->flags, SHIP_RACING)) {
								sfx_play(SFX_VOICE_MINES);
								self->weapon_type = WEAPON_TYPE_MINE;
								weapons_fire_delayed(self, self->weapon_type);
							}
						}
						else if ((chance >= 52) && (chance < 64)) {	// Ship will raise its shield
							self->update_strat_func = ship_ai_strat_block;
							if (flags_not(self->flags, SHIP_SHIELDED)) {
								self->weapon_type = WEAPON_TYPE_SHIELD;
								weapons_fire(self, self->weapon_type);
							}
						}
					}
					else { // Let the first ships be easy to pass
						self->update_strat_func = ship_ai_strat_avoid;
					}
				}

				self->update_timer -= system_tick();

				if (flags_is(self->flags, SHIP_OVERTAKEN)) {
					// If ship has just overtaken, slow it down to a reasonable speed
					if ((self->remote_thrust_max + behind_speed) > self->speed) {
						self->speed += self->remote_thrust_mag * 30 * system_tick();
					}
				}
				else {
					// Increase the speed of any craft just in front slightly
					if (((self->remote_thrust_max + (behind_speed >> 1)) > self->speed)) {
						self->speed += self->remote_thrust_mag * 30 * system_tick();
					}
				}

			}

			
			// Ship is JUST BEHIND; we must decided if and how many times it 'should have a go back'

			else if ((section_diff >= -10) && (section_diff <= 0)) { // Ship just behind, MAKE DECISION
				if (self->update_timer <= 0) { // Make New Decision
					self->update_timer = UPDATE_TIME_JUST_BEHIND;

					if (self->fight_back) { // Ship wants you to say "Outside Now!"
						if (self->weapon_type == WEAPON_TYPE_NONE) {
							self->update_strat_func = ship_ai_strat_avoid;
							flags_add(self->flags, SHIP_OVERTAKEN);
						}
						else {
							int chance = rand_int(0, 64);

							if (chance < 48) {
								self->update_strat_func = ship_ai_strat_block;
							}
							else if ((chance >= 40) && (chance < 54)) {
								self->update_strat_func = ship_ai_strat_avoid;
								flags_rm(self->flags, SHIP_OVERTAKEN);
								if (flags_not(self->flags, SHIP_SHIELDED) && flags_is(self->flags, SHIP_RACING)) {
									sfx_play(SFX_VOICE_ROCKETS);
									self->weapon_type = WEAPON_TYPE_ROCKET;
									weapons_fire_delayed(self, self->weapon_type);
								}
							}
							else if ((chance >= 54) && (chance < 60)) {
								self->update_strat_func = ship_ai_strat_avoid;
								flags_rm(self->flags, SHIP_OVERTAKEN);
								if (flags_not(self->flags, SHIP_SHIELDED) && flags_is(self->flags, SHIP_RACING)) {
									sfx_play(SFX_VOICE_MISSILE);
									self->weapon_type = WEAPON_TYPE_MISSILE;
									self->weapon_target = &g.ships[g.pilot];
									weapons_fire_delayed(self, self->weapon_type);
								}
							}
							else if ((chance >= 60) && (chance < 64)) {
								self->update_strat_func = ship_ai_strat_avoid;
								flags_rm(self->flags, SHIP_OVERTAKEN);
								if (flags_not(self->flags, SHIP_SHIELDED) && flags_is(self->flags, SHIP_RACING)) {
									sfx_play(SFX_VOICE_SHOCKWAVE);
									self->weapon_type = WEAPON_TYPE_EBOLT;
									self->weapon_target = &g.ships[g.pilot];
									weapons_fire_delayed(self, self->weapon_type);
								}
							}
						}
					}
					else { // If ship destined to be tail-ender then slow down
						self->remote_thrust_max = 2100 ;
						self->remote_thrust_mag = 25;
						self->speed = 2100 ;
						self->update_strat_func = ship_ai_strat_avoid;
						flags_rm(self->flags, SHIP_OVERTAKEN);
					}
				}

				for (int i = 0; i < NUM_PILOTS; i++) { // If another ship is just in front pass fight on
					if (flags_is(g.ships[i].flags, SHIP_JUST_IN_FRONT)) {
						self->update_strat_func = ship_ai_strat_avoid;
						flags_rm(self->flags, SHIP_OVERTAKEN);
					}
				}

				self->update_timer -= system_tick();


				if (flags_is(self->flags, SHIP_OVERTAKEN)) {
					if ((self->remote_thrust_max + 700) > self->speed) {
						self->speed += self->remote_thrust_mag * 2 * 30 * system_tick();
					}
				}
				else {
					if (((self->remote_thrust_max + behind_speed) > self->speed)) {
						self->speed += self->remote_thrust_mag * 30 * system_tick();
					}
				}
			}


			// Ship is WELL AHEAD; we must slow the opponent to
			// give the weaker player a chance to catch up
			
			else if (section_diff > (NUM_PILOTS - self->position_rank) * 15 && section_diff < 150) {
				self->speed += self->remote_thrust_mag * 0.5 * 30 * system_tick();
				if (self->speed > self->remote_thrust_max * 0.5) {
					self->speed = self->remote_thrust_max * 0.5;
				}

				self->update_timer = 0;
				self->update_strat_func = ship_ai_strat_hold_center;
			}


			// Ship is TOO FAR AHEAD

			else if (section_diff >= 150) { // Ship too far ahead, let it continue
				self->update_timer = 0;
				self->update_strat_func = ship_ai_strat_avoid;

				if ((self->remote_thrust_max > self->speed)) {
					self->speed += self->remote_thrust_mag * 30 * system_tick();
				}
			}


			// Ship is IN SIGHT

			else if ((section_diff <= 10) && (section_diff > 4)) { // Ship close by, beware does not account for lapped opponents yet
				if (self->update_timer <= 0) { // Make New Decision
					int chance = rand_int(0, 5);

					self->update_timer = UPDATE_TIME_IN_SIGHT;
					switch (chance) {
						case 0: self->update_strat_func = ship_ai_strat_hold_center; break;
						case 1: self->update_strat_func = ship_ai_strat_hold_left; break;
						case 2: self->update_strat_func = ship_ai_strat_hold_right; break;
						case 3:	self->update_strat_func = ship_ai_strat_block; break;
						case 4:	self->update_strat_func = ship_ai_strat_zig_zag; break;
					}
				}

				self->update_timer -= system_tick();

				if ((self->remote_thrust_max > self->speed)) {
					self->speed += self->remote_thrust_mag * 30 * system_tick();
				}
			} // End of DPA control options


			// Ship is JUST OUT OF SIGHT

			else {
				self->update_timer = 0;
				self->update_strat_func = ship_ai_strat_hold_center;
				if ((self->remote_thrust_max > self->speed)) {
					self->speed += self->remote_thrust_mag * 30 * system_tick();
				}
			}
		}

		if (!self->update_strat_func) {
			self->update_strat_func = ship_ai_strat_hold_center;
		}
		offset_vector = (self->update_strat_func)(self, face);


		// Make decision as to which path the craft will take at a junction

		section_t *section = self->section->prev;

		for (int i = 0; i < 4; i++) {
			section = section->next;
		}

		if (section->junction) {
			if (flags_is(section->junction->flags, SECTION_JUNCTION_START)) {
				int chance = rand_int(0, 2);
				if (chance == 0) {
					flags_add(self->flags, SHIP_JUNCTION_LEFT);
				}
				else {
					flags_rm(self->flags, SHIP_JUNCTION_LEFT);
				}
			}
		}

		section = self->section->prev;

		for (int i = 0; i < 4; i++) {
			if (section->junction) {
				if (flags_is(section->junction->flags, SECTION_JUNCTION_START)) {
					if (flags_is(self->flags, SHIP_JUNCTION_LEFT)) {
						section = section->junction;
					}
					else {
						section = section->next;
					}
				}
				else {
					section = section->next;
				}
			}
			else {
				section = section->next;
			}
		}
		section_t *next = section->next;
		section = self->section;




		// General routines - Non decision based


		// Bleed off speed as orientation changes

		self->speed -= fabsf(self->speed * self->angular_velocity.y) * 4 / (M_PI * 2) * system_tick(); // >> 14
		self->speed -= fabsf(self->speed * self->angular_velocity.x) * 4 / (M_PI * 2) * system_tick(); // >> 14

		// If remote has gone over boost
		if (flags_is(face->flags, FACE_BOOST) && (self->update_strat_func == ship_ai_strat_hold_left || self->update_strat_func == ship_ai_strat_hold_center)) {
			self->speed += 200 * 30 * system_tick();
		}
		face++;
		if (flags_is(face->flags, FACE_BOOST) && (self->update_strat_func == ship_ai_strat_hold_right || self->update_strat_func == ship_ai_strat_hold_center)) {
			self->speed += 200 * 30 * system_tick();
		}

		vec3_t track_target;
		if (flags_is(self->section->flags, SECTION_JUMP)) {	// Cure for ships getting stuck on hump lip
			track_target = vec3_sub(self->section->center, self->section->prev->center);
		}
		else {
			track_target = vec3_sub(next->center, section->center);
		}

		float gap_length = vec3_len(track_target);
		track_target = vec3_mulf(track_target, self->speed / gap_length);

		vec3_t path1 = vec3_add(section->center, offset_vector);
		vec3_t path2 = vec3_add(next->center, offset_vector);

		vec3_t best_path = vec3_project_to_ray(self->position, path2, path1);
		self->acceleration = vec3_add(track_target, vec3_mulf(vec3_sub(best_path, self->position), 0.5));


		vec3_t face_point = face->tris[0].vertices[0].pos;
		float height = vec3_distance_to_plane(self->position, face_point, face->normal);

		if (height < 50) {
			height = 50;
		}

		self->acceleration = vec3_add(self->acceleration, vec3_mulf(vec3_sub(
			vec3_mulf(face->normal, (SHIP_TRACK_FLOAT * SHIP_TRACK_MAGNET) / height),
			vec3_mulf(face->normal, SHIP_TRACK_MAGNET)
		), 16.0));
		self->velocity = vec3_add(self->velocity, vec3_mulf(self->acceleration, 30 * system_tick()));


		float xy_dist = sqrt(track_target.x * track_target.x + track_target.z * track_target.z);

		self->angular_velocity.x = wrap_angle(-atan2(track_target.y, xy_dist) - self->angle.x) * (1.0/16.0) * 30;
		self->angular_velocity.y = (wrap_angle(-atan2(track_target.x, track_target.z) - self->angle.y) * (1.0/16.0)) * 30 + self->turn_rate_from_hit;
	}


	// Ship is SHIP_FLYING

	else {
		section_t *section = self->section->next->next;
		section_t *next = section->next;

		self->update_strat_func = ship_ai_strat_hold_center;
		offset_vector = (self->update_strat_func)(self, NULL);

		if (self->remote_thrust_max > self->speed) {
			self->speed += self->remote_thrust_mag ;
		}

		self->speed -= fabsf(self->speed * self->angular_velocity.y) * (4 * M_PI * 2) * system_tick();
		vec3_t track_target = vec3_sub(next->center, section->center);
		float gap_length = vec3_len(track_target);

		track_target.x = (track_target.x * self->speed) / gap_length;
		track_target.z = (track_target.z * self->speed) / gap_length;

		track_target.y = 500;

		vec3_t best_path = vec3_project_to_ray(self->position, next->center, self->section->center);
		self->acceleration = vec3(
			(track_target.x + ((best_path.x - self->position.x) * 0.5)),
			track_target.y,
			(track_target.z + ((best_path.z - self->position.z) * 0.5))
		);
		self->velocity = vec3_add(self->velocity, vec3_mulf(self->acceleration, 30 * system_tick()));

		self->angular_velocity.x = -0.3 - self->angle.x * 30;
		self->angular_velocity.y = wrap_angle(-atan2(track_target.x, track_target.z) - self->angle.y) * (1.0/16.0) * 30;
	}

	
	self->angular_velocity.z += (self->angular_velocity.y * 2.0 - self->angular_velocity.z * 0.5) * 30 * system_tick();
	self->turn_rate_from_hit -= self->turn_rate_from_hit * 0.125 * 30 * system_tick();

	self->angle = vec3_add(self->angle, vec3_mulf(self->angular_velocity, system_tick()));
	self->angle.z -= self->angle.z * 0.125 * 30 * system_tick();
	self->angle = vec3_wrap_angle(self->angle);

	self->velocity = vec3_sub(self->velocity, vec3_mulf(self->velocity, 0.125 * 30 * system_tick()));
	self->position = vec3_add(self->position, vec3_mulf(self->velocity, 0.015625 * 30 * system_tick()));

	if (flags_is(self->flags, SHIP_ELECTROED)) {
		self->ebolt_effect_timer += system_tick();

		if (self->ebolt_effect_timer > 0.1) {
			self->ebolt_effect_timer -= 0.1;

			self->position = vec3_add(self->position, vec3(rand_float(-20, 20), rand_float(-20, 20), rand_float(-20, 20)));

			if (rand_int(0, 10) == 0) {
				self->speed -= self->speed * 0.5;
			}
		}
	}

	sfx_set_position(self->sfx_engine_thrust, self->position, self->velocity, 0.5);
}