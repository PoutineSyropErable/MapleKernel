#pragma once
#include "ps2_controller_public.h"
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
	PS2_KB_ERR_self_test_failed,
};

typedef enum ScanCodeSet {
	SCS_get_current = 0,
	SCS_set_scan_code_set_1 = 1,
	SCS_set_scan_code_set_2 = 2,
	SCS_set_scan_code_set_3 = 3,
} kcb_scan_code_set_t;
// don't need a uint equivalent, because this doesn't go into a packed struct with bit fields

struct ps2_keyboard_verified_scan_code_set {
	enum ps2_keyboard_error_code err;
	enum ScanCodeSet response;
};

struct ps2_keyboard_type_verified {
	enum ps2_keyboard_error_code err;
	enum ps2_device_type type;
};

void set_single_keyboard_port(uint8_t single_keyboard_port);
void set_dual_keyboard_port();

const char* ps2_keyboard_error_to_string(enum ps2_keyboard_error_code code);

enum ps2_keyboard_error_code echo_keyboard();
struct ps2_keyboard_verified_scan_code_set get_scan_code_set();
enum ps2_keyboard_error_code set_scan_code_set(enum ScanCodeSet scan_code_set);

void test_echo_quick();

void test_scancode_set(uint8_t set_value);

int ps2_get_scancode_set(void);

void disable_keyboard();
void enable_keyboard();

enum ps2_keyboard_error_code setup_keyboard();
void test_ps2_keyboard_commands();

struct keyboard_porting_state_context {
	bool initialized;
	uint8_t number_of_keyboards;

	uint8_t active_port;
	uint8_t active_irq;
	uint8_t active_interrupt_vector;

	uint8_t active_scancode_set;
};
