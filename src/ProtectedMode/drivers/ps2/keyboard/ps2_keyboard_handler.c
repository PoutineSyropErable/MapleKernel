#include "apic.h"
#include "assert.h"
#include "keycodes.h"
#include "more_types.h"
#include "pic.h"
#include "pic_public.h"
#include "ps2_keyboard_handler.h"
#include "ps2_keyboard_handler_public.h"
#include "ps2_keyboard_key_functions.h"
#include "ps2_keyboard_public.h"
#include "stdio.h"
#include "vga_terminal.h"
// possible, create a ps2 controller device, and then a keyboard controller device. What was done here is essentially a keyboard hadnler
// device

void parse_scan_code(uint8_t scancode, bool pressed_not_released);
void parse_extended_scan_code(uint8_t scancode, bool pressed_not_released);

void parse_print_screen(bool pressed_not_released);
void parse_pause_pressed();

void keyboard_handler_scs1(uint8_t scancode, uint8_t port_number);
void keyboard_handler_scs2(uint8_t scancode, uint8_t port_number);
void keyboard_handler_scs3(uint8_t scancode, uint8_t port_number);

static inline void send_eoi([[gnu::unused]] uint8_t irq)
{
	apic_send_eoi();
	// PIC_sendEOI(irq);
}

kcb_led_state_t led_keys;
/* ========================================================Setup the handling of special keys
 * ===================================================================== */
bool		   is_key_pressed[SCANCODE_LEN] = {0};
bool		   is_print_screen_pressed		= false;
function_t_ptr key_functions[256];
bool		   is_key_tied_to_function[256] = {0};

bool		   is_extended_key_pressed[EXTENED_SCANCODE_LEN] = {0};
function_t_ptr extended_key_functions[256];
bool		   is_extended_key_tied_to_function[256] = {0};

void set_key_function(scancode_t key, function_t_ptr func)
{
	assert(validate_scancode(key), "key should be inside the thing\n");
	key_functions[key]			 = func;
	is_key_tied_to_function[key] = true;
}

void set_extended_key_function(extended_scancode_t scancode, function_t_ptr func)
{
	assert(validate_extended_scancode(scancode), "key should be inside the thing\n");
	extended_key_functions[scancode]		   = func;
	is_extended_key_tied_to_function[scancode] = true;
}

static void nop()
{
}

void setup_keyboard_extra()
{
	for (uint16_t i = 0; i < 256; i++)
	{
		key_functions[i]		   = nop;
		is_key_tied_to_function[i] = false;

		extended_key_functions[i]			= nop;
		is_extended_key_tied_to_function[i] = false;
	}

	// For the parts that changes leds
	set_key_function(SC_CAPSLOCK, pressed_caps_lock);
	set_key_function(SC_NUMLOCK, pressed_num_lock);
	set_key_function(SC_SCROLLLOCK, pressed_scroll_lock);

	// This should be handled by the keycodes handler in cpp (for regular keys)
	// TODO: MOVE THIS
	set_key_function(SC_A, pressed_a);
	set_extended_key_function(SCE_UP, pressed_up);
	set_extended_key_function(SCE_DOWN, pressed_down);
	set_extended_key_function(SCE_LEFT, pressed_left);
	set_extended_key_function(SCE_RIGHT, pressed_right);
	set_key_function(SC_R, pressed_r);
	set_key_function(SC_F, pressed_f);

	led_keys.CapsLock	= false;
	led_keys.NumberLock = false;
	led_keys.ScrollLock = false;
	led_keys.reserved	= false;
	set_leds(led_keys);

	ps2_keyboard_cpp_init();
}
/* ========================================================  The keyboard handler
 * ===================================================================== */

extern struct keyboard_porting_state_context kps;

