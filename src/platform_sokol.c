#include "platform.h"
#include "system.h"
#include "utils.h"
#include "mem.h"

#if defined(RENDERER_GL)
	#ifdef __EMSCRIPTEN__
		#define SOKOL_GLES3
	#else
		#define SOKOL_GLCORE33
	#endif
#else
	#error "Unsupported renderer for platform SOKOL"
#endif

#define SOKOL_IMPL
#include <sokol_audio.h>
#include <sokol_time.h>
#include <sokol_app.h>
#include "input.h"

// FIXME: we should figure out the actual path where the executabe resides,
// instead of just assuming it's in the pwd
#ifdef PATH_ASSETS
	static const char *path_assets = TOSTRING(PATH_ASSETS);
#else
	static const char *path_assets = "";
#endif

#ifdef PATH_USERDATA
	static const char *path_userdata = TOSTRING(PATH_USERDATA);
#else
	static const char *path_userdata = "";
#endif

static char *temp_path;

static const uint8_t keyboard_map[] = {
	[SAPP_KEYCODE_SPACE] = INPUT_KEY_SPACE,
	[SAPP_KEYCODE_APOSTROPHE] = INPUT_KEY_APOSTROPHE,
	[SAPP_KEYCODE_COMMA] = INPUT_KEY_COMMA,
	[SAPP_KEYCODE_MINUS] = INPUT_KEY_MINUS,
	[SAPP_KEYCODE_PERIOD] = INPUT_KEY_PERIOD,
	[SAPP_KEYCODE_SLASH] = INPUT_KEY_SLASH,
	[SAPP_KEYCODE_0] = INPUT_KEY_0,
	[SAPP_KEYCODE_1] = INPUT_KEY_1,
	[SAPP_KEYCODE_2] = INPUT_KEY_2,
	[SAPP_KEYCODE_3] = INPUT_KEY_3,
	[SAPP_KEYCODE_4] = INPUT_KEY_4,
	[SAPP_KEYCODE_5] = INPUT_KEY_5,
	[SAPP_KEYCODE_6] = INPUT_KEY_6,
	[SAPP_KEYCODE_7] = INPUT_KEY_7,
	[SAPP_KEYCODE_8] = INPUT_KEY_8,
	[SAPP_KEYCODE_9] = INPUT_KEY_9,
	[SAPP_KEYCODE_SEMICOLON] = INPUT_KEY_SEMICOLON,
	[SAPP_KEYCODE_EQUAL] = INPUT_KEY_EQUALS,
	[SAPP_KEYCODE_A] = INPUT_KEY_A,
	[SAPP_KEYCODE_B] = INPUT_KEY_B,
	[SAPP_KEYCODE_C] = INPUT_KEY_C,
	[SAPP_KEYCODE_D] = INPUT_KEY_D,
	[SAPP_KEYCODE_E] = INPUT_KEY_E,
	[SAPP_KEYCODE_F] = INPUT_KEY_F,
	[SAPP_KEYCODE_G] = INPUT_KEY_G,
	[SAPP_KEYCODE_H] = INPUT_KEY_H,
	[SAPP_KEYCODE_I] = INPUT_KEY_I,
	[SAPP_KEYCODE_J] = INPUT_KEY_J,
	[SAPP_KEYCODE_K] = INPUT_KEY_K,
	[SAPP_KEYCODE_L] = INPUT_KEY_L,
	[SAPP_KEYCODE_M] = INPUT_KEY_M,
	[SAPP_KEYCODE_N] = INPUT_KEY_N,
	[SAPP_KEYCODE_O] = INPUT_KEY_O,
	[SAPP_KEYCODE_P] = INPUT_KEY_P,
	[SAPP_KEYCODE_Q] = INPUT_KEY_Q,
	[SAPP_KEYCODE_R] = INPUT_KEY_R,
	[SAPP_KEYCODE_S] = INPUT_KEY_S,
	[SAPP_KEYCODE_T] = INPUT_KEY_T,
	[SAPP_KEYCODE_U] = INPUT_KEY_U,
	[SAPP_KEYCODE_V] = INPUT_KEY_V,
	[SAPP_KEYCODE_W] = INPUT_KEY_W,
	[SAPP_KEYCODE_X] = INPUT_KEY_X,
	[SAPP_KEYCODE_Y] = INPUT_KEY_Y,
	[SAPP_KEYCODE_Z] = INPUT_KEY_Z,
	[SAPP_KEYCODE_LEFT_BRACKET] = INPUT_KEY_LEFTBRACKET,
	[SAPP_KEYCODE_BACKSLASH] = INPUT_KEY_BACKSLASH,
	[SAPP_KEYCODE_RIGHT_BRACKET] = INPUT_KEY_RIGHTBRACKET,
	[SAPP_KEYCODE_GRAVE_ACCENT] = INPUT_KEY_TILDE,
	[SAPP_KEYCODE_WORLD_1] = INPUT_INVALID,				// not implemented
	[SAPP_KEYCODE_WORLD_2] = INPUT_INVALID,				// not implemented
	[SAPP_KEYCODE_ESCAPE] = INPUT_KEY_ESCAPE,
	[SAPP_KEYCODE_ENTER] = INPUT_KEY_RETURN,
	[SAPP_KEYCODE_TAB] = INPUT_KEY_TAB,
	[SAPP_KEYCODE_BACKSPACE] = INPUT_KEY_BACKSPACE,
	[SAPP_KEYCODE_INSERT] = INPUT_KEY_INSERT,
	[SAPP_KEYCODE_DELETE] = INPUT_KEY_DELETE,
	[SAPP_KEYCODE_RIGHT] = INPUT_KEY_RIGHT,
	[SAPP_KEYCODE_LEFT] = INPUT_KEY_LEFT,
	[SAPP_KEYCODE_DOWN] = INPUT_KEY_DOWN,
	[SAPP_KEYCODE_UP] = INPUT_KEY_UP,
	[SAPP_KEYCODE_PAGE_UP] = INPUT_KEY_PAGEUP,
	[SAPP_KEYCODE_PAGE_DOWN] = INPUT_KEY_PAGEDOWN,
	[SAPP_KEYCODE_HOME] = INPUT_KEY_HOME,
	[SAPP_KEYCODE_END] = INPUT_KEY_END,
	[SAPP_KEYCODE_CAPS_LOCK] = INPUT_KEY_CAPSLOCK,
	[SAPP_KEYCODE_SCROLL_LOCK] = INPUT_KEY_SCROLLLOCK,
	[SAPP_KEYCODE_NUM_LOCK] = INPUT_KEY_NUMLOCK,
	[SAPP_KEYCODE_PRINT_SCREEN] = INPUT_KEY_PRINTSCREEN,
	[SAPP_KEYCODE_PAUSE] = INPUT_KEY_PAUSE,
	[SAPP_KEYCODE_F1] = INPUT_KEY_F1,
	[SAPP_KEYCODE_F2] = INPUT_KEY_F2,
	[SAPP_KEYCODE_F3] = INPUT_KEY_F3,
	[SAPP_KEYCODE_F4] = INPUT_KEY_F4,
	[SAPP_KEYCODE_F5] = INPUT_KEY_F5,
	[SAPP_KEYCODE_F6] = INPUT_KEY_F6,
	[SAPP_KEYCODE_F7] = INPUT_KEY_F7,
	[SAPP_KEYCODE_F8] = INPUT_KEY_F8,
	[SAPP_KEYCODE_F9] = INPUT_KEY_F9,
	[SAPP_KEYCODE_F10] = INPUT_KEY_F10,
	[SAPP_KEYCODE_F11] = INPUT_KEY_F11,
	[SAPP_KEYCODE_F12] = INPUT_KEY_F12,
	[SAPP_KEYCODE_F13] = INPUT_INVALID, 				// not implemented
	[SAPP_KEYCODE_F14] = INPUT_INVALID, 				// not implemented
	[SAPP_KEYCODE_F15] = INPUT_INVALID, 				// not implemented
	[SAPP_KEYCODE_F16] = INPUT_INVALID, 				// not implemented
	[SAPP_KEYCODE_F17] = INPUT_INVALID, 				// not implemented
	[SAPP_KEYCODE_F18] = INPUT_INVALID, 				// not implemented
	[SAPP_KEYCODE_F19] = INPUT_INVALID, 				// not implemented
	[SAPP_KEYCODE_F20] = INPUT_INVALID, 				// not implemented
	[SAPP_KEYCODE_F21] = INPUT_INVALID, 				// not implemented
	[SAPP_KEYCODE_F22] = INPUT_INVALID, 				// not implemented
	[SAPP_KEYCODE_F23] = INPUT_INVALID, 				// not implemented
	[SAPP_KEYCODE_F24] = INPUT_INVALID, 				// not implemented
	[SAPP_KEYCODE_F25] = INPUT_INVALID, 				// not implemented
	[SAPP_KEYCODE_KP_0] = INPUT_KEY_KP_0,
	[SAPP_KEYCODE_KP_1] = INPUT_KEY_KP_1,
	[SAPP_KEYCODE_KP_2] = INPUT_KEY_KP_2,
	[SAPP_KEYCODE_KP_3] = INPUT_KEY_KP_3,
	[SAPP_KEYCODE_KP_4] = INPUT_KEY_KP_4,
	[SAPP_KEYCODE_KP_5] = INPUT_KEY_KP_5,
	[SAPP_KEYCODE_KP_6] = INPUT_KEY_KP_6,
	[SAPP_KEYCODE_KP_7] = INPUT_KEY_KP_7,
	[SAPP_KEYCODE_KP_8] = INPUT_KEY_KP_8,
	[SAPP_KEYCODE_KP_9] = INPUT_KEY_KP_9,
	[SAPP_KEYCODE_KP_DECIMAL] = INPUT_KEY_KP_PERIOD,
	[SAPP_KEYCODE_KP_DIVIDE] = INPUT_KEY_KP_DIVIDE,
	[SAPP_KEYCODE_KP_MULTIPLY] = INPUT_KEY_KP_MULTIPLY,
	[SAPP_KEYCODE_KP_SUBTRACT] = INPUT_KEY_KP_MINUS,
	[SAPP_KEYCODE_KP_ADD] = INPUT_KEY_KP_PLUS,
	[SAPP_KEYCODE_KP_ENTER] = INPUT_KEY_KP_ENTER,
	[SAPP_KEYCODE_KP_EQUAL] = INPUT_INVALID, 			// not implemented
	[SAPP_KEYCODE_LEFT_SHIFT] = INPUT_KEY_LSHIFT,
	[SAPP_KEYCODE_LEFT_CONTROL] = INPUT_KEY_LCTRL,
	[SAPP_KEYCODE_LEFT_ALT] = INPUT_KEY_LALT,
	[SAPP_KEYCODE_LEFT_SUPER] = INPUT_INVALID, 			// not implemented
	[SAPP_KEYCODE_RIGHT_SHIFT] = INPUT_KEY_RSHIFT,
	[SAPP_KEYCODE_RIGHT_CONTROL] = INPUT_KEY_RCTRL,
	[SAPP_KEYCODE_RIGHT_ALT] = INPUT_KEY_RALT,
	[SAPP_KEYCODE_RIGHT_SUPER] = INPUT_INVALID, 		// not implemented
	[SAPP_KEYCODE_MENU] = INPUT_INVALID, 				// not implemented
};


