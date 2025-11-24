#include "assert.h"
#include "ps2_keyboard_handler_public.h"
#include "ps2_keyboard_key_functions.h"
#include "ps2_keyboard_public.h"
#include "stdio.h"
#include "vga_terminal_public.h"

extern kcb_led_state_t toggle_keys;

void pressed_a() {
	kprintf("Hey, you pressed a!\n");
}

void pressed_caps_lock() {
	toggle_keys.CapsLock = !toggle_keys.CapsLock;
	set_leds(toggle_keys);
}

void pressed_num_lock() {
	toggle_keys.NumberLock = !toggle_keys.NumberLock;
	set_leds(toggle_keys);
}

void pressed_scroll_lock() {
	toggle_keys.ScrollLock = !toggle_keys.ScrollLock;
	set_leds(toggle_keys);
}

// Possibly, create a state machine, for which "game I'm playing"
// And pressed up, will do pressed up on the correct state machine (switch statement to the correct function)
void pressed_up() {
	terminal_arrow_up();
}

void pressed_down() {
	terminal_arrow_down();
}

void pressed_right() {
	kprintf("Hey, you pressed Up!\n");
	terminal_arrow_right();
}

void pressed_left() {
	kprintf("Hey, you pressed Up!\n");
	terminal_arrow_left();
}

void pressed_r() {
	if (is_ctrl_held()) {
		terminal_ctrl_r();
	}
}

void pressed_f() {
	if (is_ctrl_held()) {
		terminal_ctrl_f();
	}
}