extern inline void keyboard_handler(uint8_t scancode, uint8_t port_number)
{
	switch (kps.active_scancode_set)
	{
	case 1:
	{
		keyboard_handler_scs1(scancode, port_number);
		return;
	}
	case 2: return keyboard_handler_scs3(scancode, port_number);
	case 3: return keyboard_handler_scs3(scancode, port_number);
	default: abort_msg("Impossible scan scode set (%u)\n", kps.active_scancode_set);
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
void keyboard_handler_scs1(uint8_t scancode, uint8_t port_number)
{
	uint8_t keyboard_irq;
	if (port_number == 1)
	{
		keyboard_irq = PS2_PORT1_IRQ;
	}
	else
	{
		keyboard_irq = PS2_PORT2_IRQ;
	}
	// this can be optimized into a cmovs with immediates (no memory reads or branch)
	// Blazingly fast

	// kprintf("Scancode = %u:3, :%h:2, :%b:8\n", scancode, scancode, scancode);

	bool		extended_signal				= false;
	static bool previous_extended_signal[2] = {false, false}; //  index with port number
	// Use true/false for length 2 sequence (basic extended)
	static bool	   four_long_sequence[2]	  = {false, false};
	static bool	   six_long_sequence[2]		  = {false, false};
	static uint8_t current_sequence_length[2] = {0, 0};

	bool press_not_release = true;
	if (scancode == 0xe0)
	{
		extended_signal = true;
	}
	// =============== Len 2, 4 and 6 prelude
	if (extended_signal && !four_long_sequence[port_number] && !six_long_sequence[port_number])
	{
		previous_extended_signal[port_number] = extended_signal;
		send_eoi(keyboard_irq);
		return;
	}

	// ==================== Start of length 4 sequence dealing ===================
	const uint8_t print_screen_pressed_sequence[] = {0xE0, 0x2A, 0xE0, 0x37};
	const uint8_t psps1							  = print_screen_pressed_sequence[1];

	const uint8_t print_screen_released_sequence[] = {0xE0, 0xB7, 0xE0, 0xAA};
	const uint8_t psrs1							   = print_screen_released_sequence[1];

	if (previous_extended_signal[port_number] && !four_long_sequence[port_number])
	{
		if (scancode == psps1 || scancode == psrs1)
		{
			four_long_sequence[port_number]		  = true;
			current_sequence_length[port_number]  = 2;
			previous_extended_signal[port_number] = false;
			// kprintf("Start of 4 long sequence\n");
			send_eoi(keyboard_irq);
			return;
		}
	}

	if (four_long_sequence[port_number])
	{
		previous_extended_signal[port_number] = false;
		//
		uint8_t i = current_sequence_length[port_number];

		// kprintf("i = %u, scancode = %h:2, comp = %h:2\n", i, scancode, print_screen_pressed_sequence[i]);
		if (scancode == print_screen_pressed_sequence[i])
		{
			if (i == 3)
			{
				current_sequence_length[port_number] = 0;
				four_long_sequence[port_number]		 = false;
				parse_print_screen(true);
				send_eoi(keyboard_irq);
				return;
			}
			current_sequence_length[port_number]++;
			send_eoi(keyboard_irq);
			return;
		}
		else if (scancode == print_screen_released_sequence[i])
		{
			if (i == 3)
			{
				current_sequence_length[port_number] = 0;
				four_long_sequence[port_number]		 = false;
				parse_print_screen(false);
				send_eoi(keyboard_irq);
				return;
			}
			current_sequence_length[port_number]++;
			send_eoi(keyboard_irq);
			return;
		}
		else
		{
			current_sequence_length[port_number] = 0;
			four_long_sequence[port_number]		 = 0;
			send_eoi(keyboard_irq);
			return;
		}
	}
	// ==================== End of length 4 sequence dealing ===================

	// ==================== Start of length 6 sequence dealing ===================
	const uint8_t pause_pressed_sequence[] = {0xE1, 0x1D, 0x45, 0xE1, 0x9D, 0xC5};
	if (scancode == pause_pressed_sequence[0])
	{
		six_long_sequence[port_number] = true;
		current_sequence_length[port_number]++;
		previous_extended_signal[port_number] = false;
		send_eoi(keyboard_irq);
		return;
	}

	if (six_long_sequence[port_number])
	{
		previous_extended_signal[port_number] = false;
		uint8_t i							  = current_sequence_length[port_number];
		if (scancode == pause_pressed_sequence[i])
		{
			if (i == 5)
			{
				current_sequence_length[port_number] = 0;
				parse_pause_pressed();
				send_eoi(keyboard_irq);
				return;
			}

			current_sequence_length[port_number]++;
			send_eoi(keyboard_irq);
			return;
		}
	}
	// ==================== End of length 6 sequence dealing ===================

	// ==================== Start of length 2 sequence dealing ===================
	if (previous_extended_signal[port_number])
	{

		if (scancode >= EXTENDED_SCANCODE_RELEASE_START)
		{
			scancode -= EXTENDED_SCANCODE_RELEASE_OFFSET;
			press_not_release = false;
		}
		parse_extended_scan_code(scancode, press_not_release);
		previous_extended_signal[port_number] = extended_signal;
		send_eoi(keyboard_irq);
		return;
	}
	else
	{
		if (scancode >= SCANCODE_RELEASE_START)
		{
			scancode -= SCANCODE_RELEASE_OFFSET;
			press_not_release = false;
		}
		parse_scan_code(scancode, press_not_release);
		previous_extended_signal[port_number] = extended_signal;
		send_eoi(keyboard_irq);
		return;
	}
	// ==================== End of length 2 sequence dealing ===================
}

void keyboard_handler_scs2(uint8_t scancode, uint8_t port_number)
{
	kprintf("[PANIC-WARNING] scan code 2 handling not supported!\n");
}

void keyboard_handler_scs3(uint8_t scancode, uint8_t port_number)
{
	kprintf("[PANIC-WARNING] scan code 3 handling not supported!\n");
}

/* ================================= GET States of modifier keys ============================================ */

bool is_shift_held()
{
	return is_key_pressed[SC_LEFT_SHIFT] || is_key_pressed[SC_RIGHT_SHIFT];
}

bool is_ctrl_held()
{
	return is_key_pressed[SC_LEFT_CTRL] || is_extended_key_pressed[SCE_RIGHT_CTRL];
}

inline bool is_alt_held()
{
	return is_key_pressed[SC_LEFT_ALT];
}

inline bool is_altgr_held()
{
	return is_extended_key_pressed[SCE_RIGHT_ALT];
}

inline bool is_capslock_on()
{
	return led_keys.CapsLock;
}

inline bool is_caps()
{
	return is_shift_held() || is_capslock_on();
}

/* =================================== Get the modifier state ===================== */
struct KeyModifierState get_current_mod_state()
{
	bool left_shift_pressed	 = is_key_pressed[SC_LEFT_SHIFT];
	bool right_shift_pressed = is_key_pressed[SC_RIGHT_SHIFT];

	bool left_ctrl_pressed	= is_key_pressed[SC_LEFT_CTRL];
	bool right_ctrl_pressed = is_extended_key_pressed[SCE_RIGHT_CTRL];

	bool left_meta_pressed	= is_extended_key_pressed[SCE_LEFT_META];
	bool right_meta_pressed = is_extended_key_pressed[SCE_RIGHT_META];
	bool alt_pressed		= is_key_pressed[SC_LEFT_ALT];
	bool altgr_pressed		= is_extended_key_pressed[SCE_RIGHT_ALT];

	bool menu_app_pressed	 = is_extended_key_pressed[SCE_APPS];
	bool capslock_pressed	 = is_key_pressed[SC_CAPSLOCK];
	bool scroll_lock_pressed = is_key_pressed[SC_SCROLLLOCK];

	struct KeyModifierState state = {
		.left_shift	 = left_shift_pressed,
		.right_shift = right_shift_pressed,

		.left_ctrl	= left_ctrl_pressed,
		.right_ctrl = right_ctrl_pressed,

		.left_meta	= left_meta_pressed,
		.right_meta = right_meta_pressed,
		.menu_app	= menu_app_pressed,

		.alt   = alt_pressed,
		.altgr = altgr_pressed,

		.capslock	 = capslock_pressed,
		.scroll_lock = scroll_lock_pressed,
	};

	return state;
}

/* ================================= Parse Scan Codes ============================================ */
void parse_scan_code(uint8_t press_scancode, bool pressed_not_released)
{
	const char *scancode_name = scancode_to_string(press_scancode);
	// kprintf("(pressed = %b): Scancode  %u:3, %h:2, %b:8 = |%s|\n", pressed_not_released, press_scancode, press_scancode, press_scancode,
	//     scancode_name);

	// kprintf("Scancode %h:2 = |%s|\n", press_scancode, scancode_name);

	if (pressed_not_released)
	{
		if (is_key_tied_to_function[press_scancode])
		{
			key_functions[press_scancode]();
		}

		struct KeyCode			keycode		= scancode_to_keycode(press_scancode);
		struct KeyModifierState state		= get_current_mod_state();
		bool					first_press = !is_key_pressed[press_scancode];
		keycode_handler(keycode, state, first_press, true);
		is_key_pressed[press_scancode] = true;
	}
	else
	{
		is_key_pressed[press_scancode]	= false;
		struct KeyCode			keycode = scancode_to_keycode(press_scancode);
		struct KeyModifierState state	= get_current_mod_state();
		keycode_handler(keycode, state, false, false);
	}
}
void parse_extended_scan_code(uint8_t press_scancode, bool pressed_not_released)
{

	const char *scancode_name = extended_scancode_to_string(press_scancode);
	// kprintf("Scancode %u = |%s|\n", press_scancode, scancode_name);

	if (pressed_not_released)
	{

		if (is_extended_key_tied_to_function[press_scancode])
		{
			extended_key_functions[press_scancode]();
		}

		struct KeyCode			keycode		= extended_scancode_to_keycode(press_scancode);
		struct KeyModifierState state		= get_current_mod_state();
		bool					first_press = !is_extended_key_pressed[press_scancode];
		keycode_handler(keycode, state, first_press, true);
		is_extended_key_pressed[press_scancode] = true;
	}
	else
	{
		is_extended_key_pressed[press_scancode] = false;
		struct KeyCode			keycode			= extended_scancode_to_keycode(press_scancode);
		struct KeyModifierState state			= get_current_mod_state();
		keycode_handler(keycode, state, false, false);
	}
}

void parse_print_screen(bool pressed_not_released)
{
	// kprintf("You pressed=%b print screen\n", pressed_not_released);

	if (pressed_not_released)
	{
		struct KeyCode			keycode = print_screen_to_keycode();
		struct KeyModifierState state	= get_current_mod_state();
		keycode_handler(keycode, state, !is_print_screen_pressed, true);
	}
	else
	{

		struct KeyCode			keycode = print_screen_to_keycode();
		struct KeyModifierState state	= get_current_mod_state();
		keycode_handler(keycode, state, false, false);
	}

	is_print_screen_pressed = pressed_not_released;
}
void parse_pause_pressed()
{
	struct KeyCode			keycode = pause_to_keycode();
	struct KeyModifierState state	= get_current_mod_state();
	keycode_handler(keycode, state, true, true);
}
