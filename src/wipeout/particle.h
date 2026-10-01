#ifndef PARTICLE_H
#define PARTICLE_H

#include "../types.h"

#define PARTICLES_MAX 1024

typedef enum {
	PARTICLE_TYPE_NONE = -1,
	PARTICLE_TYPE_FIRE = 0,
	PARTICLE_TYPE_FIRE_WHITE,
	PARTICLE_TYPE_SMOKE,
	PARTICLE_TYPE_EBOLT,
	PARTICLE_TYPE_HALO,
	PARTICLE_TYPE_GREENY,
} particle_type_t;

typedef struct particle_t {
	vec3_t position;
	vec3_t velocity;
	vec2i_t size;
	rgba_t color;
	float timer;
	uint16_t type;
	uint16_t texture;
} particle_t;

void particles_load(void);
void particles_init(void);
void particles_spawn(vec3_t position, particle_type_t type, vec3_t velocity, int size);
void particles_draw(void);
void particles_update(void);

#endif
