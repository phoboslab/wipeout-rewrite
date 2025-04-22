#include "../mem.h"
#include "../utils.h"
#include "../system.h"
#include "../render.h"

#include "particle.h"
#include "object.h"
#include "image.h"

static particle_t *particles;
static int particles_active = 0;
static texture_list_t particle_textures;

void particles_load(void) {
	particles = mem_bump(sizeof(particle_t) * PARTICLES_MAX);
	particle_textures = image_get_compressed_textures("wipeout/common/effects.cmp");
	particles_init();
}

void particles_init(void) {
	particles_active = 0;
}

void particles_update(void) {
	for (int i = 0; i < particles_active; i++) {
		particle_t *p = &particles[i];

		p->timer -= system_tick();
		p->position = vec3_add(p->position, vec3_mulf(p->velocity, system_tick()));
		if (p->timer < 0) {
			particles[i--] = particles[--particles_active];
			continue;
		}
	}
}

void particles_draw(void) {
	if (particles_active == 0) {
		return;
	}

	render_set_model_mat(&mat4_identity());
	render_set_depth_write(false);
	render_set_blend_mode(RENDER_BLEND_LIGHTER);
	render_set_depth_offset(-32.0);

	for (int i = 0; i < particles_active; i++) {
		particle_t *p = &particles[i];
		render_push_sprite(p->position, p->size, p->color, p->texture, PRM_SINGLE_SIDED);
	}

	render_set_depth_offset(0.0);
	render_set_depth_write(true);
	render_set_blend_mode(RENDER_BLEND_NORMAL);
}

void particles_spawn(vec3_t position, uint16_t type, vec3_t velocity, int size) {
	if (particles_active == PARTICLES_MAX) {
		return;
	}

	particle_t *p = &particles[particles_active++];
	p->color = rgba(128, 128, 128, 128);
	p->texture = texture_from_list(particle_textures, type);
	p->position = position;
	p->velocity = velocity;
	p->timer = rand_float(0.75, 1.0);
	p->size.x = size;
	p->size.y = size;
}

