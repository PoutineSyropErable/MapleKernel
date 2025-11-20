#pragma once
#include "stdio.h"
#include <stddef.h>
#include <stdint.h>
/*
The uint8_t type must always be casted to the enum once it's obtained from the struct.



*/

/* ===================================================*/

typedef enum KeyboardCommandByte {
	KCB_setLEDs = 0xED,
	KCB_echo = 0xEE,
	KCB_get_set_ScanCodeSet = 0xF0,
	KCB_identify_keyboard = 0xF2,
	KCB_set_typematic_rate_delay = 0xF3,
	KCB_enable_scanning = 0xF4,
	KCB_disable_scanning = 0xF5, // May also reset to default parameters
	KCB_set_default_parameters = 0xF6,
	KCB_set_all_to_typematic_autorepeat = 0xF7,
	KCB_set_all_to_make_release = 0xF8,
	KCB_set_all_to_make_only = 0xF9,
	KCB_set_all_to_typematic_autorepeat_make_release = 0xFA,
	KCB_set_specific_to_typematic_autorepeat = 0xFB,
	KCB_set_specific_to_make_release = 0xFC,
	KCB_set_specific_to_make_only = 0xFD,
	KCB_resend_last_byte = 0xFE,
	KCB_reset_and_self_test = 0xFF
} KeyboardCommandByte_t;
typedef uint8_t keyboard_command_byte_t;

typedef struct [[gnu::packed]] {
	bool ScrollLock : 1;
	bool NumberLock : 1;
	bool CapsLock : 1;
	uint8_t reserved : 5;
} kcb_led_state_t;

_Static_assert(sizeof(kcb_led_state_t) == 1, "kcb_led_sate_t must be of size: 1 byte");

typedef enum ScanCodeSet {
	SCS_get_current = 0,
	SCS_set_scan_code_set_1 = 1,
	SCS_set_scan_code_set_2 = 2,
	SCS_set_scan_code_set_3 = 3,
} kcb_scan_code_set_t;
// don't need a uint equivalent, because this doesn't go into a packed struct with bit fields

typedef enum KeyRepeatDelay {
	KRD_250ms = 0b00,
	KRD_500ms = 0b01,
	KRD_750ms = 0b11,
	KRB_1000ms = 0b11
} KeyRepeatDelay_t;
typedef uint8_t key_repeat_delay_t;

typedef struct [[gnu::packed]] {
	uint8_t repeat_rate : 5; // 30 - repeat_rate == true_repeat_rate
	key_repeat_delay_t delay_repeat : 2;
	bool must_be_zero : 1;
} kcb_typematic_rate_and_delay_t;
_Static_assert(sizeof(kcb_typematic_rate_and_delay_t) == 1, "kcb_typematic_rate_and_delay_t must be of size: 1 byte");

/* ===================================================*/

typedef enum KeyboardResponseByte {
	KRB_key_dectection_error = 0x00,  // or Internal Buffer Overrun
	KRB_self_test_passed = 0xAA,      //
	KRB_echo_response = 0xEE,         //
	KRB_command_acknowledge = 0xFA,   // (ACK)
	KRB_self_test_failed = 0xFC,      // Ack And Resend are very popular responses
	KRB_self_test_failed2 = 0xFD,     //
	KRB_resend = 0xFE,                // (Resend)
	KRB_key_dectection_error2 = 0xFF, // or Internal Buffer Overrun
} KeyboardResponseByte_t;
typedef uint8_t keyboard_response_byte_t;

/* ===================================================*/
