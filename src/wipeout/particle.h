#ifndef PARTICLE_H
#define PARTICLE_H

#include "../types.h"

#define PARTICLES_MAX 1024

#define PARTICLE_TYPE_NONE -1
#define PARTICLE_TYPE_FIRE 0
#define PARTICLE_TYPE_FIRE_WHITE 1
#define PARTICLE_TYPE_SMOKE 2
#define PARTICLE_TYPE_EBOLT 3
#define PARTICLE_TYPE_HALO 4
#define PARTICLE_TYPE_GREENY 5

typedef struct particle_t {
	vec3_t position;
	vec3_t velocity;
	vec2i_t size;
	rgba_t color;
	float timer;
	uint16_t type;
	uint16_t texture;
} particle_t;

void particles_load();
void particles_init();
void particles_spawn(vec3_t position, uint16_t type, vec3_t velocity, int size);
void particles_draw();
void particles_update();

#endif