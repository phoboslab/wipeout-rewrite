#ifndef TRACK_H
#define TRACK_H


#include "../types.h"
#include "object.h"
#include "image.h"

#define TRACK_VERSION 8

#define TRACK_PICKUP_COOLDOWN_TIME 1

#define TRACK_SEARCH_LOOK_BACK 3
#define TRACK_SEARCH_LOOK_AHEAD 6

typedef struct track_face_t {
	tris_t tris[2];
	vec3_t normal;
	uint8_t flags;
	uint8_t texture;
} track_face_t;

#define FACE_TRACK_BASE       (1<<0)
#define FACE_PICKUP_LEFT      (1<<1)
#define FACE_FLIP_TEXTURE     (1<<2)
#define FACE_PICKUP_RIGHT     (1<<3)
#define FACE_START_GRID       (1<<4)
#define FACE_BOOST            (1<<5)
#define FACE_PICKUP_COLLECTED (1<<6)
#define FACE_PICKUP_ACTIVE    (1<<7)

typedef struct {
	uint16_t near[16];
	uint16_t med[4];
	uint16_t far;
} ttf_tile_t;

typedef struct {
	uint32_t len;
	ttf_tile_t tiles[];
} ttf_t;

typedef struct section_t {
	struct section_t *junction;
	struct section_t *prev;
	struct section_t *next;

	vec3_t center;

	int16_t face_start;
	int16_t face_count;

	int16_t flags;
	int16_t num;
} section_t;

#define SECTION_JUMP            1
#define SECTION_JUNCTION_END    8
#define SECTION_JUNCTION_START 16
#define SECTION_JUNCTION       32

typedef struct {
	track_face_t *face;
	float cooldown_timer;
} track_pickup_t;

typedef struct track_t {
	int32_t vertex_count;
	int32_t face_count;
	int32_t section_count;
	int32_t pickups_len;
	int32_t total_section_nums;
	texture_list_t textures;
	
	track_face_t *faces;
	section_t *sections;
	track_pickup_t *pickups;
} track_t;


void track_load(const char *base_path);
ttf_t *track_load_tile_format(char *ttf_name);
vec3_t *track_load_vertices(char *file);
void track_load_faces(char *file, vec3_t *vertices);
void track_load_sections(char *file);
bool track_collect_pickups(track_face_t *face);
void track_face_set_color(track_face_t *face, rgba_t color);
track_face_t *track_section_get_base_face(section_t *section);
section_t *track_nearest_section(vec3_t pos, vec3_t bias, section_t *section, float *distance);

struct camera_t;
void track_draw(struct camera_t *camera);

void track_cycle_pickups(void);

#endif
