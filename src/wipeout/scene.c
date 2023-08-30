#include "../mem.h"
#include "../utils.h"
#include "../system.h"

#include "object.h"
#include "track.h"
#include "ship.h"
#include "weapon.h"
#include "scene.h"
#include "droid.h"
#include "camera.h"
#include "object.h"
#include "game.h"


#define SCENE_START_BOOMS_MAX 4
#define SCENE_OIL_PUMPS_MAX 2
#define SCENE_RED_LIGHTS_MAX 4
#define SCENE_STANDS_MAX 20

static Object *scene_objects;
static Object *sky_object;
static vec3_t sky_offset;

static Object *start_booms[SCENE_START_BOOMS_MAX];
static int start_booms_len;

static Object *oil_pumps[SCENE_OIL_PUMPS_MAX];
static int oil_pumps_len;

static Object *red_lights[SCENE_RED_LIGHTS_MAX];
static int red_lights_len;

typedef struct {
	sfx_t *sfx;
	vec3_t pos;
} scene_stand_t;
static scene_stand_t stands[SCENE_STANDS_MAX];
static int stands_len;

static struct {
	bool enabled;
	GT4	*primitives[80];
	int16_t *coords[80];
	int16_t grey_coords[80];	
} aurora_borealis;

void scene_pulsate_red_light(Object *obj);
void scene_move_oil_pump(Object *obj);
void scene_update_aurora_borealis();

void scene_load(const char *base_path, float sky_y_offset) {
	texture_list_t scene_textures = image_get_compressed_textures(get_path(base_path, "scene.cmp"));
	scene_objects = objects_load(get_path(base_path, "scene.prm"), scene_textures);
	
	texture_list_t sky_textures = image_get_compressed_textures(get_path(base_path, "sky.cmp"));
	sky_object = objects_load(get_path(base_path, "sky.prm") , sky_textures);
	sky_offset = vec3(0, sky_y_offset, 0);

	// Collect all objects that need to be updated each frame
	start_booms_len = 0;
	oil_pumps_len = 0;
	red_lights_len = 0;
	stands_len = 0;

	Object *obj = scene_objects;
	while (obj) {
		mat4_set_translation(&obj->mat, obj->origin);

		if (str_starts_with(obj->name, "start")) {
			error_if(start_booms_len >= SCENE_START_BOOMS_MAX, "SCENE_START_BOOMS_MAX reached");
			start_booms[start_booms_len++] = obj;
		}
		else if (str_starts_with(obj->name, "redl")) {
			error_if(red_lights_len >= SCENE_RED_LIGHTS_MAX, "SCENE_RED_LIGHTS_MAX reached");
			red_lights[red_lights_len++] = obj;
		}
		else if (str_starts_with(obj->name, "donkey")) {
			error_if(oil_pumps_len >= SCENE_OIL_PUMPS_MAX, "SCENE_OIL_PUMPS_MAX reached");
			oil_pumps[oil_pumps_len++] = obj;
		}
		else if (
			str_starts_with(obj->name, "lostad") || 
			str_starts_with(obj->name, "stad_") ||
			str_starts_with(obj->name, "newstad_")
		) {
			error_if(stands_len >= SCENE_STANDS_MAX, "SCENE_STANDS_MAX reached");
			stands[stands_len++] = (scene_stand_t){.sfx = NULL, .pos = obj->origin};
		}
		obj = obj->next;
	}

	aurora_borealis.enabled = false;
}

void scene_init() {
	scene_set_start_booms(0);
	for (int i = 0; i < stands_len; i++) {
		stands[i].sfx = sfx_reserve_loop(SFX_CROWD);
	}
}

void scene_update() {
	for (int i = 0; i < red_lights_len; i++) {
		scene_pulsate_red_light(red_lights[i]);
	}
	for (int i = 0; i < oil_pumps_len; i++) {
		scene_move_oil_pump(oil_pumps[i]);
	}
	for (int i = 0; i < stands_len; i++) {
		sfx_set_position(stands[i].sfx, stands[i].pos, vec3(0, 0, 0), 0.4);
	}

	if (aurora_borealis.enabled) {
		scene_update_aurora_borealis();
	}
}

void scene_draw(camera_t *camera) {
	// Sky
	render_set_depth_write(false);
	mat4_set_translation(&sky_object->mat, vec3_add(camera->position, sky_offset));
	object_draw(sky_object, &sky_object->mat);
	render_set_depth_write(true);

	// Nearby objects
	vec3_t cam_pos = camera->position;
	Object *object = scene_objects;
	float max_dist_sq = RENDER_FADEOUT_FAR * RENDER_FADEOUT_FAR;
	while (object) {
		vec3_t d = vec3_sub(cam_pos, object->origin);
		float dist_sq = d.x * d.x + d.y * d.y + d.z * d.z;

		if (dist_sq < max_dist_sq) {
			object_draw(object, &object->mat);
		}
		
		object = object->next;
	}

}

