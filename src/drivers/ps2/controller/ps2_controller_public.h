#pragma once
#include "intrinsics.h"
#include "stdint.h"

// #define PS2_DATA_PORT_RW 0x60   // Read and write
// #define PS2_STATUS_PORT_R 0x64  // read.
// #define PS2_COMMAND_PORT_W 0x64 // write

enum ps2_controller_error_code {
	PS2_ERR_none = 0,
	PS2_ERR_invalid_port_number = -1,
	PS2_ERR_wait_max_itt_in = 1,
	PS2_ERR_wait_max_itt_out = 2,
	PS2_ERR_invalid_test_port_response = 3,
	PS2_ERR_invalid_test_controller_response = 4,
	PS2_ERR_invalid_configuration_byte = 5,
	PS2_ERR_status_parity = 6,
	PS2_ERR_status_timeout = 7,

	PS2_ERR_device_command_failed_to_acknowledge = 8,
	PS2_ERR_device_command_failed_self_test = 9,
	PS2_ERR_device_rep3_invalid = 10,
	PS2_ERR_device_rep4_invalid = 10,

	// Warnings
	PS2_WARN_A20_line_not_set = 1000,
	PS2_WARN_invalid_configuration_byte_post_is_zero = 999,
	PS2_WARN_n_is_zero = 998,

};

enum ps2_device_type {
	PS2_DT_ancient_at_keyboard,
	PS2_DT_standard_mouse,
	PS2_DT_mouse_with_scroll_wheel,
	PS2_DT_mouse_with_5_button, // regular gaming mouse, left, right, scroll, mouse4, mouse5
	PS2_DT_mf2_keyboard_1,      // 0xAB, 0x83
	PS2_DT_mf2_keyboard_2,      // 0xAB, 0xC1
	PS2_DT_short_keyboard,
	PS2_DT_122_key_host_connected,
	PS2_DT_122_key,
	PS2_DT_japanese_g_keyboard,
	PS2_DT_japanese_p_keyboard,
	PS2_DT_japanese_a_keyboard,
	PS2_DT_ncd_sun_layout_keyboard,
};
enum ps2_device_super_type {
	PS2_DST_unknown = 0,
	PS2_DST_keyboard = 1,
	PS2_DST_mouse = 2,
};

struct ps2_device_type_verified {
	enum ps2_controller_error_code err;
	enum ps2_device_type type;
	enum ps2_device_super_type mouse_or_keyboard;
};

// ================================== The accessible functions
struct ps2_verified_response_test_controller ps2_perform_controller_self_test();

// ================================= The to strings
static inline const char* PS2_OS_Error_to_string(enum ps2_controller_error_code err) {
	switch (err) {
	case PS2_ERR_none:
		return "PS2_ERR_NONE";
	case PS2_ERR_invalid_port_number:
		return "PS2_ERR_INVALID_PORT_NUMBER";
	case PS2_ERR_wait_max_itt_in:
		return "PS2_ERR_WAIT_MAX_ITT_IN";
	case PS2_ERR_wait_max_itt_out:
		return "PS2_ERR_WAIT_MAX_ITT_OUT";
	case PS2_ERR_invalid_test_port_response:
		return "PS2_ERR_INVALID_TEST_PORT_RESPONSE";
	case PS2_ERR_invalid_test_controller_response:
		return "PS2_ERR_INVALID_TEST_CONTROLLER_RESPONSE";
	case PS2_ERR_invalid_configuration_byte:
		return "PS2_ERR_INVALID_CONFIGURATION_BYTE";
	case PS2_ERR_status_parity:
		return "PS2_ERR_STATUS_PARITY";
	case PS2_ERR_status_timeout:
		return "PS2_ERR_STATUS_TIMEOUT";

	// Warnings
	case PS2_WARN_A20_line_not_set:
		return "PS2_WARN_A20_LINE_NOT_SET";
	case PS2_WARN_n_is_zero:
		return "PS2_WARN_N_IS_ZERO";

	case PS2_WARN_invalid_configuration_byte_post_is_zero:
		return "PS2_WARN_invalid_configuration_byte_post_is_zero";

	default:
		return "Unknown PS2 error code!";
	}
}
