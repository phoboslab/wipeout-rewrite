#include <string.h>

#include "input.h"
#include "utils.h"

static const char *button_names[] = {
	NULL,
	NULL,
	NULL,
	NULL,
	[INPUT_KEY_A] = "A",
	[INPUT_KEY_B] = "B",
	[INPUT_KEY_C] = "C",
	[INPUT_KEY_D] = "D",
	[INPUT_KEY_E] = "E",
	[INPUT_KEY_F] = "F",
	[INPUT_KEY_G] = "G",
	[INPUT_KEY_H] = "H",
	[INPUT_KEY_I] = "I",
	[INPUT_KEY_J] = "J",
	[INPUT_KEY_K] = "K",
	[INPUT_KEY_L] = "L",
	[INPUT_KEY_M] = "M",
	[INPUT_KEY_N] = "N",
	[INPUT_KEY_O] = "O",
	[INPUT_KEY_P] = "P",
	[INPUT_KEY_Q] = "Q",
	[INPUT_KEY_R] = "R",
	[INPUT_KEY_S] = "S",
	[INPUT_KEY_T] = "T",
	[INPUT_KEY_U] = "U",
	[INPUT_KEY_V] = "V",
	[INPUT_KEY_W] = "W",
	[INPUT_KEY_X] = "X",
	[INPUT_KEY_Y] = "Y",
	[INPUT_KEY_Z] = "Z",
	[INPUT_KEY_1] = "1",
	[INPUT_KEY_2] = "2",
	[INPUT_KEY_3] = "3",
	[INPUT_KEY_4] = "4",
	[INPUT_KEY_5] = "5",
	[INPUT_KEY_6] = "6",
	[INPUT_KEY_7] = "7",
	[INPUT_KEY_8] = "8",
	[INPUT_KEY_9] = "9",
	[INPUT_KEY_0] = "0",
	[INPUT_KEY_RETURN] = "RETURN",
	[INPUT_KEY_ESCAPE] = "ESCAPE",
	[INPUT_KEY_BACKSPACE] = "BACKSP",
	[INPUT_KEY_TAB] = "TAB",
	[INPUT_KEY_SPACE] = "SPACE",
	[INPUT_KEY_MINUS] = "MINUS",
	[INPUT_KEY_EQUALS] = "EQUALS",
	[INPUT_KEY_LEFTBRACKET] = "LBRACKET",
	[INPUT_KEY_RIGHTBRACKET] = "RBRACKET",
	[INPUT_KEY_BACKSLASH] = "BSLASH",
	[INPUT_KEY_HASH] = "HASH",
	[INPUT_KEY_SEMICOLON] = "SMICOL",
	[INPUT_KEY_APOSTROPHE] = "APO",
	[INPUT_KEY_TILDE] = "TILDE",
	[INPUT_KEY_COMMA] = "COMMA",
	[INPUT_KEY_PERIOD] = "PERIOD",
	[INPUT_KEY_SLASH] = "SLASH",
	[INPUT_KEY_CAPSLOCK] = "CAPS",
	[INPUT_KEY_F1] = "F1",
	[INPUT_KEY_F2] = "F2",
	[INPUT_KEY_F3] = "F3",
	[INPUT_KEY_F4] = "F4",
	[INPUT_KEY_F5] = "F5",
	[INPUT_KEY_F6] = "F6",
	[INPUT_KEY_F7] = "F7",
	[INPUT_KEY_F8] = "F8",
	[INPUT_KEY_F9] = "F9",
	[INPUT_KEY_F10] = "F10",
	[INPUT_KEY_F11] = "F11",
	[INPUT_KEY_F12] = "F12",
	[INPUT_KEY_PRINTSCREEN] = "PRTSC",
	[INPUT_KEY_SCROLLLOCK] = "SCRLK",
	[INPUT_KEY_PAUSE] = "PAUSE",
	[INPUT_KEY_INSERT] = "INSERT",
	[INPUT_KEY_HOME] = "HOME",
	[INPUT_KEY_PAGEUP] = "PG UP",
	[INPUT_KEY_DELETE] = "DELETE",
	[INPUT_KEY_END] = "END",
	[INPUT_KEY_PAGEDOWN] = "PG DOWN",
	[INPUT_KEY_RIGHT] = "RIGHT",
	[INPUT_KEY_LEFT] = "LEFT",
	[INPUT_KEY_DOWN] = "DOWN",
	[INPUT_KEY_UP] = "UP",
	[INPUT_KEY_NUMLOCK] = "NLOCK",
	[INPUT_KEY_KP_DIVIDE] = "KPDIV",
	[INPUT_KEY_KP_MULTIPLY] = "KPMUL",
	[INPUT_KEY_KP_MINUS] = "KPMINUS",
	[INPUT_KEY_KP_PLUS] = "KPPLUS",
	[INPUT_KEY_KP_ENTER] = "KPENTER",
	[INPUT_KEY_KP_1] = "KP1",
	[INPUT_KEY_KP_2] = "KP2",
	[INPUT_KEY_KP_3] = "KP3",
	[INPUT_KEY_KP_4] = "KP4",
	[INPUT_KEY_KP_5] = "KP5",
	[INPUT_KEY_KP_6] = "KP6",
	[INPUT_KEY_KP_7] = "KP7",
	[INPUT_KEY_KP_8] = "KP8",
	[INPUT_KEY_KP_9] = "KP9",
	[INPUT_KEY_KP_0] = "KP0",
	[INPUT_KEY_KP_PERIOD] = "KPPERIOD",

	[INPUT_KEY_LCTRL] = "LCTRL",
	[INPUT_KEY_LSHIFT] = "LSHIFT",
	[INPUT_KEY_LALT] = "LALT",
	[INPUT_KEY_LGUI] = "LGUI",
	[INPUT_KEY_RCTRL] = "RCTRL",
	[INPUT_KEY_RSHIFT] = "RSHIFT",
	[INPUT_KEY_RALT] = "RALT",
	NULL,
	[INPUT_GAMEPAD_A] = "A",
	[INPUT_GAMEPAD_Y] = "Y",
	[INPUT_GAMEPAD_B] = "B",
	[INPUT_GAMEPAD_X] = "X",
	[INPUT_GAMEPAD_L_SHOULDER] = "LSHLDR",
	[INPUT_GAMEPAD_R_SHOULDER] = "RSHLDR",
	[INPUT_GAMEPAD_L_TRIGGER] = "LTRIG",
	[INPUT_GAMEPAD_R_TRIGGER] = "RTRIG",
	[INPUT_GAMEPAD_SELECT] = "SELECT",
	[INPUT_GAMEPAD_START] = "START",
	[INPUT_GAMEPAD_L_STICK_PRESS] = "LSTK",
	[INPUT_GAMEPAD_R_STICK_PRESS] = "RSTK",
	[INPUT_GAMEPAD_DPAD_UP] = "DPUP",
	[INPUT_GAMEPAD_DPAD_DOWN] = "DPDOWN",
	[INPUT_GAMEPAD_DPAD_LEFT] = "DPLEFT",
	[INPUT_GAMEPAD_DPAD_RIGHT] = "DPRIGHT",
	[INPUT_GAMEPAD_HOME] = "HOME",
	[INPUT_GAMEPAD_L_STICK_UP] = "LSTKUP",
	[INPUT_GAMEPAD_L_STICK_DOWN] = "LSTKDOWN",
	[INPUT_GAMEPAD_L_STICK_LEFT] = "LSTKLEFT",
	[INPUT_GAMEPAD_L_STICK_RIGHT] = "LSTKRIGHT",
	[INPUT_GAMEPAD_R_STICK_UP] = "RSTKUP",
	[INPUT_GAMEPAD_R_STICK_DOWN] = "RSTKDOWN",
	[INPUT_GAMEPAD_R_STICK_LEFT] = "RSTKLEFT",
	[INPUT_GAMEPAD_R_STICK_RIGHT] = "RSTKRIGHT",
	NULL,
	[INPUT_MOUSE_LEFT] = "MLEFT",
	[INPUT_MOUSE_MIDDLE] = "MMIDDLE",
	[INPUT_MOUSE_RIGHT] = "MRIGHT",
	[INPUT_MOUSE_WHEEL_UP] = "MWUP",
	[INPUT_MOUSE_WHEEL_DOWN] = "MWDOWN",
};

