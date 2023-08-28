#include <SDL2/SDL.h>

#include "platform.h"
#include "input.h"
#include "system.h"
#include "utils.h"
#include "mem.h"

static uint64_t perf_freq = 0;
static bool wants_to_exit = false;
static SDL_Window *window;
static SDL_AudioDeviceID audio_device;
static SDL_GameController *gamepad;
static void (*audio_callback)(float *buffer, uint32_t len) = NULL;
static char *path_assets = NULL;
static char *path_userdata = NULL;
static char *temp_path = NULL;


uint8_t platform_sdl_gamepad_map[] = {
	[SDL_CONTROLLER_BUTTON_A] = INPUT_GAMEPAD_A,
	[SDL_CONTROLLER_BUTTON_B] = INPUT_GAMEPAD_B,
	[SDL_CONTROLLER_BUTTON_X] = INPUT_GAMEPAD_X,
	[SDL_CONTROLLER_BUTTON_Y] = INPUT_GAMEPAD_Y,
	[SDL_CONTROLLER_BUTTON_BACK] = INPUT_GAMEPAD_SELECT,
	[SDL_CONTROLLER_BUTTON_GUIDE] = INPUT_GAMEPAD_HOME,
	[SDL_CONTROLLER_BUTTON_START] = INPUT_GAMEPAD_START,
	[SDL_CONTROLLER_BUTTON_LEFTSTICK] = INPUT_GAMEPAD_L_STICK_PRESS,
	[SDL_CONTROLLER_BUTTON_RIGHTSTICK] = INPUT_GAMEPAD_R_STICK_PRESS,
	[SDL_CONTROLLER_BUTTON_LEFTSHOULDER] = INPUT_GAMEPAD_L_SHOULDER,
	[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER] = INPUT_GAMEPAD_R_SHOULDER,
	[SDL_CONTROLLER_BUTTON_DPAD_UP] = INPUT_GAMEPAD_DPAD_UP,
	[SDL_CONTROLLER_BUTTON_DPAD_DOWN] = INPUT_GAMEPAD_DPAD_DOWN,
	[SDL_CONTROLLER_BUTTON_DPAD_LEFT] = INPUT_GAMEPAD_DPAD_LEFT,
	[SDL_CONTROLLER_BUTTON_DPAD_RIGHT] = INPUT_GAMEPAD_DPAD_RIGHT,
	[SDL_CONTROLLER_BUTTON_MAX] = INPUT_INVALID
};


uint8_t platform_sdl_axis_map[] = {
	[SDL_CONTROLLER_AXIS_LEFTX] = INPUT_GAMEPAD_L_STICK_LEFT,
	[SDL_CONTROLLER_AXIS_LEFTY] = INPUT_GAMEPAD_L_STICK_UP,
	[SDL_CONTROLLER_AXIS_RIGHTX] = INPUT_GAMEPAD_R_STICK_LEFT,
	[SDL_CONTROLLER_AXIS_RIGHTY] = INPUT_GAMEPAD_R_STICK_UP,
	[SDL_CONTROLLER_AXIS_TRIGGERLEFT] = INPUT_GAMEPAD_L_TRIGGER,
	[SDL_CONTROLLER_AXIS_TRIGGERRIGHT] = INPUT_GAMEPAD_R_TRIGGER,
	[SDL_CONTROLLER_AXIS_MAX] = INPUT_INVALID
};


void platform_exit() {
	wants_to_exit = true;
}

SDL_GameController *platform_find_gamepad() {
	for (int i = 0; i < SDL_NumJoysticks(); i++) {
		if (SDL_IsGameController(i)) {
			return SDL_GameControllerOpen(i);
		}
	}

	return NULL;
}


