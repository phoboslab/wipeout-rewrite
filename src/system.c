#include "system.h"
#include "input.h"
#include "render.h"
#include "platform.h"
#include "mem.h"
#include "utils.h"

#include "wipeout/game.h"

static double time_real;
static double time_scaled;
static double time_scale = 1.0;
static double tick_last;
static double cycle_time = 0;

void system_init() {
	time_real = platform_now();
	input_init();
	render_init(platform_screen_size());
	game_init();
}

void system_cleanup() {
	render_cleanup();
	input_cleanup();
}

void system_exit() {
	platform_exit();
}

void system_update() {
	double time_real_now = platform_now();
	double real_delta = time_real_now - time_real;
	time_real = time_real_now;
	tick_last = min(real_delta, 0.1) * time_scale;
	time_scaled += tick_last;

	// FIXME: come up with a better way to wrap the cycle_time, so that it
	// doesn't lose precission, but also doesn't jump upon reset.
	cycle_time = time_scaled;
	if (cycle_time > 3600 * M_PI) {
		cycle_time -= 3600 * M_PI;
	}
	
	render_frame_prepare();
	
	game_update();

	render_frame_end();
	input_clear();
	mem_temp_check();
}

void system_reset_cycle_time() {
	cycle_time = 0;
}

void system_resize(vec2i_t size) {
	render_resize(size);
}

double system_time_scale_get() {
	return time_scale;
}

void system_time_scale_set(double scale) {
	time_scale = scale;
}

double system_tick() {
	return tick_last;
}

double system_time() {
	return time_scaled;
}

double system_cycle_time() {
	return cycle_time;
}
