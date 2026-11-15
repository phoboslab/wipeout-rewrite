#ifndef OBJECT_H
#define OBJECT_H

#include "../types.h"
#include "../render.h"
#include "../utils.h"
#include "image.h"

enum {
	// "Universal" primitives (models are converted to use these at load time.)
	PRM_TYPE_TRI,
	PRM_TYPE_QUAD,
	PRM_TYPE_SPR,
};

enum {
	PRM_SINGLE_SIDED = 1<<0,
	PRM_SHIP_ENGINE  = 1<<1,
	PRM_TRANSLUCENT  = 1<<2,
};

typedef struct {
	int16_t coord;
	uint8_t u, v;
	rgba_t color;
} primitive_vertex_t;

typedef struct Primitive {
	int8_t type;
	int8_t flag;
	union {
		struct {
			int16_t texture;
			primitive_vertex_t v[3];
		} tri;

		struct {
			int16_t texture;
			primitive_vertex_t v[4];
		} quad;

		struct {
			int16_t texture;
			int16_t coord;
			int16_t width;
			int16_t height;
			rgba_t color;
		} spr;

		/* TODO: Implement spline.
		 * The struct is commented out now to avoid paying for unimplemented features
		 * (it makes the union substantially larger.)
		 */
		/*
		struct {
			vec3_t control1;
			vec3_t position;
			vec3_t control2;
			rgba_t color;
		} spline;
		*/
	} u;
} primitive_t;

typedef struct Object {
	char name[16];

	mat4_t mat;

	int16_t vertices_len;
	vec3_t *vertices;
	int16_t primitives_len;
	primitive_t *primitives;

	vec3_t origin;
	float radius;
	struct Object *next;
} Object;

Object *objects_load(char *name, texture_list_t tl);
void object_draw(Object *object, mat4_t *mat);

#endif
