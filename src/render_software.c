#include "system.h"
#include "render.h"
#include "mem.h"
#include "utils.h"
#include "platform.h"
#include "types.h"

#define NEAR_PLANE 16.0
#define FAR_PLANE (RENDER_FADEOUT_FAR)
#define TEXTURES_MAX 1024
#define TRIS_BUFFER_SIZE 4096

typedef struct {
	vec2i_t size;
	rgba_t *pixels;
} render_texture_t;

typedef struct {
	vec4_t clip_pos;
	vec2_t uv;
	vec4_t color;
} clip_vert_t;

typedef struct {
	render_texture_t *texture;
	clip_vert_t verts[3];
} clip_tris_t;

static void draw_tris(clip_tris_t t);
static void render_flush(void);

static rgba_t *screen_buffer;
static int32_t screen_pitch;
static int32_t screen_ppr;
static vec2i_t screen_size;
static float *depth_buffer;
static uint32_t depth_buffer_len;

static render_blend_mode_t blend_mode = RENDER_BLEND_NORMAL;
static bool depth_write_enabled = true;
static bool depth_test_enabled = true;
static float depth_offset = 0.0f;
static bool cull_backface_enabled = false;

static mat4_t view_mat = mat4_identity();
static mat4_t mvp_mat = mat4_identity();
static mat4_t projection_mat = mat4_identity();
static mat4_t sprite_mat = mat4_identity();

static render_texture_t textures[TEXTURES_MAX];
static uint32_t textures_len;

uint16_t RENDER_NO_TEXTURE;

int32_t tris_buffer_len = 0;
clip_tris_t tris_buffer[TRIS_BUFFER_SIZE];


void render_init(vec2i_t screen_size) {
	render_set_screen_size(screen_size);
	textures_len = 0;

	rgba_t white_pixels[4] = {
		rgba(128,128,128,255), rgba(128,128,128,255),
		rgba(128,128,128,255), rgba(128,128,128,255)
	};
	RENDER_NO_TEXTURE = render_texture_create(2, 2, white_pixels);
}

void render_cleanup(void) {
	free(depth_buffer);
	depth_buffer = NULL;
	depth_buffer_len = 0;
}

void render_set_screen_size(vec2i_t size) {
	screen_size = size;
	error_if(size.x <= 0 || size.y <= 0, "Invalid screen size %d x %d", size.x, size.y);

	uint32_t pixel_count = (uint32_t)size.x * (uint32_t)size.y;
	if (pixel_count != depth_buffer_len) {
		// This is where our own memory model falls apart. We cannot reallocate
		// in bump memory. Maybe use a fixed allocation with maximum size
		// instead?
		// Since this whole renderer is only a toy we just malloc() for now.
		float *resized = realloc(depth_buffer, pixel_count * sizeof(float));
		error_if(resized == NULL, "Failed to allocate depth buffer");
		depth_buffer = resized;
		depth_buffer_len = pixel_count;
	}

	float aspect = (float)size.x / (float)size.y;
	float fov = (73.75 / 180.0) * M_PI;
	float f = 1.0 / tan(fov / 2);
	float nf = 1.0 / (NEAR_PLANE - FAR_PLANE);
	projection_mat = mat4(
		f / aspect, 0, 0, 0,
		0, f, 0, 0, 
		0, 0, (FAR_PLANE + NEAR_PLANE) * nf, -1, 
		0, 0, 2 * FAR_PLANE * NEAR_PLANE * nf, 0
	);
}

void render_set_resolution(render_resolution_t res) {}
void render_set_post_effect(render_post_effect_t post) {}

vec2i_t render_size(void) {
	return screen_size;
}

