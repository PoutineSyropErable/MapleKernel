#include "ps2.h"
#include "ps2_keyboard.h"
#include "ps2_keyboard_public.h"

// This is used to send data to the first or second ps2 device. Port 1 = Keyboard, Port 2 = mouse
extern enum ps2_controller_error_code send_command_or_data_to_ps2_port(enum PS2_PortNumber port_number, uint8_t data);

uint8_t _number_of_keyboard;
uint8_t _single_keyboard_port;

uint8_t _keyboard1_port;
uint8_t _keyboard2_port;

// Single keyboard support
// Because I found it too boring, let's only really support 1 ps2 keyboard.
// Supporting two here is just needless complexity. And I want speed, not perfect code. The ps2 controller driver is good enough.
// At the end of the day, motivation is king.
void sks() {
	assert(_number_of_keyboard == 1, "We don't support two keyboards!\n");
}

void set_single_keyboard_port(uint8_t single_keyboard_port) {
	_single_keyboard_port = single_keyboard_port;
	_number_of_keyboard = 1;
}

void set_dual_keyboard_port() {
	_keyboard1_port = 1;
	_keyboard2_port = 2;
	_number_of_keyboard = 2;
	// True by definition and hardware
	sks();
}

/* ================================== The validates ============================= */
bool validate_keyboard_command(uint8_t cmd) {
	switch ((KeyboardCommandByte_t)cmd) {
	case KCB_setLEDs:
	case KCB_echo:
	case KCB_get_set_ScanCodeSet:
	case KCB_identify_keyboard:
	case KCB_set_typematic_rate_delay:
	case KCB_enable_scanning:
	case KCB_disable_scanning:
	case KCB_set_default_parameters:
	case KCB_set_all_to_typematic_autorepeat:
	case KCB_set_all_to_make_release:
	case KCB_set_all_to_make_only:
	case KCB_set_all_to_typematic_autorepeat_make_release:
	case KCB_set_specific_to_typematic_autorepeat:
	case KCB_set_specific_to_make_release:
	case KCB_set_specific_to_make_only:
	case KCB_resend_last_byte:
	case KCB_reset_and_self_test:
		return true;
	default:
		return false;
	}
}

static inline bool validate_scan_code_set(kcb_scan_code_set_t set) {
	return set >= SCS_get_current && set <= SCS_set_scan_code_set_3;
}

// Validate key repeat delay
static inline bool validate_key_repeat_delay(KeyRepeatDelay_t delay) {
	return delay >= KRD_250ms && delay <= KRD_1000ms;
}

/* ================================ The main command ================================= */
static inline enum ps2_controller_error_code
_send_command_or_data_to_ps2_keyboard(enum KeyboardCommandByte command) {
	return send_command_or_data_to_ps2_port(_single_keyboard_port, command);
}

// note that in practice, this is the exact same function. So, it should be optimizable into the other?
// The function name is just to simplify stuff
static struct ps2_keyboard_verified_response send_data_to_ps2_keyboard(uint8_t data) {

	struct ps2_keyboard_verified_response ret;

	uint8_t response_count = 0;
	const uint8_t max_response_repeat = 3;
	// prefer a while loop.
	// This way, I can explicitely compare to resend
	while (response_count < max_response_repeat) {

		enum ps2_controller_error_code c_err = _send_command_or_data_to_ps2_keyboard(data);
		if (c_err) {
			ret.err = PS2_KB_ERR_could_not_send_command;
			return ret;
		}
		c_err = wait_till_ready_for_response();
		if (c_err) {
			ret.err = PS2_KB_ERR_could_not_recieve_response;
			return ret;
		}
		uint8_t response = ps2_recieve_raw_response();
		enum KeyboardResponseByte response_typed = response;
		// should be portable and optimized out

		if (response_typed == KRB_resend) {
			response_count++;
		} else if (response_typed == KRB_command_acknowledged) {
			ret.err = PS2_KB_ERR_none;
			ret.response = response;
			return ret;
		} else {
			ret.err = PS2_KB_ERR_non_ack_response;
			ret.response = response;
			return ret;
		}
	}

	ret.err = PS2_KB_ERR_resend_too_much;
	return ret;
}