static void (*audio_callback)(float *buffer, uint32_t len) = NULL;

void platform_exit(void) {
	sapp_quit();
}

vec2i_t platform_screen_size(void) {
	return vec2i(sapp_width(), sapp_height());
}

double platform_now(void) {
	return stm_sec(stm_now());
}

bool platform_get_fullscreen() {
	return sapp_is_fullscreen();
}

void platform_set_fullscreen(bool fullscreen) {
	if (fullscreen == sapp_is_fullscreen()) {
		return;
	}

	sapp_toggle_fullscreen();
	sapp_show_mouse(!fullscreen);
}

void platform_handle_event(const sapp_event* ev) {
	// Detect ALT+Enter press to toggle fullscreen
	if (
		ev->type == SAPP_EVENTTYPE_KEY_DOWN && 
		ev->key_code == SAPP_KEYCODE_ENTER &&
		(ev->modifiers & SAPP_MODIFIER_ALT)
	) {
		platform_set_fullscreen(!sapp_is_fullscreen());
	}

	// Input Keyboard
	else if (ev->type == SAPP_EVENTTYPE_KEY_DOWN || ev->type == SAPP_EVENTTYPE_KEY_UP) {
		float state = ev->type == SAPP_EVENTTYPE_KEY_DOWN ? 1.0 : 0.0;
		if (ev->key_code > 0 && ev->key_code < sizeof(keyboard_map)) {
			int code = keyboard_map[ev->key_code];
			input_set_button_state(code, state);
		}
	}

	else if (ev->type == SAPP_EVENTTYPE_CHAR) {
		input_textinput(ev->char_code);
	}


	// Input Gamepad Axis
	// TODO: not implemented by sokol_app itself

	// Mouse buttons
	else if (
		ev->type == SAPP_EVENTTYPE_MOUSE_DOWN ||
		ev->type == SAPP_EVENTTYPE_MOUSE_UP
	) {
		button_t button = INPUT_BUTTON_NONE;
		switch (ev->mouse_button) {
			case SAPP_MOUSEBUTTON_LEFT: button = INPUT_MOUSE_LEFT; break;
			case SAPP_MOUSEBUTTON_MIDDLE: button = INPUT_MOUSE_MIDDLE; break;
			case SAPP_MOUSEBUTTON_RIGHT: button = INPUT_MOUSE_RIGHT; break;
			default: break;
		}
		if (button != INPUT_BUTTON_NONE) {
			float state = ev->type == SAPP_EVENTTYPE_MOUSE_DOWN ? 1.0 : 0.0;
			input_set_button_state(button, state);
		}
	}

	// Mouse wheel
	else if (ev->type == SAPP_EVENTTYPE_MOUSE_SCROLL) {
		button_t button = ev->scroll_y > 0 
			? INPUT_MOUSE_WHEEL_UP
			: INPUT_MOUSE_WHEEL_DOWN;
		input_set_button_state(button, 1.0);
		input_set_button_state(button, 0.0);
	}

	// Mouse move
	else if (ev->type == SAPP_EVENTTYPE_MOUSE_MOVE) {
		input_set_mouse_pos(ev->mouse_x, ev->mouse_y);
	}

	// Window Events
	if (ev->type == SAPP_EVENTTYPE_RESIZED) {
		system_resize(vec2i(ev->window_width, ev->window_height));
	}
}