void render_frame_prepare(void) {
	screen_buffer = platform_get_screenbuffer(&screen_pitch);
	screen_ppr = screen_pitch / sizeof(rgba_t);

	rgba_t clear_color = rgba(0, 0, 0, 255);
	uint32_t screen_buffer_len = (screen_pitch / sizeof(rgba_t)) * screen_size.y;
	for (int i = 0; i < screen_buffer_len; i++) {
		screen_buffer[i] = clear_color;
	}

	for (uint32_t i = 0; i < depth_buffer_len; i++) {
		depth_buffer[i] = 1.0f;
	}
}

void render_frame_end(void) {
	render_flush();
}

void render_set_view(vec3_t pos, vec3_t angles) {
	render_set_depth_write(true);
	render_set_depth_test(true);

	view_mat = mat4_identity();
	mat4_set_translation(&view_mat, vec3(0, 0, 0));
	mat4_set_roll_pitch_yaw(&view_mat, vec3(angles.x, -angles.y + M_PI, angles.z + M_PI));
	mat4_translate(&view_mat, vec3_inv(pos));
	mat4_set_yaw_pitch_roll(&sprite_mat, vec3(-angles.x, angles.y - M_PI, 0));

	render_set_model_mat(&mat4_identity());
}

void render_set_view_2d(void) {
	render_set_depth_test(false);
	render_set_depth_write(false);

	float near = -1;
	float far = 1;
	float left = 0;
	float right = screen_size.x;
	float bottom = screen_size.y;
	float top = 0;
	float lr = 1 / (left - right);
	float bt = 1 / (bottom - top);
	float nf = 1 / (near - far);
	mvp_mat = mat4(
		-2 * lr,  0,  0,  0,
		0,  -2 * bt,  0,  0,
		0,		0,  2 * nf,	0, 
		(left + right) * lr, (top + bottom) * bt, (far + near) * nf, 1
	);
}

void render_set_model_mat(mat4_t *m) {
	mat4_t vm_mat;
	mat4_mul(&vm_mat, &view_mat, m);
	mat4_mul(&mvp_mat, &projection_mat, &vm_mat);
}

void render_set_depth_write(bool enabled) {
	render_flush();
	depth_write_enabled = enabled;
}
void render_set_depth_test(bool enabled) {
	render_flush();
	depth_test_enabled = enabled;
}
void render_set_depth_offset(float offset) {
	render_flush();
	depth_offset = offset;
}
void render_set_screen_position(vec2_t pos) {}

void render_set_blend_mode(render_blend_mode_t mode) {
	render_flush();
	blend_mode = mode;
}

void render_set_cull_backface(bool enabled) {
	render_flush();
	cull_backface_enabled = enabled;
}

vec3_t render_transform(vec3_t pos) {
	return vec4_perspective_divide(vec3_transform_perspective(vec3_transform(pos, &view_mat), &projection_mat));
}


static int sort_sw_tris_by_depth(const void *a, const void *b) {
	const clip_tris_t *ta = (const clip_tris_t *)a;
	const clip_tris_t *tb = (const clip_tris_t *)b;

	float za = (ta->verts[0].clip_pos.z + ta->verts[1].clip_pos.z + ta->verts[2].clip_pos.z) * 10000.0f;
	float zb = (tb->verts[0].clip_pos.z + tb->verts[1].clip_pos.z + tb->verts[2].clip_pos.z) * 10000.0f;
	return za - zb;
}

static void render_flush(void) {
	// Sort tris by depth and draw front to back to minimize overdraw. The final
	// pixel color calculation is only a small part of the whole triangle
	// rasterization, but it still helps a little to skip it when depth testing.

	qsort(tris_buffer, tris_buffer_len, sizeof(clip_tris_t), sort_sw_tris_by_depth);

	for (int i = 0; i < tris_buffer_len; i++) {
		draw_tris(tris_buffer[i]);
	}
	tris_buffer_len = 0;
}

static vec4_t rgba_to_vec4(rgba_t c) {
	return vec4(
		min(c.r * 2, 255) * (1.0/255.0),
		min(c.g * 2, 255) * (1.0/255.0),
		min(c.b * 2, 255) * (1.0/255.0),
		c.a * (1.0/255.0)
	);
}