void platform_pump_events() {
	SDL_Event ev;
	while (SDL_PollEvent(&ev)) {
		// Detect ALT+Enter press to toggle fullscreen
		if (
			ev.type == SDL_KEYDOWN && 
			ev.key.keysym.scancode == SDL_SCANCODE_RETURN &&
			(ev.key.keysym.mod & (KMOD_LALT | KMOD_RALT))
		) {
			platform_set_fullscreen(!platform_get_fullscreen());
		}

		// Input Keyboard
		else if (ev.type == SDL_KEYDOWN || ev.type == SDL_KEYUP) {
			int code = ev.key.keysym.scancode;
			float state = ev.type == SDL_KEYDOWN ? 1.0 : 0.0;
			if (code >= SDL_SCANCODE_LCTRL && code <= SDL_SCANCODE_RALT) {
				int code_internal = code - SDL_SCANCODE_LCTRL + INPUT_KEY_LCTRL;
				input_set_button_state(code_internal, state);
			}
			else if (code > 0 && code < INPUT_KEY_MAX) {
				input_set_button_state(code, state);
			}
		}

		else if (ev.type == SDL_TEXTINPUT) {
			input_textinput(ev.text.text[0]);
		}

		// Gamepads connect/disconnect
		else if (ev.type == SDL_CONTROLLERDEVICEADDED) {
			gamepad = SDL_GameControllerOpen(ev.cdevice.which);
		}
		else if (ev.type == SDL_CONTROLLERDEVICEREMOVED) {
			if (gamepad && ev.cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gamepad))) {
				SDL_GameControllerClose(gamepad);
				gamepad = platform_find_gamepad();
			}
		}

		// Input Gamepad Buttons
		else if (
			ev.type == SDL_CONTROLLERBUTTONDOWN || 
			ev.type == SDL_CONTROLLERBUTTONUP
		) {
			if (ev.cbutton.button < SDL_CONTROLLER_BUTTON_MAX) {
				button_t button = platform_sdl_gamepad_map[ev.cbutton.button];
				if (button != INPUT_INVALID) {
					float state = ev.type == SDL_CONTROLLERBUTTONDOWN ? 1.0 : 0.0;
					input_set_button_state(button, state);
				}
			}
		}

		// Input Gamepad Axis
		else if (ev.type == SDL_CONTROLLERAXISMOTION) {
			float state = (float)ev.caxis.value / 32767.0;

			if (ev.caxis.axis < SDL_CONTROLLER_AXIS_MAX) {
				int code = platform_sdl_axis_map[ev.caxis.axis];
				if (
					code == INPUT_GAMEPAD_L_TRIGGER || 
					code == INPUT_GAMEPAD_R_TRIGGER
				) {
					input_set_button_state(code, state);
				}
				else if (state > 0) {
					input_set_button_state(code, 0.0);
					input_set_button_state(code+1, state);
				}
				else {
					input_set_button_state(code, -state);
					input_set_button_state(code+1, 0.0);
				}
			}
		}

		// Mouse buttons
		else if (
			ev.type == SDL_MOUSEBUTTONDOWN ||
			ev.type == SDL_MOUSEBUTTONUP
		) {
			button_t button = INPUT_BUTTON_NONE;
			switch (ev.button.button) {
				case SDL_BUTTON_LEFT: button = INPUT_MOUSE_LEFT; break;
				case SDL_BUTTON_MIDDLE: button = INPUT_MOUSE_MIDDLE; break;
				case SDL_BUTTON_RIGHT: button = INPUT_MOUSE_RIGHT; break;
				default: break;
			}
			if (button != INPUT_BUTTON_NONE) {
				float state = ev.type == SDL_MOUSEBUTTONDOWN ? 1.0 : 0.0;
				input_set_button_state(button, state);
			}
		}

		// Mouse wheel
		else if (ev.type == SDL_MOUSEWHEEL) {
			button_t button = ev.wheel.y > 0 
				? INPUT_MOUSE_WHEEL_UP
				: INPUT_MOUSE_WHEEL_DOWN;
			input_set_button_state(button, 1.0);
			input_set_button_state(button, 0.0);
		}

		// Mouse move
		else if (ev.type == SDL_MOUSEMOTION) {
			input_set_mouse_pos(ev.motion.x, ev.motion.y);
		}

		// Window Events
		if (ev.type == SDL_QUIT) {
			wants_to_exit = true;
		}
		else if (
			ev.type == SDL_WINDOWEVENT &&
			(
				ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED ||
				ev.window.event == SDL_WINDOWEVENT_RESIZED
			)
		) {
			system_resize(platform_screen_size());
		}
	}
}

double platform_now() {
	uint64_t perf_counter = SDL_GetPerformanceCounter();
	return (double)perf_counter / (double)perf_freq;
}

bool platform_get_fullscreen() {
	return SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN;
}

void platform_set_fullscreen(bool fullscreen) {
	if (fullscreen) {
		int32_t display = SDL_GetWindowDisplayIndex(window);
		
		SDL_DisplayMode mode;
		SDL_GetDesktopDisplayMode(display, &mode);
		SDL_SetWindowDisplayMode(window, &mode);
		SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
		SDL_ShowCursor(SDL_DISABLE);
	}
	else {
		SDL_SetWindowFullscreen(window, 0);
		SDL_ShowCursor(SDL_ENABLE);
	}
}

void platform_audio_callback(void* userdata, uint8_t* stream, int len) {
	if (audio_callback) {
		audio_callback((float *)stream, len/sizeof(float));
	}
	else {
		memset(stream, 0, len);
	}
}

