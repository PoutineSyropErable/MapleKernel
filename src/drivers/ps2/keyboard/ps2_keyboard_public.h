#pragma once
#include <stdint.h>
enum ps2_keyboard_error_code {
	PS2_KB_ERR_none = 0,
	PS2_KB_ERR_invalid_command,
	PS2_KB_ERR_could_not_send_command,
	PS2_KB_ERR_could_not_recieve_response,
	PS2_KB_ERR_resend_too_much,
	PS2_KB_ERR_non_ack_response, // Not command acknowledged
	PS2_KB_ERR_response_was_not_echo,

	PS2_KB_ERR_invalid_scancode,
};

typedef enum ScanCodeSet {
	SCS_get_current = 0,
	SCS_set_scan_code_set_1 = 1,
	SCS_set_scan_code_set_2 = 2,
	SCS_set_scan_code_set_3 = 3,
} kcb_scan_code_set_t;
// don't need a uint equivalent, because this doesn't go into a packed struct with bit fields

union ps2_keyboard_verified_scan_code_set {
	enum ps2_keyboard_error_code err;
	enum ScanCodeSet response;
};

enum ps2_keyboard_error_code echo();
void set_single_keyboard_port(uint8_t single_keyboard_port);
void set_dual_keyboard_port();

union ps2_keyboard_verified_scan_code_set get_scan_code_set();
enum ps2_keyboard_error_code set_scan_code_set(enum ScanCodeSet scan_code_set);