static int clip_near(clip_vert_t *in, int in_len, clip_vert_t *out) {
	if (in_len < 3) {
		return 0;
	}

	int out_len = 0;
	clip_vert_t prev = in[in_len - 1];
	bool prev_in = (prev.clip_pos.z >= -prev.clip_pos.w);

	for (int i = 0; i < in_len; i++) {
		clip_vert_t curr = in[i];
		bool curr_in = (curr.clip_pos.z >= -curr.clip_pos.w);

		if (prev_in != curr_in) {
			float a = prev.clip_pos.z + prev.clip_pos.w;
			float b = curr.clip_pos.z + curr.clip_pos.w;
			float t = a / (a - b);
			out[out_len++] = (clip_vert_t){
				.clip_pos = vec4_lerp(prev.clip_pos, curr.clip_pos, t),
				.uv = vec2_lerp(prev.uv, curr.uv, t),
				.color = vec4_lerp(prev.color, curr.color, t)
			};
		}

		if (curr_in) {
			out[out_len++] = curr;
		}

		prev = curr;
		prev_in = curr_in;
	}

	return out_len;
}

void render_push_tris(tris_t tris, uint16_t texture_index) {
	if (tris_buffer_len >= TRIS_BUFFER_SIZE - 8) {
		render_flush();
	}

	error_if(texture_index >= textures_len, "Invalid texture %d", texture_index);
	render_texture_t *texture = &textures[texture_index];

	vec4_t color0 = rgba_to_vec4(tris.vertices[0].color);
	vec4_t color1 = rgba_to_vec4(tris.vertices[1].color);
	vec4_t color2 = rgba_to_vec4(tris.vertices[2].color);

	clip_vert_t in[3] = {
		{.clip_pos = vec3_transform_perspective(tris.vertices[0].pos, &mvp_mat), .uv = tris.vertices[0].uv, .color = color0},
		{.clip_pos = vec3_transform_perspective(tris.vertices[1].pos, &mvp_mat), .uv = tris.vertices[1].uv, .color = color1},
		{.clip_pos = vec3_transform_perspective(tris.vertices[2].pos, &mvp_mat), .uv = tris.vertices[2].uv, .color = color2},
	};
	clip_vert_t clipped[8];
	int clipped_len = clip_near(in, len(in), clipped);
	if (clipped_len < 3) {
		return;
	}

	// Perspective divide the screen space tris now. We don't need the original
	// position anymore, so just overwrite.
	for (int i = 0; i < clipped_len; i++) {
		vec3_t d = vec4_perspective_divide(clipped[i].clip_pos);
		clipped[i].clip_pos.x = d.x;
		clipped[i].clip_pos.y = d.y;
		clipped[i].clip_pos.z = d.z;
	}

	for (int i = 1; i < clipped_len - 1; i++) {
		tris_buffer[tris_buffer_len++] = (clip_tris_t){
			.texture = texture,
			.verts = {clipped[0], clipped[i], clipped[i + 1]}
		};
	}
}

void render_push_sprite(vec3_t pos, vec2i_t size, rgba_t color, uint16_t texture_index) {
	error_if(texture_index >= textures_len, "Invalid texture %d", texture_index);

	vec3_t p0 = vec3_add(pos, vec3_transform(vec3(-size.x * 0.5, -size.y * 0.5, 0), &sprite_mat));
	vec3_t p1 = vec3_add(pos, vec3_transform(vec3( size.x * 0.5, -size.y * 0.5, 0), &sprite_mat));
	vec3_t p2 = vec3_add(pos, vec3_transform(vec3(-size.x * 0.5,  size.y * 0.5, 0), &sprite_mat));
	vec3_t p3 = vec3_add(pos, vec3_transform(vec3( size.x * 0.5,  size.y * 0.5, 0), &sprite_mat));

	render_texture_t *t = &textures[texture_index];
	render_push_tris((tris_t){
		.vertices = {
			{.pos = p0, .uv = {0, 0}, .color = color},
			{.pos = p1, .uv = {0 + t->size.x ,0}, .color = color},
			{.pos = p2, .uv = {0, 0 + t->size.y}, .color = color},
		}
	}, texture_index);
	render_push_tris((tris_t){
		.vertices = {
			{.pos = p2, .uv = {0, 0 + t->size.y}, .color = color},
			{.pos = p1, .uv = {0 + t->size.x, 0}, .color = color},
			{.pos = p3, .uv = {0 + t->size.x, 0 + t->size.y}, .color = color},
		}
	}, texture_index);
}