void platform_audio_callback(float* buffer, int num_frames, int num_channels) {
	if (audio_callback) {
		audio_callback(buffer, num_frames * num_channels);
	}
	else {
		memset(buffer, 0, num_frames * sizeof(float));
	}
}

void platform_set_audio_mix_cb(void (*cb)(float *buffer, uint32_t len)) {
	audio_callback = cb;
}

void platform_force_feedback(double strength, uint32_t duration) {
	// Sokol does not support haptic feedback
	(void)strength; // avoid unused variable warning
	(void)duration; // avoid unused variable warning
}

void platform_set_force_feedback(bool enable) {
	// Sokol does not support haptic feedback
	(void)enable; // avoid unused variable warning
}

FILE *platform_open_asset(const char *name, const char *mode) {
	char *path = strcat(strcpy(temp_path, path_assets), name);
	return fopen(path, mode);
}

uint8_t *platform_load_asset(const char *name, uint32_t *bytes_read) {
	char *path = strcat(strcpy(temp_path, path_assets), name);
	return file_load(path, bytes_read);
}

uint8_t *platform_load_userdata(const char *name, uint32_t *bytes_read) {
	char *path = strcat(strcpy(temp_path, path_userdata), name);
	if (!file_exists(path)) {
		*bytes_read = 0;
		return NULL;
	}
	return file_load(path, bytes_read);
}

uint32_t platform_store_userdata(const char *name, void *bytes, int32_t len) {
	char *path = strcat(strcpy(temp_path, path_userdata), name);
	return file_store(path, bytes, len);
}

void platform_cleanup() {
	system_cleanup();
	saudio_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
	temp_path = mem_bump(max(strlen(path_assets), strlen(path_userdata)) + 64);

	stm_setup();

	saudio_setup(&(saudio_desc){
		.sample_rate = 44100,
		.buffer_frames = 1024,
		.num_channels = 2,
		.stream_cb = platform_audio_callback,
	});

	return (sapp_desc) {
		.width = SYSTEM_WINDOW_WIDTH,
		.height = SYSTEM_WINDOW_HEIGHT,
		.init_cb = system_init,
		.frame_cb = system_update,
		.window_title = SYSTEM_WINDOW_NAME,
		.cleanup_cb = platform_cleanup,
		.event_cb = platform_handle_event,
		.win32_console_attach = true
	};
}
