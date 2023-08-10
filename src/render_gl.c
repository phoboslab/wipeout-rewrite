
// macOS
#if defined(__APPLE__) && defined(__MACH__)
	#include <OpenGL/gl.h>
	#include <OpenGL/glext.h>

	void glCreateTextures(GLuint ignored, GLsizei n, GLuint *name) {
		glGenTextures(1, name);
	}
	#define glGenVertexArrays glGenVertexArraysAPPLE
	#define glBindVertexArray glBindVertexArrayAPPLE
	#define glDeleteVertexArrays glDeleteVertexArraysAPPLE

// Linux
#elif defined(__unix__)
	#include <GL/glew.h>
	
	#ifdef __EMSCRIPTEN__
		void glCreateTextures(GLuint ignored, GLsizei n, GLuint *name) {
			glGenTextures(1, name);
		}
	#endif

// WINDOWS
#else
	#include <windows.h>

	#define GL3_PROTOTYPES 1
	#include <glew.h>
	#pragma comment(lib, "glew32.lib")

	#include <gl/GL.h>
	#pragma comment(lib, "opengl32.lib")
#endif



#include "libs/stb_image_write.h"

#include "render.h"
#include "mem.h"
#include "utils.h"


#define NEAR_PLANE 16.0
#define FAR_PLANE 262144.0

#define ATLAS_SIZE 64
#define ATLAS_GRID 32
#define ATLAS_BORDER 16

#define RENDER_TRIS_BUFFER_CAPACITY 2048
#define TEXTURES_MAX 1024

// WebGL (GLES) needs the `precision` to be set, OpenGL 2.something 
// doesn't like that...
#ifdef __EMSCRIPTEN__
	#define SHADER_SOURCE(...) "precision highp float;" #__VA_ARGS__
#else
	#define SHADER_SOURCE(...) #__VA_ARGS__
#endif
	

typedef struct {
	vec2i_t offset;
	vec2i_t size;
} render_texture_t;

uint16_t RENDER_NO_TEXTURE;

static GLuint u_color;
static GLuint u_view;
static GLuint u_model;
static GLuint u_projection;
static GLuint u_screen;
static GLuint u_camera_pos;
static GLuint u_fade;

static GLuint a_pos;
static GLuint a_uv;
static GLuint a_color;

static GLuint vbo;

static tris_t tris_buffer[RENDER_TRIS_BUFFER_CAPACITY];
static uint32_t tris_len = 0;

static vec2i_t screen_size;

static uint32_t atlas_map[ATLAS_SIZE] = {0};
static GLuint atlas_texture = 0;
static render_blend_mode_t blend_mode = RENDER_BLEND_NORMAL;

static mat4_t projection_mat_2d = mat4_identity();
static mat4_t projection_mat_3d = mat4_identity();
static mat4_t sprite_mat = mat4_identity();
static mat4_t view_mat = mat4_identity();


static render_texture_t textures[TEXTURES_MAX];
static uint32_t textures_len = 0;
static bool texture_mipmap_is_dirty = false;

static const char * const VERTEX_SHADER = SHADER_SOURCE(
	attribute vec3 pos;
	attribute vec2 uv;
	attribute vec4 color;

	varying vec4 v_color;
	varying vec2 v_uv;
	uniform mat4 view;
	uniform mat4 model;
	uniform mat4 projection;
	uniform vec2 screen;
	uniform vec3 camera_pos;
	uniform vec2 fade;
	
	void main() {
		gl_Position = projection * view * model * vec4(pos, 1.0);
		gl_Position.xy += screen.xy * gl_Position.w;
		v_color = color;
		v_color.a *= smoothstep(
			fade.y, fade.x, // fadeout far, near
			length(vec4(camera_pos, 1.0) - model * vec4(pos, 1.0))
		);
		v_uv = uv;
		v_uv = uv / 2048.0; // ATLAS_GRID * ATLAS_SIZE
	}
);

static const char * const FRAGMENT_SHADER_YCRCB = SHADER_SOURCE(
	varying vec4 v_color;
	varying vec2 v_uv;
	uniform sampler2D texture;
	void main() {
		vec4 tex_color = texture2D(texture, v_uv);
		vec4 color = tex_color * v_color;
		if (color.a == 0.0) {
			discard;
		}
		color.rgb = color.rgb * 2.0;
		gl_FragColor = color;
	}
);


