#ifndef SHIP_AI_H
#define SHIP_AI_H

#include "ship.h"

#define UPDATE_TIME_JUST_FRONT  (150.0 * (1.0/30.0))
#define UPDATE_TIME_JUST_BEHIND (200.0 * (1.0/30.0))
#define UPDATE_TIME_IN_SIGHT    (200.0 * (1.0/30.0))

void ship_ai_update_race(ship_t *self);
void ship_ai_update_intro(ship_t *self);
void ship_ai_update_intro_await_go(ship_t *self);

#endif
