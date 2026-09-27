#include "../types.h"
#include "../mem.h"
#include "../render.h"
#include "../utils.h"
#include "../platform.h"

#include "object.h"

// Altering this enum (other than by adding to the end)
// will break compatibility with the assets.
enum {
	PSX_PRM_TYPE_F3 = 1,
	PSX_PRM_TYPE_FT3,
	PSX_PRM_TYPE_F4,
	PSX_PRM_TYPE_FT4,
	PSX_PRM_TYPE_G3,
	PSX_PRM_TYPE_GT3,
	PSX_PRM_TYPE_G4,
	PSX_PRM_TYPE_GT4,

	PSX_PRM_TYPE_TSPR = 10,
	PSX_PRM_TYPE_BSPR,

	PSX_PRM_TYPE_SPLINE = 20,
};

// The types above can be interpreted as flags like so.
// (You must decrement the type by one for this to work.)
enum {
	PSX_PRM_FLAG_TEXTURED = 1 << 0,
	PSX_PRM_FLAG_QUAD     = 1 << 1,
	PSX_PRM_FLAG_GOURAUD  = 1 << 2,
};

Object *objects_load(char *name, texture_list_t tl) {
	uint32_t length = 0;
	uint8_t *bytes = platform_load_asset(name, &length);
	if (!bytes) {
		die("Failed to load file %s\n", name);
	}
#ifdef VERBOSE_PRINTING
	printf("load: %s\n", name);
#endif
	Object *objectList = mem_mark();
	Object *prevObject = NULL;
	uint32_t p = 0;

	while (p < length) {
		Object *object = mem_bump(sizeof(Object));
		if (prevObject) {
			prevObject->next = object;
		}
		prevObject = object;

		for (int i = 0; i < 16; i++) {
			object->name[i] = get_i8(bytes, &p);
		}

		object->mat = mat4_identity();
		object->vertices_len = get_i16(bytes, &p); p += 2;
		object->vertices = NULL; get_i32(bytes, &p);

		int normals_len = get_i16(bytes, &p); p += 2;
		get_i32(bytes, &p); // normals
		error_if(normals_len != 0, "File %s contains %d normals, normals are not supported.", name, normals_len);

		object->primitives_len = get_i16(bytes, &p); p += 2;
		object->primitives = NULL; get_i32(bytes, &p);
		get_i32(bytes, &p);
		get_i32(bytes, &p);
		get_i32(bytes, &p); // Skeleton ref
		get_i32(bytes, &p); // extent
		get_i16(bytes, &p); /* flags */ p += 2;
		object->next = NULL; get_i32(bytes, &p);

		p += 3 * 3 * 2; // relative rot matrix
		p += 2; // padding

		object->origin.x = get_i32(bytes, &p);
		object->origin.y = get_i32(bytes, &p);
		object->origin.z = get_i32(bytes, &p);

		p += 3 * 3 * 2; // absolute rot matrix
		p += 2; // padding
		p += 3 * 4; // absolute translation matrix
		p += 2; // skeleton update flag
		p += 2; // padding
		p += 4; // skeleton super
		p += 4; // skeleton sub
		p += 4; // skeleton next

		object->radius = 0;
		object->vertices = mem_bump(object->vertices_len * sizeof(vec3_t));
		for (int i = 0; i < object->vertices_len; i++) {
			object->vertices[i].x = get_i16(bytes, &p);
			object->vertices[i].y = get_i16(bytes, &p);
			object->vertices[i].z = get_i16(bytes, &p);
			p += 2; // padding

			object->radius = max(vec3_len_sq(object->vertices[i]), object->radius);
		}
		object->radius = sqrt(object->radius);


		// Normals would be loaded here, if there were any.

		object->primitives = mem_bump(object->primitives_len * sizeof(primitive_t));
		for (int i = 0; i < object->primitives_len; i++) {
			primitive_t *prm = &object->primitives[i];

			int psx_prm_type = get_i16(bytes, &p);
			prm->flag = get_i16(bytes, &p);
			switch (psx_prm_type) {
			case PSX_PRM_TYPE_F3:
			case PSX_PRM_TYPE_FT3:
			case PSX_PRM_TYPE_F4:
			case PSX_PRM_TYPE_FT4:
			case PSX_PRM_TYPE_G3:
			case PSX_PRM_TYPE_GT3:
			case PSX_PRM_TYPE_G4:
			case PSX_PRM_TYPE_GT4:
				psx_prm_type--; // Flags trick does not line up right without this.
				if (psx_prm_type & PSX_PRM_FLAG_QUAD) {
					prm->type = PRM_TYPE_QUAD;
					for (int j = 0; j < 4; j++) {
						prm->u.quad.v[j].coord = get_i16(bytes, &p);
					}

					if (psx_prm_type & PSX_PRM_FLAG_TEXTURED) {
						prm->u.quad.texture = texture_from_list(tl, get_i16(bytes, &p));
						p += 2 + 2; // csb/tsb
						for (int j = 0; j < 4; j++) {
							prm->u.quad.v[j].u = get_i8(bytes, &p);
							prm->u.quad.v[j].v = get_i8(bytes, &p);
						}
						p += 2; // padding
					}
					if (psx_prm_type & PSX_PRM_FLAG_GOURAUD) {
						prm->u.quad.v[0].color = rgba_from_u32(get_u32(bytes, &p));
						prm->u.quad.v[1].color = rgba_from_u32(get_u32(bytes, &p));
						prm->u.quad.v[2].color = rgba_from_u32(get_u32(bytes, &p));
						prm->u.quad.v[3].color = rgba_from_u32(get_u32(bytes, &p));
					} else {
						prm->u.quad.v[0].color =
						prm->u.quad.v[1].color =
						prm->u.quad.v[2].color =
						prm->u.quad.v[3].color = rgba_from_u32(get_u32(bytes, &p));
					}
				} else {
					prm->type = PRM_TYPE_TRI;
					for (int j = 0; j < 3; j++) {
						prm->u.tri.v[j].coord = get_i16(bytes, &p);
					}

					if (psx_prm_type & PSX_PRM_FLAG_TEXTURED) {
						prm->u.tri.texture = texture_from_list(tl, get_i16(bytes, &p));
						p += 2 + 2; // csb/tsb
						for (int j = 0; j < 3; j++) {
							prm->u.tri.v[j].u = get_i8(bytes, &p);
							prm->u.tri.v[j].v = get_i8(bytes, &p);
						}
					}
					p += 2; // padding
					if (psx_prm_type & PSX_PRM_FLAG_GOURAUD) {
						prm->u.tri.v[0].color = rgba_from_u32(get_u32(bytes, &p));
						prm->u.tri.v[1].color = rgba_from_u32(get_u32(bytes, &p));
						prm->u.tri.v[2].color = rgba_from_u32(get_u32(bytes, &p));
					} else {
						prm->u.tri.v[0].color =
						prm->u.tri.v[1].color =
						prm->u.tri.v[2].color = rgba_from_u32(get_u32(bytes, &p));
					}
				}
				break;
			case PSX_PRM_TYPE_TSPR:
			case PSX_PRM_TYPE_BSPR:
				prm->type = PRM_TYPE_SPR;

				prm->u.spr.coord = get_i16(bytes, &p);
				prm->u.spr.width = get_i16(bytes, &p);
				prm->u.spr.height = get_i16(bytes, &p);
				prm->u.spr.texture = texture_from_list(tl, get_i16(bytes, &p));
				prm->u.spr.color = rgba_from_u32(get_u32(bytes, &p));

				object->vertices[prm->u.spr.coord].y += ((psx_prm_type == PSX_PRM_TYPE_TSPR ? prm->u.spr.height : -prm->u.spr.height) >> 1);
				break;
			case PSX_PRM_TYPE_SPLINE:
				p += 52;
				/* TODO: Implement spline.
				prm->type = psx_prm_type;
				prm->psx.spline.control1.x = get_i32(bytes, &p);
				prm->psx.spline.control1.y = get_i32(bytes, &p);
				prm->psx.spline.control1.z = get_i32(bytes, &p);
				p += 4; // padding
				prm->psx.spline.position.x = get_i32(bytes, &p);
				prm->psx.spline.position.y = get_i32(bytes, &p);
				prm->psx.spline.position.z = get_i32(bytes, &p);
				p += 4; // padding
				prm->psx.spline.control2.x = get_i32(bytes, &p);
				prm->psx.spline.control2.y = get_i32(bytes, &p);
				prm->psx.spline.control2.z = get_i32(bytes, &p);
				p += 4; // padding
				prm->psx.spline.color = rgba_from_u32(get_u32(bytes, &p));
				*/
				break;
			default:
				die("Unknown primitive type %x", prm->type);
			} // switch
		} // each prim
	} // each object

	mem_temp_free(bytes);
	return objectList;
}


