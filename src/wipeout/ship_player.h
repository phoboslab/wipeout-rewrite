#ifndef SHIP_PLAYER_H
#define SHIP_PLAYER_H

#include "ship.h"

#define UPDATE_TIME_RESCUE (500.0 * (1.0/30.0))
#define UPDATE_TIME_STALL   (90.0 * (1.0/30.0))

void ship_player_update_intro(ship_t *self);
void ship_player_update_intro_await_three(ship_t *self);
void ship_player_update_intro_await_two(ship_t *self);
void ship_player_update_intro_await_one(ship_t *self);
void ship_player_update_intro_await_go(ship_t *self);
void ship_player_update_intro_general(ship_t *self);
void ship_player_update_race(ship_t *self);
void ship_player_update_rescue(ship_t *self);

ship_t *ship_player_find_target(ship_t *self);

#endif
