#include "assert.h"
#include "more_types.h"
#include "pic.h"
#include "ps2_keyboard_handler.h"
#include "ps2_keyboard_handler_public.h"
#include "ps2_keyboard_key_functions.h"
#include "ps2_keyboard_public.h"
#include "stdio.h"
#include "vga_terminal.h"
// possible, create a ps2 controller device, and then a keyboard controller device. What was done here is essentially a keyboard hadnler device

void parse_scan_code(uint8_t scancode, bool pressed_not_released);
void parse_extended_scan_code(uint8_t scancode, bool pressed_not_released);

void keyboard_handler_scs1(uint8_t scancode, uint8_t port_number);
void keyboard_handler_scs2(uint8_t scancode, uint8_t port_number);
void keyboard_handler_scs3(uint8_t scancode, uint8_t port_number);

kcb_led_state_t toggle_keys;
/* ========================================================Setup the handling of special keys ===================================================================== */
bool is_key_pressed[SCANCODE_LEN] = {0};
function_t_ptr key_functions[256];
bool is_key_tied_to_function[256] = {0};

bool is_extended_key_pressed[EXTENED_SCANCODE_LEN] = {0};
function_t_ptr extended_key_functions[256];
bool is_extended_key_tied_to_function[256] = {0};

void set_key_function(scancode_t key, function_t_ptr func) {
	assert(validate_scancode(key), "key should be inside the thing\n");
	key_functions[key] = func;
	is_key_tied_to_function[key] = true;
}

void set_extended_key_function(extended_scancode_t scancode, function_t_ptr func) {
	assert(validate_extended_scancode(scancode), "key should be inside the thing\n");
	extended_key_functions[scancode] = func;
	is_extended_key_tied_to_function[scancode] = true;
}

static void nop() {
}

void setup_keyboard_extra() {
	for (uint16_t i = 0; i < 256; i++) {
		key_functions[i] = nop;
		is_key_tied_to_function[i] = false;

		extended_key_functions[i] = nop;
		is_extended_key_tied_to_function[i] = false;
	}

	set_key_function(SC_A, pressed_a);
	set_key_function(SC_CAPSLOCK, pressed_caps_lock);
	set_key_function(SC_NUMLOCK, pressed_num_lock);
	set_key_function(SC_SCROLLLOCK, pressed_scroll_lock);

	set_extended_key_function(SCE_UP, pressed_up);
	set_extended_key_function(SCE_DOWN, pressed_down);
	set_extended_key_function(SCE_LEFT, pressed_left);
	set_extended_key_function(SCE_RIGHT, pressed_right);

	set_key_function(SC_R, pressed_r);
	set_key_function(SC_F, pressed_f);

	toggle_keys.CapsLock = false;
	toggle_keys.NumberLock = false;
	toggle_keys.ScrollLock = false;
	toggle_keys.reserved = false;
	set_leds(toggle_keys);
}
/* ========================================================  The keyboard handler ===================================================================== */

extern struct keyboard_porting_state_context kps;

extern inline void keyboard_handler(uint8_t scancode, uint8_t port_number) {
	switch (kps.active_scancode_set) {
	case 1:
		return keyboard_handler_scs1(scancode, port_number);
	case 2:
		return keyboard_handler_scs3(scancode, port_number);
	case 3:
		return keyboard_handler_scs3(scancode, port_number);
	default:
		abort_msg("Impossible scan scode set (%u)\n", kps.active_scancode_set);
	}
}

/*
Scancode: 		The mouse scancode sent, read from port 0x60. (PS2_DATA_PORT_RW)
port_number: 	The ps2 port number the mouse is connected to.
    This allows runtimes port number, and the keyboard isn't forced to be plugged in port1, with mouse in port2

Note: This function assume standard PS/2 mouse.



preconditions:
    Port Number must be 1 or 2

    Keyboard must be in scancode set 1.

*/
void keyboard_handler_scs1(uint8_t scancode, uint8_t port_number) {
	uint8_t keyboard_irq;
	if (port_number == 1) {
		keyboard_irq = PS2_PORT1_IRQ;
	} else {
		keyboard_irq = PS2_PORT2_IRQ;
	}
	// this can be optimized into a cmovs with immediates (no memory reads or branch)
	// Blazingly fast

	bool extended_signal = false;
	static bool previous_extended_signal = false;
	// TODO: Fix this.
	// This can break if we have two keyboards
	// A race condition can happened, since this is a shared ressource!

	bool press_not_release = true;
	if (scancode == 0xe0) {
		extended_signal = true;
	}
	// ===============
	if (extended_signal) {
		// kprintf("scan code (e0       ) = |%u:3, %h:4, %b:8|\n", scancode, scancode, scancode);
		previous_extended_signal = extended_signal;
		PIC_sendEOI(keyboard_irq);
		return;
	}

	if (previous_extended_signal) {
		if (scancode >= EXTENDED_SCANCODE_RELEASE_START) {
			scancode -= EXTENDED_SCANCODE_RELEASE_OFFSET;
			press_not_release = false;
		}
		parse_extended_scan_code(scancode, press_not_release);
	} else {
		if (scancode >= SCANCODE_RELEASE_START) {
			scancode -= SCANCODE_RELEASE_OFFSET;
			press_not_release = false;
		}
		parse_scan_code(scancode, press_not_release);
	}

	if (!press_not_release) {
		// terminal_putchar('\n');
	}

	previous_extended_signal = extended_signal;
	PIC_sendEOI(keyboard_irq);
}

void keyboard_handler_scs2(uint8_t scancode, uint8_t port_number) {
	kprintf("[PANIC-WARNING] scan code 2 handling not supported!\n");
}

void keyboard_handler_scs3(uint8_t scancode, uint8_t port_number) {
	kprintf("[PANIC-WARNING] scan code 3 handling not supported!\n");
}

/* ================================= GET States of modifier keys ============================================ */

bool is_shift_held() {
	return is_key_pressed[SC_LEFT_SHIFT] || is_key_pressed[SC_RIGHT_SHIFT];
}

bool is_ctrl_held() {
	return is_key_pressed[SC_LEFT_CTRL] || is_extended_key_pressed[SCE_RIGHT_CTRL];
}

inline bool is_alt_held() {
	return is_key_pressed[SC_LEFT_ALT];
}

inline bool is_altgr_held() {
	return is_extended_key_pressed[SCE_RIGHT_ALT];
}

inline bool is_capslock_on() {
	return toggle_keys.CapsLock;
}

inline bool is_caps() {
	return is_shift_held() || is_capslock_on();
}

/* ================================= Parse Scan Codes ============================================ */
void parse_scan_code(uint8_t press_scancode, bool pressed_not_released) {
	const char* scancode_name = scancode_to_string(press_scancode);
	// kprintf("Scancode %u = |%s|\n", press_scancode, scancode_name);

	if (pressed_not_released) {
		is_key_pressed[press_scancode] = true;

		if (is_key_tied_to_function[press_scancode]) {
			key_functions[press_scancode]();
		}

	} else {
		is_key_pressed[press_scancode] = false;
	}
}
void parse_extended_scan_code(uint8_t press_scancode, bool pressed_not_released) {

	const char* scancode_name = extended_scancode_to_string(press_scancode);
	// kprintf("Scancode %u = |%s|\n", press_scancode, scancode_name);

	if (pressed_not_released) {
		is_extended_key_pressed[press_scancode] = true;

		if (is_extended_key_tied_to_function[press_scancode]) {
			extended_key_functions[press_scancode]();
		}

	} else {
		is_key_pressed[press_scancode] = false;
	}
}