static inline struct ps2_keyboard_verified_response send_command_to_ps2_keyboard(enum KeyboardCommandByte command) {

#define ERROR_CHECK_VKC
#ifdef ERROR_CHECK_VKC
	bool valid = validate_keyboard_command(command);
	if (!valid) {
		return (struct ps2_keyboard_verified_response){.err = PS2_KB_ERR_invalid_command};
	}
#endif

	return send_data_to_ps2_keyboard(command);
}

/* ====================================== The applications ============================ */

enum ps2_keyboard_error_code set_leds(kcb_led_state_t led_sate) {
	struct ps2_keyboard_verified_response obtained;
	obtained = send_command_to_ps2_keyboard(KCB_setLEDs);
	if (obtained.err) {
		return obtained.err;
	}
	union kcb_led_state_uts led_sate_ts = {.value = led_sate};

	obtained = send_data_to_ps2_keyboard(led_sate_ts.raw);
	if (obtained.err) {
		return obtained.err;
	}
	return PS2_KB_ERR_none;
}

enum ps2_keyboard_error_code echo() {
	struct ps2_keyboard_verified_response obtained;
	obtained = send_command_to_ps2_keyboard(KCB_echo);
	if (obtained.err != PS2_KB_ERR_non_ack_response) {
		return obtained.err;
	}
	if (obtained.response != KRB_echo_response) {
		return PS2_KB_ERR_response_was_not_echo;
	}
	return PS2_KB_ERR_none;
}

enum ps2_keyboard_error_code set_scan_code_set(enum ScanCodeSet scan_code_set) {
	if (scan_code_set == SCS_get_current || scan_code_set > SCS_set_scan_code_set_3) {
#define ABORT_ON_VSCS
#ifdef ABORT_ON_VSCS
		kprintf("can't set with scancode = %u\n", scan_code_set);
		abort_msg("Invalid scan code set! Must be 1,2 or 3\n");
#endif

		return PS2_KB_ERR_invalid_scancode;
	}

	struct ps2_keyboard_verified_response obtained;
	obtained = send_command_to_ps2_keyboard(KCB_get_set_ScanCodeSet);
	if (obtained.err) {
		return obtained.err;
	}

	obtained = send_data_to_ps2_keyboard(scan_code_set);
	if (obtained.err) {
		return obtained.err;
	}
	return PS2_KB_ERR_none;
}

union ps2_keyboard_verified_scan_code_set get_scan_code_set() {

	struct ps2_keyboard_verified_response obtained;
	union ps2_keyboard_verified_scan_code_set ret;
	obtained = send_command_to_ps2_keyboard(KCB_get_set_ScanCodeSet);
	if (obtained.err) {
		ret.err = obtained.err;
		return ret;
	}

	obtained = send_data_to_ps2_keyboard(SCS_get_current);
	if (obtained.err != PS2_KB_ERR_non_ack_response) {
		ret.err = obtained.err;
		return ret;
	}

	enum ScanCodeSet scan_code_set = (enum ScanCodeSet)obtained.response;
	if (scan_code_set == SCS_get_current || scan_code_set > SCS_set_scan_code_set_3) {

		ret.err = PS2_KB_ERR_invalid_scancode;
		return ret;
	}

	ret.err = PS2_KB_ERR_none;
	ret.response = scan_code_set;
	return ret;
}

struct ps2_device_type_verified identify_device() {
	struct ps2_device_type_verified ret;
	struct ps2_keyboard_verified_response response;
	enum ps2_controller_error_code err;

	response = send_command_to_ps2_keyboard(KCB_identify_keyboard);
	if (response.err) {
		ret.err = response.err;
		return ret;
	}
	err = wait_till_ready_for_response();
	if (err) {
		ret.err = err;
		return ret;
	}
	uint8_t rep1 = ps2_recieve_raw_response();
	if (rep1 != DEVICE_COMMAND_ACKNOLEDGED) {
		ret.err = PS2_ERR_device_command_failed_to_acknowledge;
		return ret;
	}

	err = wait_till_ready_for_response();
	if (err) {
		ret.err = err;
		return ret;
	}
	uint8_t rep2 = ps2_recieve_raw_response();
	if (rep2 != DEVICE_COMMAND_SELF_TEST_PASSED) {
		ret.err = PS2_ERR_device_command_failed_self_test;
		return ret;
	}