void render_push_2d(vec2i_t pos, vec2i_t size, rgba_t color, uint16_t texture_index) {
	render_push_2d_tile(pos, vec2i(0, 0), render_texture_size(texture_index), size, color, texture_index);
}

void render_push_2d_tile(vec2i_t pos, vec2i_t uv_offset, vec2i_t uv_size, vec2i_t size, rgba_t color, uint16_t texture_index) {
	error_if(texture_index >= textures_len, "Invalid texture %d", texture_index);
	render_push_tris((tris_t){
		.vertices = {
			{.pos = {pos.x, pos.y + size.y, 0}, .uv = {uv_offset.x , uv_offset.y + uv_size.y}, .color = color},
			{.pos = {pos.x + size.x, pos.y, 0}, .uv = {uv_offset.x +  uv_size.x, uv_offset.y}, .color = color},
			{.pos = {pos.x, pos.y, 0}, .uv = {uv_offset.x , uv_offset.y}, .color = color},
		}
	}, texture_index);

	render_push_tris((tris_t){
		.vertices = {
			{.pos = {pos.x + size.x, pos.y + size.y, 0}, .uv = {uv_offset.x + uv_size.x, uv_offset.y + uv_size.y}, .color = color},
			{.pos = {pos.x + size.x, pos.y, 0}, .uv = {uv_offset.x + uv_size.x, uv_offset.y}, .color = color},
			{.pos = {pos.x, pos.y + size.y, 0}, .uv = {uv_offset.x , uv_offset.y + uv_size.y}, .color = color},
		}
	}, texture_index);
}


uint16_t render_texture_create(uint32_t width, uint32_t height, rgba_t *pixels) {
	error_if(textures_len >= TEXTURES_MAX, "TEXTURES_MAX reached");

	uint32_t byte_size = width * height * sizeof(rgba_t);
	uint16_t texture_index = textures_len;
	
	textures[texture_index] = (render_texture_t){{width, height}, mem_bump(byte_size)};
	memcpy(textures[texture_index].pixels, pixels, byte_size);

	textures_len++;
	return texture_index;
}

vec2i_t render_texture_size(uint16_t texture_index) {
	error_if(texture_index >= textures_len, "Invalid texture %d", texture_index);
	return textures[texture_index].size;
}

void render_texture_replace_pixels(int16_t texture_index, rgba_t *pixels) {
	error_if(texture_index >= textures_len, "Invalid texture %d", texture_index);
	render_texture_t *t = &textures[texture_index];
	memcpy(t->pixels, pixels, t->size.x * t->size.y * sizeof(rgba_t));
}

uint16_t render_textures_len(void) {
	return textures_len;
}

void render_textures_reset(uint16_t len) {
	error_if(len > textures_len, "Invalid texture reset len %d >= %d", len, textures_len);
	textures_len = len;
}

void render_textures_dump(const char *path) {}



// Rasterizer ------------------------------------------------------------------

typedef struct {
	vec2_t p;     // Screen-space (x, y)
	float z;      // NDC Depth
	float q;      // 1/w
	vec2_t uv_q;  // uv * q
	vec4_t col_q; // rgba * q (floats 0-255)
} ss_vertex_t;

typedef struct {
	float z, q;
	vec2_t uv_q;
	vec4_t col_q;
} ss_interpolants_t;

