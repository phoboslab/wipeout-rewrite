#ifndef WEAPON_H
#define WEAPON_H

#define WEAPONS_MAX 64

#define WEAPON_MINE_DURATION (450 * (1.0/30.0))
#define WEAPON_ROCKET_DURATION (200 * (1.0/30.0))
#define WEAPON_EBOLT_DURATION (140 * (1.0/30.0))
#define WEAPON_REV_CON_DURATION (60 * (1.0/30.0))
#define WEAPON_MISSILE_DURATION (200 * (1.0/30.0))
#define WEAPON_SHIELD_DURATION (200 * (1.0/30.0))
#define WEAPON_FLARE_DURATION (200 * (1.0/30.0))
#define WEAPON_SPECIAL_DURATION (400 * (1.0/30.0))

#define WEAPON_MINE_RELEASE_RATE (3 * (1.0/30.0))
#define WEAPON_DELAY (40 * (1.0/30.0))

#define WEAPON_TYPE_NONE      0
#define WEAPON_TYPE_MINE      1
#define WEAPON_TYPE_MISSILE   2
#define WEAPON_TYPE_ROCKET    3
#define WEAPON_TYPE_SPECIAL   4
#define WEAPON_TYPE_EBOLT     5
#define WEAPON_TYPE_FLARE     6
#define WEAPON_TYPE_REV_CON   7
#define WEAPON_TYPE_SHIELD    8
#define WEAPON_TYPE_TURBO     9
#define WEAPON_TYPE_MAX      10

#define WEAPON_MINE_COUNT 5

#define WEAPON_HIT_NONE 0
#define WEAPON_HIT_SHIP 1
#define WEAPON_HIT_TRACK 2

#define WEAPON_PARTICLE_SPAWN_RATE 0.011
#define WEAPON_AI_DELAY 1.1

#define WEAPON_CLASS_ANY 1
#define WEAPON_CLASS_PROJECTILE 2


void weapons_load();
void weapons_init();
void weapons_fire(ship_t *ship, int weapon_type);
void weapons_fire_delayed(ship_t *ship, int weapon_type);
void weapons_update();
void weapons_draw();
int weapon_get_random_type(int type_class);

#endif
