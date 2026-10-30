#ifndef OBJECT_H
#define OBJECT_H

#include "../types.h"
#include "../render.h"
#include "../utils.h"
#include "image.h"

enum {
	PRM_SINGLE_SIDED = 1<<0,
	PRM_SHIP_ENGINE  = 1<<1,
};

typedef struct {
	int16_t coord;
	uint8_t u, v;
	rgba_t color;
} primitive_vertex_t;

// Not to be confused with tris_t.
typedef struct triangle {
	int8_t flag;
	int16_t texture;
	primitive_vertex_t v[3];
} triangle_t;

typedef struct sprite {
	int16_t texture;
	int16_t coord;
	int16_t width;
	int16_t height;
	rgba_t color;
} sprite_t;

typedef struct Object {
	char name[16];
	mat4_t mat;
	vec3_t origin;
	float radius;
	struct Object *next;

	// Graphics
	size_t vertices_len;
	vec3_t *vertices;
	size_t tris_len;
	triangle_t *tris;
	size_t sprites_len;
	sprite_t *sprites;
} Object;

Object *objects_load(char *name, texture_list_t tl);
void object_draw(Object *object, mat4_t *mat);

#endif
