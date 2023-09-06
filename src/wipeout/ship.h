#ifndef SHIP_H
#define SHIP_H

#include "../types.h"
#include "track.h"
#include "sfx.h"

#define SHIP_IN_TOW			 	(1<< 0)
#define SHIP_VIEW_REMOTE	 	(1<< 1)
#define SHIP_VIEW_INTERNAL		(1<< 2)
#define SHIP_DIRECTION_FORWARD	(1<< 3)
#define SHIP_FLYING				(1<< 4)
#define SHIP_LEFT_SIDE			(1<< 5)
#define SHIP_RACING				(1<< 6)
#define SHIP_COLL				(1<< 7)
#define SHIP_ON_JUNCTION		(1<< 8)
#define SHIP_VISIBLE			(1<< 9)
#define SHIP_IN_RESCUE 			(1<<10)
#define SHIP_OVERTAKEN 			(1<<11)
#define SHIP_JUST_IN_FRONT	    (1<<12)
#define SHIP_JUNCTION_LEFT		(1<<13)
#define SHIP_SHIELDED			(1<<14)
#define SHIP_ELECTROED			(1<<15)
#define SHIP_REVCONNED			(1<<16)
#define SHIP_SPECIALED			(1<<17)


// Timings

#define UPDATE_TIME_INITIAL   (200.0 * (1.0/30.0))
#define UPDATE_TIME_THREE     (150.0 * (1.0/30.0))
#define UPDATE_TIME_RACE_VIEW (100.0 * (1.0/30.0))
#define UPDATE_TIME_TWO       (100.0 * (1.0/30.0))
#define UPDATE_TIME_ONE       ( 50.0 * (1.0/30.0))
#define UPDATE_TIME_GO        (  0.0 * (1.0/30.0))


// Physics conversion

#define FIXED_TO_FLOAT(V) ((V) * (1.0/4096.0))
#define ANGLE_NORM_TO_RADIAN(V) ((V) * M_PI * 2.0)
#define NTSC_STEP_TO_RATE_PER_SECOND(V) ((V) * 30.0)
#define NTSC_ACCELERATION(V) NTSC_STEP_TO_RATE_PER_SECOND(NTSC_STEP_TO_RATE_PER_SECOND(V))
#define NTSC_VELOCITY(V) NTSC_STEP_TO_RATE_PER_SECOND(V)

#define PITCH_VELOCITY(V) ((V) * (1.0/16.0))
#define YAW_VELOCITY(V) ((V) * (1.0/64.0))
#define ROLL_VELOCITY(V) ((V) * (1.0))

#define SHIP_FLYING_GRAVITY   80000.0
#define SHIP_ON_TRACK_GRAVITY 30000.0
#define SHIP_MIN_RESISTANCE 	20	 // 12
#define SHIP_MAX_RESISTANCE 	74
#define SHIP_VELOCITY_SHIFT 	6
#define SHIP_TRACK_MAGNET		64	// 64
#define SHIP_TRACK_FLOAT 	256

#define SHIP_PITCH_ACCEL    NTSC_ACCELERATION(ANGLE_NORM_TO_RADIAN(FIXED_TO_FLOAT(PITCH_VELOCITY(30))))
#define SHIP_THRUST_RATE    NTSC_VELOCITY(16)
#define SHIP_THRUST_FALLOFF NTSC_VELOCITY(8)
#define SHIP_BRAKE_RATE     NTSC_VELOCITY(32)

typedef struct ship_t {
	int16_t pilot;
	int flags;

	section_t *section, *prev_section;

	vec3_t dir_forward;
	vec3_t dir_right;
	vec3_t dir_up;

	vec3_t position;
	vec3_t velocity;
	vec3_t acceleration;
	vec3_t thrust;

	vec3_t angle;
	vec3_t angular_velocity;
	vec3_t angular_acceleration;

	vec3_t temp_target; // used for start position and rescue target
	
	float turn_rate;
	float turn_rate_target;
	float turn_rate_max;
	float turn_rate_from_hit;

	float mass;
	float thrust_mag;
	float thrust_max;
	float current_thrust_max;
	float speed;
	float brake_left;
	float brake_right;

	float resistance;
	float skid;

	float remote_thrust_mag;
	float remote_thrust_max;

	// Remote Ship Attributes
	int16_t fight_back;
	float start_accelerate_timer;

	// Weapon Attributes
	uint8_t weapon_type;
	struct ship_t *weapon_target;

	float ebolt_timer;
	float ebolt_effect_timer;
	float revcon_timer;
	float special_timer;

	// Race Control Attributes
	int position_rank;
	int lap;
	int max_lap;
	float lap_time;

	int16_t section_num;
	int16_t prev_section_num;
	int16_t total_section_num;

	float update_timer;
	float last_impact_time;

	mat4_t mat;
	Object *model;
	Object *collision_model;
	uint16_t shadow_texture;

	struct {
		vec3_t *v;
		vec3_t initial;
	} exhaust_plume[3];

	// Control Routines
	vec3_t (*update_strat_func)(struct ship_t *, track_face_t *);
	void (*update_func)(struct ship_t *);

	// Audio
	sfx_t *sfx_engine_thrust;
	sfx_t *sfx_engine_intake;
	sfx_t *sfx_turbulence;
	sfx_t *sfx_shield;
} ship_t;

void ships_load(void);
void ships_init(section_t *section);
void ships_draw(void);
void ships_update(void);

void ship_init(ship_t *self, section_t *section, int pilot, int position);
void ship_init_exhaust_plume(ship_t *self);
void ship_draw(ship_t *self);
void ship_draw_shadow(ship_t *self);
void ship_update(ship_t *self);
void ship_collide_with_track(ship_t *self, track_face_t *face);
void ship_collide_with_ship(ship_t *self, ship_t *other);

vec3_t ship_cockpit(ship_t *self);
vec3_t ship_nose(ship_t *self);
vec3_t ship_wing_left(ship_t *self);
vec3_t ship_wing_right(ship_t *self);


#endif