	err = wait_till_ready_for_response();
	if (err) {
		// If the third one is none, then it's an ancient keyboard.
		// so a timeout error would happen here.
		// Idk if a timeout error could happen in another case, but...
		// Kinda weird that the error is the expected behavior
		// But I guess that's what it's all about, they need to be handled eventually.
		// WIth something different then just return error one layer deeper or abort os.
		ret.type = PS2_DT_ancient_at_keyboard;
		ret.mouse_or_keyboard = PS2_DST_keyboard;
		ret.err = PS2_ERR_none;
		return ret;
	}

	uint8_t rep3 = ps2_recieve_raw_response();
	const uint8_t len4_prefix = 0xAB;
	if (rep3 != len4_prefix) {
		enum len3_device_byte {
			STANDARD_PS2_MOUSE = 0x00,
			MOUSE_WITH_SCROLL_WHEEL = 0x03,
			FIVE_BUTTON_MOUSE = 0x04,
		};

		ret.mouse_or_keyboard = PS2_DST_mouse;
		switch (rep3) {
		case STANDARD_PS2_MOUSE:
			ret.type = PS2_DT_standard_mouse;
			ret.err = PS2_ERR_none;
			return ret;
		case MOUSE_WITH_SCROLL_WHEEL:
			ret.type = PS2_DT_mouse_with_scroll_wheel;
			ret.err = PS2_ERR_none;
			return ret;
		case FIVE_BUTTON_MOUSE:
			ret.type = PS2_DT_mouse_with_5_button;
			ret.err = PS2_ERR_none;
			return ret;
		default:
			ret.type = rep3;
			ret.err = PS2_ERR_device_rep3_invalid;
			return ret;
		}
	}

	err = wait_till_ready_for_response();
	if (err) {
		ret.err = err;
		return ret;
	}
	uint8_t rep4 = ps2_recieve_raw_response();
	enum len4_device_byte {
		MF2_KEYBOARD_1 = 0x83,
		MF2_KEYBOARD_2 = 0xC1,
		MF2_KEYBOARD_1_TRANSLATED = 0x41,

		SHORT_KEYBOARD = 0x84,
		SHORT_KEYBOARD_TRANSLATED = 0x54,

		KEY122_HOST_CONNECTED = 0x85,
		KEY122 = 0x86,

		JAPANESE_G = 0x90,
		JAPANESE_P = 0x91,
		JAPANESE_A = 0x92,

		NCD_SUN = 0xA1
	};

	ret.mouse_or_keyboard = PS2_DST_keyboard;
	switch (rep4) {
	case MF2_KEYBOARD_1:
		ret.type = PS2_DT_mf2_keyboard_1;
		ret.err = PS2_ERR_none;
		return ret;
	case MF2_KEYBOARD_1_TRANSLATED:
		ret.type = PS2_DT_mf2_keyboard_1;
		ret.err = PS2_ERR_none;
		return ret;

	case MF2_KEYBOARD_2:
		ret.type = PS2_DT_mf2_keyboard_2;
		ret.err = PS2_ERR_none;
		return ret;

	case SHORT_KEYBOARD:
		ret.type = PS2_DT_short_keyboard;
		ret.err = PS2_ERR_none;
		return ret;
	case SHORT_KEYBOARD_TRANSLATED:
		ret.type = PS2_DT_short_keyboard;
		ret.err = PS2_ERR_none;
		return ret;

	case KEY122_HOST_CONNECTED:
		ret.type = PS2_DT_122_key_host_connected;
		ret.err = PS2_ERR_none;
		return ret;
	case KEY122:
		ret.type = PS2_DT_122_key;
		ret.err = PS2_ERR_none;
		return ret;

	case JAPANESE_G:
		ret.type = PS2_DT_japanese_g_keyboard;
		ret.err = PS2_ERR_none;
		return ret;
	case JAPANESE_P:
		ret.type = PS2_DT_japanese_p_keyboard;
		ret.err = PS2_ERR_none;
		return ret;
	case JAPANESE_A:
		ret.type = PS2_DT_japanese_a_keyboard;
		ret.err = PS2_ERR_none;
		return ret;

	case NCD_SUN:
		ret.type = PS2_DT_ncd_sun_layout_keyboard;
		ret.err = PS2_ERR_none;
		return ret;

	default:
		ret.type = rep4;
		ret.err = PS2_ERR_device_rep4_invalid;
		return ret;
	}
}