static float actions_state[INPUT_ACTION_MAX];
static bool actions_pressed[INPUT_ACTION_MAX];
static bool actions_released[INPUT_ACTION_MAX];

static uint8_t expected_button[INPUT_ACTION_MAX];
static uint8_t bindings[INPUT_LAYER_MAX][INPUT_BUTTON_MAX];

static input_capture_callback_t capture_callback;
static void *capture_user;

static int32_t mouse_x;
static int32_t mouse_y;

void input_init(void) {
	input_unbind_all(INPUT_LAYER_SYSTEM);
	input_unbind_all(INPUT_LAYER_USER);
}

void input_cleanup(void) {

}

void input_clear(void) {
	clear(actions_pressed);
	clear(actions_released);
}

void input_set_layer_button_state(input_layer_t layer, button_t button, float state) {
	error_if(layer < 0 || layer >= INPUT_LAYER_MAX, "Invalid input layer %d", layer);

	uint8_t action = bindings[layer][button];
	if (action == INPUT_ACTION_NONE) {
		return;
	}

	uint8_t expected = expected_button[action];
	if (!expected || expected == button) {
		state = (state > INPUT_DEADZONE) ? state : 0;

		if (state && !actions_state[action]) {
			actions_pressed[action] = true;
			expected_button[action] = button;
		}
		else if (!state && actions_state[action]) {
			actions_released[action] = true;
			expected_button[action] = INPUT_BUTTON_NONE;
		}
		actions_state[action] = state;
	}
}