#define render_bind_va_f(index, container, member, start) \
	glVertexAttribPointer( \
		index, member_size(container, member)/sizeof(float), GL_FLOAT, false, \
		sizeof(container), \
		(GLvoid*)(offsetof(container, member) + start) \
	)

#define render_bind_va_color(index, container, member, start) \
	glVertexAttribPointer( \
		index, 4,  GL_UNSIGNED_BYTE, true, \
		sizeof(container), \
		(GLvoid*)(offsetof(container, member) + start) \
	)

static void render_flush();
static GLuint compile_shader(GLenum type, const char *source);

static GLuint compile_shader(GLenum type, const char *source) {
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	
	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		int log_written;
		char log[256];
		glGetShaderInfoLog(shader, 256, &log_written, log);
		die("Error compiling shader: %s\n", log);
	}
	return shader;
}

// static void gl_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
// 	puts(message);
// }

void render_init(vec2i_t size) {	
	#if defined(__APPLE__) && defined(__MACH__)
		// OSX
		// (nothing to do here)
	#else
		// Windows, Linux
		glewExperimental = GL_TRUE;
		glewInit();
	#endif

	// glEnable(GL_DEBUG_OUTPUT);
	// glDebugMessageCallback(gl_message_callback, NULL);
	// glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);


	// Atlas Texture

	glCreateTextures(GL_TEXTURE_2D, 1, &atlas_texture);
	glBindTexture(GL_TEXTURE_2D, atlas_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, RENDER_USE_MIPMAPS ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	float anisotropy = 0;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisotropy);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);

	uint32_t tw = ATLAS_SIZE * ATLAS_GRID;
	uint32_t th = ATLAS_SIZE * ATLAS_GRID;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tw, th, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	printf("atlas texture %5d\n", atlas_texture);


	// Shaders

	GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER_YCRCB);
	GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, VERTEX_SHADER);
	GLuint shader_program = glCreateProgram();

	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);
	glUseProgram(shader_program);

	u_color = glGetUniformLocation(shader_program, "color");
	u_view = glGetUniformLocation(shader_program, "view");
	u_model = glGetUniformLocation(shader_program, "model");
	u_projection = glGetUniformLocation(shader_program, "projection");
	u_screen = glGetUniformLocation(shader_program, "screen");
	u_camera_pos = glGetUniformLocation(shader_program, "camera_pos");
	u_fade = glGetUniformLocation(shader_program, "fade");

	a_pos = glGetAttribLocation(shader_program, "pos");
	a_uv = glGetAttribLocation(shader_program, "uv");
	a_color = glGetAttribLocation(shader_program, "color");

	// Tris buffer

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	GLuint va;
	glGenVertexArrays(1, &va);
	glBindVertexArray(va);


	// Defaults

	glEnableVertexAttribArray(a_pos);
	glEnableVertexAttribArray(a_uv);
	glEnableVertexAttribArray(a_color);

	render_bind_va_f(a_pos, vertex_t, pos, 0);
	render_bind_va_f(a_uv, vertex_t, uv, 0);
	render_bind_va_color(a_color, vertex_t, color, 0);

	render_resize(size);
	render_set_view(vec3(0, 0, 0), vec3(0, 0, 0));
	render_set_model_mat(&mat4_identity());

	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	// Create white texture

	rgba_t white_pixels[4] = {
		rgba(128,128,128,255), rgba(128,128,128,255),
		rgba(128,128,128,255), rgba(128,128,128,255)
	};
	RENDER_NO_TEXTURE = render_texture_create(2, 2, white_pixels);
}

void render_cleanup() {
	// TODO
}


static void render_setup_2d_projection_mat() {
	float near = -1;
	float far = 1;
	float left = 0;
	float right = screen_size.x;
	float bottom = screen_size.y;
	float top = 0;
	float lr = 1 / (left - right);
	float bt = 1 / (bottom - top);
	float nf = 1 / (near - far);
  	projection_mat_2d = mat4(
		-2 * lr,  0,  0,  0,
		0,  -2 * bt,  0,  0,
		0,        0,  2 * nf,    0, 
		(left + right) * lr, (top + bottom) * bt, (far + near) * nf, 1
	);
}