void platform_set_audio_mix_cb(void (*cb)(float *buffer, uint32_t len)) {
	audio_callback = cb;
	SDL_PauseAudioDevice(audio_device, 0);
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




#if defined(RENDERER_GL) // ----------------------------------------------------
	#define PLATFORM_WINDOW_FLAGS SDL_WINDOW_OPENGL
	SDL_GLContext platform_gl;

	void platform_video_init() {
		#if defined(USE_GLES2)
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
		#endif

		platform_gl = SDL_GL_CreateContext(window);
		SDL_GL_SetSwapInterval(1);
	}

	void platform_prepare_frame() {
		// nothing
	}

	void platform_video_cleanup() {
		SDL_GL_DeleteContext(platform_gl);
	}

	void platform_end_frame() {
		SDL_GL_SwapWindow(window);
	}

	vec2i_t platform_screen_size() {
		int width, height;
		SDL_GL_GetDrawableSize(window, &width, &height);
		return vec2i(width, height);
	}


#elif defined(RENDERER_SOFTWARE) // ----------------------------------------------
	#define PLATFORM_WINDOW_FLAGS 0
	static SDL_Renderer *renderer;
	static SDL_Texture *screenbuffer = NULL;
	static void *screenbuffer_pixels = NULL;
	static int screenbuffer_pitch;
	static vec2i_t screenbuffer_size = vec2i(0, 0);
	static vec2i_t screen_size = vec2i(0, 0);


	void platform_video_init() {
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	}

	void platform_video_cleanup() {
		
	}

	void platform_prepare_frame() {
		if (screen_size.x != screenbuffer_size.x || screen_size.y != screenbuffer_size.y) {
			if (screenbuffer) {
				SDL_DestroyTexture(screenbuffer);
			}
			screenbuffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, screen_size.x, screen_size.y);
			screenbuffer_size = screen_size;
		}
		SDL_LockTexture(screenbuffer, NULL, &screenbuffer_pixels, &screenbuffer_pitch);
	}

	void platform_end_frame() {
		screenbuffer_pixels = NULL;
		SDL_UnlockTexture(screenbuffer);
		SDL_RenderCopy(renderer, screenbuffer, NULL, NULL);
		SDL_RenderPresent(renderer);
	}

	rgba_t *platform_get_screenbuffer(int32_t *pitch) {
		*pitch = screenbuffer_pitch;
		return screenbuffer_pixels;
	}

	vec2i_t platform_screen_size() {
		int width, height;
		SDL_GetWindowSize(window, &width, &height);

		// float aspect = (float)width / (float)height;
		// screen_size = vec2i(240 * aspect, 240);
		screen_size = vec2i(width, height);
		return screen_size;
	}

#else
	#error "Unsupported renderer for platform SDL"
#endif



int main(int argc, char *argv[]) {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER);

	// Figure out the absolute asset and userdata paths. These may either be
	// supplied at build time through -DPATH_ASSETS=.. and -DPATH_USERDATA=.. 
	// or received at runtime from SDL. Note that SDL may return NULL for these.
	// We fall back to the current directory (i.e. just "") in this case.

	#ifdef PATH_ASSETS
		path_assets = TOSTRING(PATH_ASSETS);
	#else
		path_assets = SDL_GetBasePath();
		if (path_assets == NULL) {
			path_assets = "";
		}
	#endif

	#ifdef PATH_USERDATA
		path_userdata = TOSTRING(PATH_USERDATA);
	#else
		path_userdata = SDL_GetPrefPath("phoboslab", "wipeout");
		if (path_userdata == NULL) {
			path_userdata = "";
		}
	#endif

	// Reserve some space for concatenating the asset and userdata paths with
	// local filenames.
	temp_path = mem_bump(max(strlen(path_assets), strlen(path_userdata)) + 64);

	// Load gamecontrollerdb.txt if present.
	// FIXME: Should this load from userdata instead?
	char *gcdb_path = strcat(strcpy(temp_path, path_assets), "gamecontrollerdb.txt");
	int gcdb_res = SDL_GameControllerAddMappingsFromFile(gcdb_path);
	if (gcdb_res < 0) {
		printf("Failed to load gamecontrollerdb.txt\n");
	}
	else {
		printf("load gamecontrollerdb.txt\n");
	}



	gamepad = platform_find_gamepad();

	perf_freq = SDL_GetPerformanceFrequency();

	audio_device = SDL_OpenAudioDevice(NULL, 0, &(SDL_AudioSpec){
		.freq = 44100,
		.format = AUDIO_F32,
		.channels = 2,
		.samples = 1024,
		.callback = platform_audio_callback
	}, NULL, 0);

	window = SDL_CreateWindow(
		SYSTEM_WINDOW_NAME,
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		SYSTEM_WINDOW_WIDTH, SYSTEM_WINDOW_HEIGHT,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | PLATFORM_WINDOW_FLAGS | SDL_WINDOW_ALLOW_HIGHDPI
	);

	platform_video_init();
	system_init();

	while (!wants_to_exit) {
		platform_pump_events();
		platform_prepare_frame();
		system_update();
		platform_end_frame();
	}

	system_cleanup();
	platform_video_cleanup();

	SDL_free(path_assets);
	SDL_free(path_userdata);

	SDL_CloseAudioDevice(audio_device);
	SDL_Quit();
	return 0;
}