void scene_set_start_booms(int light_index) {
	
	int lights_len = 1;
	rgba_t color = rgba(0, 0, 0, 0);

	if (light_index == 0) { // reset all 3
		lights_len = 3;
		color = rgba(0x20, 0x20, 0x20, 0xff);
	}
	else if (light_index == 1) {
		color = rgba(0xff, 0x00, 0x00, 0xff);
	}
	else if (light_index == 2) {
		color = rgba(0xff, 0x80, 0x00, 0xff);
	}
	else if (light_index == 3) {
		color = rgba(0x00, 0xff, 0x00, 0xff);
	}

	for (int i = 0; i < start_booms_len; i++) {
		Prm libPoly = {.primitive = start_booms[i]->primitives};

		for (int j = 1; j < light_index; j++) {
			libPoly.gt4 += 1;
		}

		for (int j = 0; j < lights_len; j++) {
			for (int v = 0; v < 4; v++) {
				libPoly.gt4->colour[v].r = color.r;
				libPoly.gt4->colour[v].g = color.g;
				libPoly.gt4->colour[v].b = color.b;
			}
			libPoly.gt4 += 1;
		}
	}
}


void scene_pulsate_red_light(Object *obj) {
	uint8_t r = clamp(sin(system_cycle_time() * M_PI * 2) * 128 + 128, 0, 255);
	Prm libPoly = {.primitive = obj->primitives};

	for (int v = 0; v < 4; v++) {
		libPoly.gt4->colour[v].r = r;
		libPoly.gt4->colour[v].g = 0x00;
		libPoly.gt4->colour[v].b = 0x00;
	}
}

void scene_move_oil_pump(Object *pump) {
	mat4_set_yaw_pitch_roll(&pump->mat, vec3(sin(system_cycle_time() * 0.125 * M_PI * 2), 0, 0));
}

void scene_init_aurora_borealis() {
	aurora_borealis.enabled = true;
	clear(aurora_borealis.grey_coords);

	int count = 0;
	int16_t *coords;
	float y;

	Prm poly = {.primitive = sky_object->primitives};
	for (int i = 0; i < sky_object->primitives_len; i++) {
		switch (poly.primitive->type) {
		case PRM_TYPE_GT3:
			poly.gt3 += 1;
			break;
		case PRM_TYPE_GT4:
			coords = poly.gt4->coords;
			y = sky_object->vertices[coords[0]].y;
			if (y < -6000) { // -8000
				aurora_borealis.primitives[count] = poly.gt4;
				if (y > -6800) {
					aurora_borealis.coords[count] = poly.gt4->coords;
					aurora_borealis.grey_coords[count] = -1;
				}
				else if (y < -11000) {
					aurora_borealis.coords[count] = poly.gt4->coords;
					aurora_borealis.grey_coords[count] = -2;
				}
				else {
					aurora_borealis.coords[count] = poly.gt4->coords;
				}
				count++;
			}
			poly.gt4 += 1;
			break;
		}
	}
}

void scene_update_aurora_borealis() {
	float phase = system_time() / 30.0;
	for (int i = 0; i < 80; i++) {
		int16_t *coords = aurora_borealis.coords[i];

		if (aurora_borealis.grey_coords[i] != -2) {
			aurora_borealis.primitives[i]->colour[0].r = (sin(coords[0] * phase) * 64.0) + 190;
			aurora_borealis.primitives[i]->colour[0].g = (sin(coords[0] * (phase + 0.054)) * 64.0) + 190;
			aurora_borealis.primitives[i]->colour[0].b = (sin(coords[0] * (phase + 0.039)) * 64.0) + 190;
		}
		if (aurora_borealis.grey_coords[i] != -2) {
			aurora_borealis.primitives[i]->colour[1].r = (sin(coords[1] * phase) * 64.0) + 190;
			aurora_borealis.primitives[i]->colour[1].g = (sin(coords[1] * (phase + 0.054)) * 64.0) + 190;
			aurora_borealis.primitives[i]->colour[1].b = (sin(coords[1] * (phase + 0.039)) * 64.0) + 190;
		}
		if (aurora_borealis.grey_coords[i] != -1) {
			aurora_borealis.primitives[i]->colour[2].r = (sin(coords[2] * phase) * 64.0) + 190;
			aurora_borealis.primitives[i]->colour[2].g = (sin(coords[2] * (phase + 0.054)) * 64.0) + 190;
			aurora_borealis.primitives[i]->colour[2].b = (sin(coords[2] * (phase + 0.039)) * 64.0) + 190;
		}

		if (aurora_borealis.grey_coords[i] != -1) {
			aurora_borealis.primitives[i]->colour[3].r = (sin(coords[3] * phase) * 64.0) + 190;
			aurora_borealis.primitives[i]->colour[3].g = (sin(coords[3] * (phase + 0.054)) * 64.0) + 190;
			aurora_borealis.primitives[i]->colour[3].b = (sin(coords[3] * (phase + 0.039)) * 64.0) + 190;
		}
	}
}