static void render_setup_3d_projection_mat() {
	// wipeout has a horizontal fov of 90deg, but we want the fov to be fixed 
	// for the vertical axis, so that widescreen displays just have a wider 
	// view. For the original 4/3 aspect ratio this equates to a vertial fov
	// of 73.75deg.
	float aspect = (float)screen_size.x / (float)screen_size.y;
	float fov = (73.75 / 180.0) * 3.14159265358;
	float f = 1.0 / tan(fov / 2);
	float nf = 1.0 / (NEAR_PLANE - FAR_PLANE);
	projection_mat_3d = mat4(
		f / aspect, 0, 0, 0,
		0, f, 0, 0, 
		0, 0, (FAR_PLANE + NEAR_PLANE) * nf, -1, 
		0, 0, 2 * FAR_PLANE * NEAR_PLANE * nf, 0
	);
}

void render_resize(vec2i_t size) {
	glViewport(0, 0, size.x, size.y);
	screen_size = size;

	render_setup_2d_projection_mat();
	render_setup_3d_projection_mat();
}

vec2i_t render_size() {
	return screen_size;
}

void render_frame_prepare() {
	glUniform2f(u_screen, 0, 0);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(true);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST); 
}

void render_frame_end() {	
	render_flush();
}

void render_flush() {
	if (tris_len == 0) {
		return;
	}

	if (texture_mipmap_is_dirty) {
		glGenerateMipmap(GL_TEXTURE_2D);
		texture_mipmap_is_dirty = false;
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(tris_t) * tris_len, tris_buffer, GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, tris_len * 3);
	tris_len = 0;
}


void render_set_view(vec3_t pos, vec3_t angles) {
	render_flush();
	render_set_depth_write(true);
	render_set_depth_test(true);

	view_mat = mat4_identity();
	mat4_set_translation(&view_mat, vec3(0, 0, 0));
	mat4_set_roll_pitch_yaw(&view_mat, vec3(angles.x, -angles.y + M_PI, angles.z + M_PI));
	mat4_translate(&view_mat, vec3_inv(pos));
	mat4_set_yaw_pitch_roll(&sprite_mat, vec3(-angles.x, angles.y - M_PI, 0));

	render_set_model_mat(&mat4_identity());

	render_flush();
	glUniformMatrix4fv(u_view, 1, false, view_mat.m);
	glUniformMatrix4fv(u_projection, 1, false, projection_mat_3d.m);
	glUniform3f(u_camera_pos, pos.x, pos.y, pos.z);
	glUniform2f(u_fade, RENDER_FADEOUT_NEAR, RENDER_FADEOUT_FAR);
}

void render_set_view_2d() {
	render_flush();
	render_set_depth_test(false);
	render_set_depth_write(false);

	render_set_model_mat(&mat4_identity());
	glUniform3f(u_camera_pos, 0, 0, 0);
	glUniformMatrix4fv(u_view, 1, false, mat4_identity().m);
	glUniformMatrix4fv(u_projection, 1, false, projection_mat_2d.m);
}

void render_set_model_mat(mat4_t *m) {
	render_flush();
	glUniformMatrix4fv(u_model, 1, false, m->m);
}

void render_set_depth_write(bool enabled) {
	render_flush();
	glDepthMask(enabled);
}

void render_set_depth_test(bool enabled) {
	render_flush();
	if (enabled) {
		glEnable(GL_DEPTH_TEST);
	}
	else {
		glDisable(GL_DEPTH_TEST); 
	}
}

void render_set_depth_offset(float offset) {
	render_flush();
	if (offset == 0) {
		glDisable(GL_POLYGON_OFFSET_FILL);
		return;	
	}

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(offset, 1.0);
}

void render_set_screen_position(vec2_t pos) {
	render_flush();
	glUniform2f(u_screen, pos.x, -pos.y);
}