void object_draw(Object *object, mat4_t *mat) {
	vec3_t *vertex = object->vertices;
	render_set_model_mat(mat);

	for (int i = 0; i < object->primitives_len; i++) {
		primitive_t *prm = &object->primitives[i];
		vertex_t v[4];
		switch (prm->type) {
		case PRM_TYPE_SPR:
			render_push_sprite(
				vertex[prm->u.spr.coord],
				vec2i(prm->u.spr.width, prm->u.spr.height),
				prm->u.spr.color,
				prm->u.spr.texture
			);
			break;
		case PRM_TYPE_TRI:
			for (int j = 0; j < 3; j++) {
				primitive_vertex_t p = prm->u.tri.v[j];
				v[j] = (vertex_t){
					.pos = vertex[p.coord],
					.uv = {p.u, p.v},
					.color = p.color
				};
			}
			render_push_tris((tris_t) {.vertices = {v[2],v[1],v[0]}}, prm->u.quad.texture);
			// Double sided polygons are achieved by submitting them twice with different winding orders.
			if (flags_not(prm->flag, PRM_SINGLE_SIDED)) {
				render_push_tris((tris_t) {.vertices = {v[1],v[2],v[0]}}, prm->u.quad.texture);
			}
			break;
		case PRM_TYPE_QUAD:
			for (int j = 0; j < 4; j++) {
				primitive_vertex_t p = prm->u.quad.v[j];
				v[j] = (vertex_t){
					.pos = vertex[p.coord],
					.uv = {p.u, p.v},
					.color = p.color
				};
			}
			render_push_tris((tris_t) {.vertices = {v[2],v[1],v[0]}}, prm->u.quad.texture);
			render_push_tris((tris_t) {.vertices = {v[2],v[3],v[1]}}, prm->u.quad.texture);
			// See above.
			if (flags_not(prm->flag, PRM_SINGLE_SIDED)) {
				render_push_tris((tris_t) {.vertices = {v[1],v[2],v[0]}}, prm->u.quad.texture);
				render_push_tris((tris_t) {.vertices = {v[3],v[2],v[1]}}, prm->u.quad.texture);
			}
			break;
		default:
			die("Can't happen: Unknown primitive type %x", prm->type);
		}
	}
}