static inline rgba_t color_mix(rgba_t in, rgba_t out) {
	float t = out.a/255.0;
	return rgba(
		lerp(in.r, out.r, t),
		lerp(in.g, out.g, t),
		lerp(in.b, out.b, t),
		255
	);
}

static inline rgba_t color_add(rgba_t in, rgba_t out) {
	float t = out.a/255.0;
	return rgba(
		min(in.r + out.r * t, 255),
		min(in.g + out.g * t, 255),
		min(in.b + out.b * t, 255),
		255
	);
}

static ss_interpolants_t lerp_it(ss_vertex_t a, ss_vertex_t b, float t) {
	return (ss_interpolants_t){
		.z	 = lerp(a.z, b.z, t),
		.q	 = lerp(a.q, b.q, t),
		.uv_q  = vec2_lerp(a.uv_q, b.uv_q, t),
		.col_q = vec4_lerp(a.col_q, b.col_q, t)
	};
}

void draw_tris(clip_tris_t t) {
	const vec4_t a = t.verts[0].clip_pos;
	const vec4_t b = t.verts[1].clip_pos;
	const vec4_t c = t.verts[2].clip_pos;

	if (cull_backface_enabled) {
		float signed_area = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
		if (signed_area <= 0.0f) {
			return;
		}
	}

	// Interpolating vertex colors is surprisingly expensive, so let's check
	// if we really need to do that.
	bool is_flat = 
		t.verts[0].color.x == t.verts[1].color.x && t.verts[0].color.y == t.verts[1].color.y && 
		t.verts[0].color.z == t.verts[1].color.z && t.verts[0].color.w == t.verts[1].color.w &&
		t.verts[0].color.x == t.verts[2].color.x && t.verts[0].color.y == t.verts[2].color.y && 
		t.verts[0].color.z == t.verts[2].color.z && t.verts[0].color.w == t.verts[2].color.w;

	float hw = screen_size.x * 0.5f;
	float hh = screen_size.y * 0.5f;

	float q0 = 1.0f / t.verts[0].clip_pos.w;
	float q1 = 1.0f / t.verts[1].clip_pos.w;
	float q2 = 1.0f / t.verts[2].clip_pos.w;

	ss_vertex_t v[3] = {
		{
			.p = vec2(a.x * hw + hw, hh - a.y * hh), .z = a.z, .q = q0, 
			.uv_q = vec2_mulf(t.verts[0].uv, q0), 
			.col_q = vec4_mulf(t.verts[0].color, q0)
		},
		{
			.p = vec2(b.x * hw + hw, hh - b.y * hh), .z = b.z, .q = q1, 
			.uv_q = vec2_mulf(t.verts[1].uv, q1), 
			.col_q = vec4_mulf(t.verts[1].color, q1)
		},
		{
			.p = vec2(c.x * hw + hw, hh - c.y * hh), .z = c.z, .q = q2, 
			.uv_q = vec2_mulf(t.verts[2].uv, q2), 
			.col_q = vec4_mulf(t.verts[2].color, q2)
		}
	};


	if (v[1].p.y < v[0].p.y) { swap(v[0], v[1]); }
	if (v[2].p.y < v[1].p.y) { swap(v[1], v[2]); }
	if (v[1].p.y < v[0].p.y) { swap(v[0], v[1]); }

	float total_dy = v[2].p.y - v[0].p.y;
	if (total_dy <= 0.0f) {
		return;
	}

	float depth_bias = 0.5f + (depth_offset / FAR_PLANE);
	int32_t y_start = max((int32_t)ceilf(v[0].p.y - 0.5f), 0);
	int32_t y_end   = min((int32_t)floorf(v[2].p.y - 0.5f), (int32_t)screen_size.y - 1);

	for (int32_t y = y_start; y <= y_end; y++) {
		float py = y + 0.5f;
		
		ss_interpolants_t it_left = lerp_it(v[0], v[2], (py - v[0].p.y) / total_dy);
		float x_left = lerp(v[0].p.x, v[2].p.x, (py - v[0].p.y) / total_dy);

		ss_interpolants_t it_right;
		float x_right;
		if (py < v[1].p.y) {
			float t = (py - v[0].p.y) / (v[1].p.y - v[0].p.y);
			it_right = lerp_it(v[0], v[1], t);
			x_right  = lerp(v[0].p.x, v[1].p.x, t);
		} else {
			float t = (py - v[1].p.y) / (v[2].p.y - v[1].p.y);
			it_right = lerp_it(v[1], v[2], t);
			x_right  = lerp(v[1].p.x, v[2].p.x, t);
		}

		if (x_left > x_right) {
			swap(x_left, x_right);
			swap(it_left, it_right);
		}

		int32_t x_s = max((int32_t)ceilf(x_left - 0.5f), 0);
		int32_t x_e = min((int32_t)floorf(x_right - 0.5f), (int32_t)screen_size.x - 1);
		float span_dx = x_right - x_left;
		if (x_s > x_e || span_dx <= 0.0f) {
			continue;
		}

		float inv_span = 1.0f / span_dx;
		ss_interpolants_t it_gradient = {
			.z	 = (it_right.z - it_left.z) * inv_span,
			.q	 = (it_right.q - it_left.q) * inv_span,
			.uv_q  = vec2_mulf(vec2_sub(it_right.uv_q, it_left.uv_q), inv_span),
			.col_q = vec4_mulf(vec4_sub(it_right.col_q, it_left.col_q), inv_span)
		};

		float offset = (x_s + 0.5f) - x_left;
		ss_interpolants_t it_current = {
			.z	 = it_left.z + it_gradient.z * offset,
			.q	 = it_left.q + it_gradient.q * offset,
			.uv_q  = vec2_add(it_left.uv_q, vec2_mulf(it_gradient.uv_q, offset)),
			.col_q = vec4_add(it_left.col_q, vec4_mulf(it_gradient.col_q, offset))
		};
		
		rgba_t *screen_ptr = screen_buffer + screen_ppr * y + x_s;
		float *depth_ptr = depth_buffer + screen_size.x * y + x_s;
		int32_t line_len = x_e - x_s;
		for (int32_t i = 0; i <= line_len; i++) {
			float depth = clamp(it_current.z * 0.5f + depth_bias, 0.0f, 1.0f);
			if ((!depth_test_enabled || depth <= depth_ptr[i]) && it_current.q > 1e-6f) {
				float iq = 1.0f / it_current.q;

				vec2_t uv = vec2_mulf(it_current.uv_q, iq);
				int32_t tx = min((uint32_t)uv.x, t.texture->size.x - 1);
				int32_t ty = min((uint32_t)uv.y, t.texture->size.y - 1);
				rgba_t texel = t.texture->pixels[ty * t.texture->size.x + tx];
				
				if (texel.a > 0) {
					vec4_t c  = is_flat ? t.verts[0].color : vec4_mulf(it_current.col_q, iq);
					rgba_t color = {
						.r = (uint8_t)(texel.r * c.x),
						.g = (uint8_t)(texel.g * c.y),
						.b = (uint8_t)(texel.b * c.z),
						.a = (uint8_t)(texel.a * c.w + 0.5)
					};

					screen_ptr[i] = blend_mode == RENDER_BLEND_LIGHTER
						? color_add(screen_ptr[i], color)
						: color.a == 255 
							? color 
							: color_mix(screen_ptr[i], color);

					if (depth_write_enabled) {
						depth_ptr[i] = depth;
					}
				}
			}
			it_current.z += it_gradient.z;
			it_current.q += it_gradient.q;
			it_current.uv_q = vec2_add(it_current.uv_q, it_gradient.uv_q);
			it_current.col_q = vec4_add(it_current.col_q, it_gradient.col_q);
		}
	}
}