void render_set_blend_mode(render_blend_mode_t new_mode) {
	if (new_mode == blend_mode) {
		return;
	}
	render_flush();

	blend_mode = new_mode;
	if (blend_mode == RENDER_BLEND_NORMAL) {
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else if (blend_mode == RENDER_BLEND_LIGHTER) {
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	}
}

void render_set_cull_backface(bool enabled) {
	render_flush();
	if (enabled) {
		glEnable(GL_CULL_FACE);
	}
	else {
		glDisable(GL_CULL_FACE);
	}
}




vec3_t render_transform(vec3_t pos) {
	return vec3_transform(vec3_transform(pos, &view_mat), &projection_mat_3d);
}

void render_push_tris(tris_t tris, uint16_t texture_index) {
	error_if(texture_index >= textures_len, "Invalid texture %d", texture_index);
	
	if (tris_len >= RENDER_TRIS_BUFFER_CAPACITY) {
		render_flush();
	}

	render_texture_t *t = &textures[texture_index];

	for (int i = 0; i < 3; i++) {
		tris.vertices[i].uv.x += t->offset.x;
		tris.vertices[i].uv.y += t->offset.y;
	}
	tris_buffer[tris_len++] = tris;
}

void render_push_sprite(vec3_t pos, vec2i_t size, rgba_t color, uint16_t texture_index) {
	error_if(texture_index >= textures_len, "Invalid texture %d", texture_index);

	vec3_t p1 = vec3_add(pos, vec3_transform(vec3(-size.x * 0.5, -size.y * 0.5, 0), &sprite_mat));
	vec3_t p2 = vec3_add(pos, vec3_transform(vec3( size.x * 0.5, -size.y * 0.5, 0), &sprite_mat));
	vec3_t p3 = vec3_add(pos, vec3_transform(vec3(-size.x * 0.5,  size.y * 0.5, 0), &sprite_mat));
	vec3_t p4 = vec3_add(pos, vec3_transform(vec3( size.x * 0.5,  size.y * 0.5, 0), &sprite_mat));

	render_texture_t *t = &textures[texture_index];
	render_push_tris((tris_t){
		.vertices = {
			{
				.pos = p1,
				.uv = {0, 0},
				.color = color
			},
			{
				.pos = p2,
				.uv = {0 + t->size.x ,0},
				.color = color
			},
			{
				.pos = p3,
				.uv = {0, 0 + t->size.y},
				.color = color
			},
		}
	}, texture_index);
	render_push_tris((tris_t){
		.vertices = {
			{
				.pos = p3,
				.uv = {0, 0 + t->size.y},
				.color = color
			},
			{
				.pos = p2,
				.uv = {0 + t->size.x, 0},
				.color = color
			},
			{
				.pos = p4,
				.uv = {0 + t->size.x, 0 + t->size.y},
				.color = color
			},
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
			{
				.pos = {pos.x, pos.y + size.y, 0},
				.uv = {uv_offset.x , uv_offset.y + uv_size.y},
				.color = color
			},
			{
				.pos = {pos.x + size.x, pos.y, 0},
				.uv = {uv_offset.x +  uv_size.x, uv_offset.y},
				.color = color
			},
			{
				.pos = {pos.x, pos.y, 0},
				.uv = {uv_offset.x , uv_offset.y},
				.color = color
			},
		}
	}, texture_index);

	render_push_tris((tris_t){
		.vertices = {
			{
				.pos = {pos.x + size.x, pos.y + size.y, 0},
				.uv = {uv_offset.x + uv_size.x, uv_offset.y + uv_size.y},
				.color = color
			},
			{
				.pos = {pos.x + size.x, pos.y, 0},
				.uv = {uv_offset.x + uv_size.x, uv_offset.y},
				.color = color
			},
			{
				.pos = {pos.x, pos.y + size.y, 0},
				.uv = {uv_offset.x , uv_offset.y + uv_size.y},
				.color = color
			},
		}
	}, texture_index);
}


uint16_t render_texture_create(uint32_t tw, uint32_t th, rgba_t *pixels) {
	error_if(textures_len >= TEXTURES_MAX, "TEXTURES_MAX reached");

	uint32_t bw = tw + ATLAS_BORDER * 2;
	uint32_t bh = th + ATLAS_BORDER * 2;

	// Find a position in the atlas for this texture (with added border)
	uint32_t grid_width = (bw + ATLAS_GRID - 1) / ATLAS_GRID;
	uint32_t grid_height = (bh + ATLAS_GRID - 1) / ATLAS_GRID;
	uint32_t grid_x = 0;
	uint32_t grid_y = ATLAS_SIZE - grid_height + 1;

	for (uint32_t cx = 0; cx < ATLAS_SIZE - grid_width; cx++) {
		if (atlas_map[cx] >= grid_y) {
			continue;
		}

		uint32_t cy = atlas_map[cx];
		bool is_best = true;

		for (uint32_t bx = cx; bx < cx + grid_width; bx++) {
			if (atlas_map[bx] >= grid_y) {
				is_best = false;
				cx = bx;
				break;
			}
			if (atlas_map[bx] > cy) {
				cy = atlas_map[bx];
			}
		}
		if (is_best) {
			grid_y = cy;
			grid_x = cx;
		}
	}

	error_if(grid_y + grid_height > ATLAS_SIZE, "Render atlas ran out of space");

	for (uint32_t cx = grid_x; cx < grid_x + grid_width; cx++) {
		atlas_map[cx] = grid_y + grid_height;
	}

	// Add the border pixels for this texture
	rgba_t *pb = mem_temp_alloc(sizeof(rgba_t) * bw * bh);

	if (tw && th) {
		// Top border
		for (int32_t y = 0; y < ATLAS_BORDER; y++) {
			memcpy(pb + bw * y + ATLAS_BORDER, pixels, tw * sizeof(rgba_t));
		}

		// Bottom border
		for (int32_t y = 0; y < ATLAS_BORDER; y++) {
			memcpy(pb + bw * (bh - ATLAS_BORDER + y) + ATLAS_BORDER, pixels + tw * (th-1), tw * sizeof(rgba_t));
		}
		
		// Left border
		for (int32_t y = 0; y < bh; y++) {
			for (int32_t x = 0; x < ATLAS_BORDER; x++) {
				pb[y * bw + x] = pixels[clamp(y-ATLAS_BORDER, 0, th-1) * tw];
			}
		}

		// Right border
		for (int32_t y = 0; y < bh; y++) {
			for (int32_t x = 0; x < ATLAS_BORDER; x++) {
				pb[y * bw + x + bw - ATLAS_BORDER] = pixels[tw - 1 + clamp(y-ATLAS_BORDER, 0, th-1) * tw];
			}
		}

		// Texture
		for (int32_t y = 0; y < th; y++) {
			memcpy(pb + bw * (y + ATLAS_BORDER) + ATLAS_BORDER, pixels + tw * y, tw * sizeof(rgba_t));
		}
	}

	uint32_t x = grid_x * ATLAS_GRID;
	uint32_t y = grid_y * ATLAS_GRID;
	glBindTexture(GL_TEXTURE_2D, atlas_texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, bw, bh, GL_RGBA, GL_UNSIGNED_BYTE, pb);
	mem_temp_free(pb);


	texture_mipmap_is_dirty = RENDER_USE_MIPMAPS;
	uint16_t texture_index = textures_len;
	textures_len++;
	textures[texture_index] = (render_texture_t){ {x + ATLAS_BORDER, y + ATLAS_BORDER}, {tw, th} };

	printf("inserted atlas texture (%3dx%3d) at (%3d,%3d)\n", tw, th, grid_x, grid_y);
	return texture_index;
}

vec2i_t render_texture_size(uint16_t texture_index) {
	error_if(texture_index >= textures_len, "Invalid texture %d", texture_index);
	return textures[texture_index].size;
}

void render_texture_replace_pixels(int16_t texture_index, rgba_t *pixels) {
	error_if(texture_index >= textures_len, "Invalid texture %d", texture_index);

	render_texture_t *t = &textures[texture_index];
	glBindTexture(GL_TEXTURE_2D, atlas_texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, t->offset.x, t->offset.y, t->size.x, t->size.y, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}

uint16_t render_textures_len() {
	return textures_len;
}

void render_textures_reset(uint16_t len) {
	error_if(len > textures_len, "Invalid texture reset len %d >= %d", len, textures_len);
	render_flush();

	textures_len = len;
	clear(atlas_map);

	// Clear completely and recreate the default white texture
	if (len == 0) {
		rgba_t white_pixels[4] = {
			rgba(128,128,128,255), rgba(128,128,128,255),
			rgba(128,128,128,255), rgba(128,128,128,255)
		};
		RENDER_NO_TEXTURE = render_texture_create(2, 2, white_pixels);
		return;
	}

	// Replay all texture grid insertions up to the reset len
	for (int i = 0; i < textures_len; i++) {
		uint32_t grid_x = (textures[i].offset.x - ATLAS_BORDER) / ATLAS_GRID;
		uint32_t grid_y = (textures[i].offset.y - ATLAS_BORDER) / ATLAS_GRID;
		uint32_t grid_width = (textures[i].size.x + ATLAS_BORDER * 2 + ATLAS_GRID - 1) / ATLAS_GRID;
		uint32_t grid_height = (textures[i].size.y + ATLAS_BORDER * 2 + ATLAS_GRID - 1) / ATLAS_GRID;
		for (uint32_t cx = grid_x; cx < grid_x + grid_width; cx++) {
			atlas_map[cx] = grid_y + grid_height;
		}
	}
}

void render_textures_dump(const char *path) {
	int width = ATLAS_SIZE * ATLAS_GRID;
	int height = ATLAS_SIZE * ATLAS_GRID;
	rgba_t *pixels = malloc(sizeof(rgba_t) * width * height);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	stbi_write_png(path, width, height, 4, pixels, 0);
	free(pixels);
}
