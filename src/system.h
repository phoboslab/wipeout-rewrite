#ifndef SYSTEM_H
#define SYSTEM_H

#include "types.h"

#define SYSTEM_WINDOW_NAME "wipEout"
#define SYSTEM_WINDOW_WIDTH 1280
#define SYSTEM_WINDOW_HEIGHT 720

void system_init();
void system_update();
void system_cleanup();
void system_exit();
void system_resize(vec2i_t size);

double system_time();
double system_tick();
double system_cycle_time();
void system_reset_cycle_time();
double system_time_scale_get();
void system_time_scale_set(double ts);

#endif