void input_set_button_state(button_t button, float state) {
	error_if(button < 0 || button >= INPUT_BUTTON_MAX, "Invalid input button %d", button);

	input_set_layer_button_state(INPUT_LAYER_SYSTEM, button, state);
	input_set_layer_button_state(INPUT_LAYER_USER, button, state);

	if (capture_callback) {
		if (state > INPUT_DEADZONE_CAPTURE) {
			capture_callback(capture_user, button, 0);
		}
	}
}

void input_set_mouse_pos(int32_t x, int32_t y) {
	mouse_x = x;
	mouse_y = y;
}

void input_capture(input_capture_callback_t cb, void *user) {
	capture_callback = cb;
	capture_user = user;
	input_clear();
}

void input_textinput(int32_t ascii_char) {
	if (capture_callback) {
		capture_callback(capture_user, INPUT_INVALID, ascii_char);
	}
}

void input_bind(input_layer_t layer, button_t button, uint8_t action) {
	error_if(button < 0 || button >= INPUT_BUTTON_MAX, "Invalid input button %d", button);
	error_if(action < 0 || action >= INPUT_ACTION_MAX, "Invalid input action %d", action);
	error_if(layer < 0 || layer >= INPUT_LAYER_MAX, "Invalid input layer %d", layer);

	actions_state[action] = 0;
	bindings[layer][button] = action;
	clear(expected_button);
}

uint8_t input_bound_to_action(button_t button) {
	error_if(button < 0 || button >= INPUT_BUTTON_MAX, "Invalid input button %d", button);
	return bindings[INPUT_LAYER_USER][button];
}

void input_unbind(input_layer_t layer, button_t button) {
	error_if(layer < 0 || layer >= INPUT_LAYER_MAX, "Invalid input layer %d", layer);
	error_if(button < 0 || button >= INPUT_BUTTON_MAX, "Invalid input button %d", button);

	bindings[layer][button] = INPUT_ACTION_NONE;
}

void input_unbind_all(input_layer_t layer) {
	error_if(layer < 0 || layer >= INPUT_LAYER_MAX, "Invalid input layer %d", layer);
	
	for (uint32_t button = 0; button < INPUT_BUTTON_MAX; button++) {
		input_unbind(layer, button);
	}
}


float input_state(uint8_t action) {
	error_if(action < 0 || action >= INPUT_ACTION_MAX, "Invalid input action %d", action);
	return actions_state[action];
}


bool input_pressed(uint8_t action) {
	error_if(action < 0 || action >= INPUT_ACTION_MAX, "Invalid input action %d", action);
	return actions_pressed[action];
}


bool input_released(uint8_t action) {
	error_if(action < 0 || action >= INPUT_ACTION_MAX, "Invalid input action %d", action);
	return actions_released[action];
}

vec2_t input_mouse_pos(void) {
	return vec2(mouse_x, mouse_y);
}


button_t input_name_to_button(const char *name) {
	for (int32_t i = 0; i < INPUT_BUTTON_MAX; i++) {
		if (button_names[i] && strcmp(name, button_names[i]) == 0) {
			return i;
		}
	}
	return INPUT_INVALID;
}

const char *input_button_to_name(button_t button) {
	if (
		button < 0 || button >= INPUT_BUTTON_MAX ||
		!button_names[button]
	) {
		return NULL;
	}
	return button_names[button];
}


#if defined(__EMSCRIPTEN__)
	#include <emscripten/emscripten.h>
	void EMSCRIPTEN_KEEPALIVE set_button(uint32_t button, uint32_t state) {
		input_set_button_state(button, state ? 1.0 : 0.0);
	}
#endif
