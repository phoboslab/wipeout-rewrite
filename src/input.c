#include <string.h>

#include "input.h"
#include "utils.h"

static const char *button_names[] = {
	NULL, 
	NULL, 
	NULL, 
	NULL,
	[INPUT_KEY_A] = "a",
	[INPUT_KEY_B] = "b",
	[INPUT_KEY_C] = "c",
	[INPUT_KEY_D] = "d",
	[INPUT_KEY_E] = "e",
	[INPUT_KEY_F] = "f",
	[INPUT_KEY_G] = "g",
	[INPUT_KEY_H] = "h",
	[INPUT_KEY_I] = "i",
	[INPUT_KEY_J] = "j",
	[INPUT_KEY_K] = "k",
	[INPUT_KEY_L] = "l",
	[INPUT_KEY_M] = "m",
	[INPUT_KEY_N] = "n",
	[INPUT_KEY_O] = "o",
	[INPUT_KEY_P] = "p",
	[INPUT_KEY_Q] = "q",
	[INPUT_KEY_R] = "r",
	[INPUT_KEY_S] = "s",
	[INPUT_KEY_T] = "t",
	[INPUT_KEY_U] = "u",
	[INPUT_KEY_V] = "v",
	[INPUT_KEY_W] = "w",
	[INPUT_KEY_X] = "x",
	[INPUT_KEY_Y] = "y",
	[INPUT_KEY_Z] = "z",
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
	[INPUT_KEY_RETURN] = "return",
	[INPUT_KEY_ESCAPE] = "escape",
	[INPUT_KEY_BACKSPACE] = "backspace",
	[INPUT_KEY_TAB] = "tab",
	[INPUT_KEY_SPACE] = "space",
	[INPUT_KEY_MINUS] = "minus",
	[INPUT_KEY_EQUALS] = "equals",
	[INPUT_KEY_LEFTBRACKET] = "left_bracket",
	[INPUT_KEY_RIGHTBRACKET] = "right_bracket",
	[INPUT_KEY_BACKSLASH] = "backslash",
	[INPUT_KEY_HASH] = "hash",
	[INPUT_KEY_SEMICOLON] = "semicolon",
	[INPUT_KEY_APOSTROPHE] = "apostrophe",
	[INPUT_KEY_TILDE] = "tilde",
	[INPUT_KEY_COMMA] = "comma",
	[INPUT_KEY_PERIOD] = "period",
	[INPUT_KEY_SLASH] = "slash",
	[INPUT_KEY_CAPSLOCK] = "capslock",
	[INPUT_KEY_F1] = "f1",
	[INPUT_KEY_F2] = "f2",
	[INPUT_KEY_F3] = "f3",
	[INPUT_KEY_F4] = "f4",
	[INPUT_KEY_F5] = "f5",
	[INPUT_KEY_F6] = "f6",
	[INPUT_KEY_F7] = "f7",
	[INPUT_KEY_F8] = "f8",
	[INPUT_KEY_F9] = "f9",
	[INPUT_KEY_F10] = "f10",
	[INPUT_KEY_F11] = "f11",
	[INPUT_KEY_F12] = "f12",
	[INPUT_KEY_PRINTSCREEN] = "print_screen",
	[INPUT_KEY_SCROLLLOCK] = "scroll_lock",
	[INPUT_KEY_PAUSE] = "pause",
	[INPUT_KEY_INSERT] = "insert",
	[INPUT_KEY_HOME] = "home",
	[INPUT_KEY_PAGEUP] = "page_up",
	[INPUT_KEY_DELETE] = "delete",
	[INPUT_KEY_END] = "end",
	[INPUT_KEY_PAGEDOWN] = "page_down",
	[INPUT_KEY_RIGHT] = "right",
	[INPUT_KEY_LEFT] = "left",
	[INPUT_KEY_DOWN] = "down",
	[INPUT_KEY_UP] = "up",
	[INPUT_KEY_NUMLOCK] = "num_lock",
	[INPUT_KEY_KP_DIVIDE] = "keypad_divide",
	[INPUT_KEY_KP_MULTIPLY] = "keypad_multiply",
	[INPUT_KEY_KP_MINUS] = "keypad_minus",
	[INPUT_KEY_KP_PLUS] = "keypad_plus",
	[INPUT_KEY_KP_ENTER] = "keypad_enter",
	[INPUT_KEY_KP_1] = "keypad_1",
	[INPUT_KEY_KP_2] = "keypad_2",
	[INPUT_KEY_KP_3] = "keypad_3",
	[INPUT_KEY_KP_4] = "keypad_4",
	[INPUT_KEY_KP_5] = "keypad_5",
	[INPUT_KEY_KP_6] = "keypad_6",
	[INPUT_KEY_KP_7] = "keypad_7",
	[INPUT_KEY_KP_8] = "keypad_8",
	[INPUT_KEY_KP_9] = "keypad_9",
	[INPUT_KEY_KP_0] = "keypad_0",
	[INPUT_KEY_KP_PERIOD] = "keypad_period",

	[INPUT_KEY_LCTRL] = "left_ctrl",
	[INPUT_KEY_LSHIFT] = "left_shift",
	[INPUT_KEY_LALT] = "left_alt",
	[INPUT_KEY_LGUI] = "left_gui",
	[INPUT_KEY_RCTRL] = "right_ctrl",
	[INPUT_KEY_RSHIFT] = "right_shift",
	[INPUT_KEY_RALT] = "right_alt",
	NULL,
	[INPUT_GAMEPAD_A] = "gamepad_a",
	[INPUT_GAMEPAD_Y] = "gamepad_y",
	[INPUT_GAMEPAD_B] = "gamepad_b",
	[INPUT_GAMEPAD_X] = "gamepad_x",
	[INPUT_GAMEPAD_L_SHOULDER] = "gamepad_left_shoulder",
	[INPUT_GAMEPAD_R_SHOULDER] = "gamepad_right_shoulder",
	[INPUT_GAMEPAD_L_TRIGGER] = "gamepad_left_trigger",
	[INPUT_GAMEPAD_R_TRIGGER] = "gamepad_right_trigger",
	[INPUT_GAMEPAD_SELECT] = "gamepad_select",
	[INPUT_GAMEPAD_START] = "gamepad_start",
	[INPUT_GAMEPAD_L_STICK_PRESS] = "gamepad_left_stick_press",
	[INPUT_GAMEPAD_R_STICK_PRESS] = "gamepad_right_stick_press",
	[INPUT_GAMEPAD_DPAD_UP] = "gamepad_dpad_up",
	[INPUT_GAMEPAD_DPAD_DOWN] = "gamepad_dpad_down",
	[INPUT_GAMEPAD_DPAD_LEFT] = "gamepad_dpad_left",
	[INPUT_GAMEPAD_DPAD_RIGHT] = "gamepad_dpad_right",
	[INPUT_GAMEPAD_HOME] = "gamepad_home",
	[INPUT_GAMEPAD_L_STICK_UP] = "gamepad_left_stick_up",
	[INPUT_GAMEPAD_L_STICK_DOWN] = "gamepad_left_stick_down",
	[INPUT_GAMEPAD_L_STICK_LEFT] = "gamepad_left_stick_left",
	[INPUT_GAMEPAD_L_STICK_RIGHT] = "gamepad_left_stick_right",
	[INPUT_GAMEPAD_R_STICK_UP] = "gamepad_right_stick_up",
	[INPUT_GAMEPAD_R_STICK_DOWN] = "gamepad_right_stick_down",
	[INPUT_GAMEPAD_R_STICK_LEFT] = "gamepad_right_stick_left",
	[INPUT_GAMEPAD_R_STICK_RIGHT] = "gamepad_right_stick_right",
	NULL,
	[INPUT_MOUSE_LEFT] = "mouse_left",
	[INPUT_MOUSE_MIDDLE] = "mouse_middle",
	[INPUT_MOUSE_RIGHT] = "mouse_right",
	[INPUT_MOUSE_WHEEL_UP] = "mouse_wheel_up",
	[INPUT_MOUSE_WHEEL_DOWN] = "mouse_wheel_down",
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

void input_init() {
	input_unbind_all(INPUT_LAYER_SYSTEM);
	input_unbind_all(INPUT_LAYER_USER);
}

void input_cleanup() {

}

void input_clear() {
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

	if (capture_callback) {
		if (state) {
			capture_callback(capture_user, button, 0);
		}
		return;
	}

	input_set_layer_button_state(INPUT_LAYER_SYSTEM, button, state);
	input_set_layer_button_state(INPUT_LAYER_USER, button, state);
}

void input_set_mouse_pos(int32_t x, int32_t y) {
	mouse_x = x;
	mouse_y = y;
}

void input_capture(input_capture_callback_t cb, void *user) {
	capture_callback = cb;
	capture_user = user;
	clear(actions_state);
}

void input_textinput(int32_t ascii_char) {
	if (capture_callback) {
		capture_callback(capture_user, 0, ascii_char);
	}
}

void input_bind(input_layer_t layer, button_t button, uint8_t action) {
	error_if(button < 0 || button >= INPUT_BUTTON_MAX, "Invalid input button %d", button);
	error_if(action < 0 || action >= INPUT_ACTION_MAX, "Invalid input action %d", action);
	error_if(layer < 0 || layer >= INPUT_LAYER_MAX, "Invalid input layer %d", layer);

	bindings[layer][button] = action;
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

vec2_t input_mouse_pos() {
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
