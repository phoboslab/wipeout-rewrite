#ifndef WEAPON_H
#define WEAPON_H

#include "ship.h"

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

#define WEAPON_MINE_COUNT 5

#define WEAPON_PARTICLE_SPAWN_RATE 0.011
#define WEAPON_AI_DELAY 1.1

enum {
    WEAPON_TYPE_NONE = 0,
    WEAPON_TYPE_MINE,
    WEAPON_TYPE_MISSILE,
    WEAPON_TYPE_ROCKET,
    WEAPON_TYPE_SPECIAL,
    WEAPON_TYPE_EBOLT,
    WEAPON_TYPE_FLARE,
    WEAPON_TYPE_REV_CON,
    WEAPON_TYPE_SHIELD,
    WEAPON_TYPE_TURBO,
    WEAPON_TYPE_MAX,
};

enum {
    WEAPON_CLASS_ANY = 1,
    WEAPON_CLASS_PROJECTILE,
};

void weapons_load(void);
void weapons_init(void);
void weapons_fire(ship_t *ship, int weapon_type);
void weapons_fire_delayed(ship_t *ship, int weapon_type);
void weapons_update(void);
void weapons_draw(void);
int weapon_get_random_type(int type_class);

#endif
