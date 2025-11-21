#include "ps2_controller.h"
#include "ps2_keyboard.h"
#include "ps2_keyboard_public.h"

// This is used to send data to the first or second ps2 device. Port 1 = Keyboard, Port 2 = mouse
extern enum ps2_controller_error_code send_command_or_data_to_ps2_port(enum PS2_PortNumber port_number, uint8_t data);

uint8_t _number_of_keyboard;
uint8_t _single_keyboard_port;

uint8_t _keyboard1_port;
uint8_t _keyboard2_port;

// SKS = Single keyboard support!
//
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

/* ==================================== The two string =============================== */
const char* ps2_keyboard_error_to_string(enum ps2_keyboard_error_code code) {
	switch (code) {
	case PS2_KB_ERR_none:
		return "No error";
	case PS2_KB_ERR_invalid_command:
		return "Invalid command";
	case PS2_KB_ERR_could_not_send_command:
		return "Could not send command";
	case PS2_KB_ERR_could_not_recieve_response:
		return "Could not receive response";
	case PS2_KB_ERR_resend_too_much:
		return "Resend attempted too many times";
	case PS2_KB_ERR_non_ack_response:
		return "Non-acknowledged response";
	case PS2_KB_ERR_response_was_not_echo:
		return "Response was not echo";
	case PS2_KB_ERR_invalid_scancode:
		return "Invalid scancode";
	default:
		return "Unknown error";
	}
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

		enum ps2_controller_error_code c_err;

		c_err = _send_command_or_data_to_ps2_keyboard(data);
		if (c_err) {
			ret.err = PS2_KB_ERR_could_not_send_command;
			return ret;
		}
		c_err = wait_till_ready_for_response();
		if (c_err) {
			kprintf("The ps2_controller error: %d, |%s|\n", c_err, PS2_OS_Error_to_string(c_err));
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

enum ps2_keyboard_error_code echo_keyboard() {
	struct ps2_keyboard_verified_response obtained;
	obtained = send_command_to_ps2_keyboard(KCB_echo);
	if (obtained.err != PS2_KB_ERR_non_ack_response) {
		return obtained.err;
	}
	if (obtained.response != KRB_echo_response) {
		return PS2_KB_ERR_response_was_not_echo;
	}
	kprintf("The echo response: %h\n", obtained.response);
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

	obtained = send_data_to_ps2_keyboard((uint8_t)scan_code_set);
	if (obtained.err) {
		return obtained.err;
	}
	return PS2_KB_ERR_none;
}

struct ps2_keyboard_verified_scan_code_set get_scan_code_set() {

	struct ps2_keyboard_verified_response obtained;
	struct ps2_keyboard_verified_scan_code_set ret;
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

struct ps2_keyboard_verified_response identify_device() {
	abort_msg("!Not implemented!\n");
	// TODO: Need to implement a recieve response from identify device function.
	// By moving the code inside
	// reset_port_and_get_device_type(PS2_PN_port_two);
	// to an internal function that sends the reset and recieve the fist 2 response
	// one that an internal function that receieve the get device type.
	// and a master function that does the steps from detecing divice types. (Pre: Disable scanning, End: Enable scanning)
	// from PS2 Controller OSDEV Wiki page

	// Then from here, i can just call the master on _single_keyboard_port
}

/* ================================ Step function ========================= */

/* ================================= Quick and dirty ========================== */

#include <stdbool.h>
#include <stdint.h>

/* PS/2 Ports */
#define PS2_DATA_PORT_RW 0x60
#define PS2_STATUS_PORT 0x64

/* Status register bits */
#define PS2_STATUS_OUTPUT_BUFFER_FULL 0x01
#define PS2_STATUS_INPUT_BUFFER_FULL 0x02

/* Commands */
#define PS2_CMD_ECHO 0xEE
#define PS2_CMD_SET_SCANCODE 0xF0
#define PS2_CMD_GET_SCANCODE 0xF0
#define PS2_ACK 0xFA

/* Simple outb/inb functions for x86 */
static inline void outb(uint16_t port, uint8_t val) {
	__asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
	uint8_t ret;
	__asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

/* Wait until keyboard input buffer is empty */
static void wait_input_empty(void) {
	while (inb(PS2_STATUS_PORT) & PS2_STATUS_INPUT_BUFFER_FULL)
		;
}

/* Wait until keyboard output buffer has data */
static void wait_output_full(void) {
	while (!(inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_BUFFER_FULL))
		;
}

/* Send a command to the keyboard */
static void ps2_send_command(uint8_t cmd) {
	wait_input_empty();
	outb(PS2_DATA_PORT_RW, cmd);
}

/* Read a byte from the keyboard */
static uint8_t ps2_read_response(void) {
	wait_output_full();
	return inb(PS2_DATA_PORT_RW);
}

/* Send a command and check ACK */
static bool ps2_send_cmd_check_ack(uint8_t cmd) {
	ps2_send_command(cmd);
	return ps2_read_response() == PS2_ACK;
}

/* Set scan code set */
static bool ps2_set_scancode_set(uint8_t set_number) {
	if (!ps2_send_cmd_check_ack(PS2_CMD_SET_SCANCODE))
		return false;
	if (!ps2_send_cmd_check_ack(set_number))
		return false;
	return true;
}

/* Get current scan code set */
int ps2_get_scancode_set(void) {
	ps2_send_command(PS2_CMD_GET_SCANCODE); // 0xF0
	ps2_send_command(0x00);                 // Get current
	uint8_t ack = ps2_read_response();      // should be 0xFA
	if (ack != PS2_ACK)
		return -1;
	return ps2_read_response(); // scan code set number
}

/* Quick test */
void test_scancode_set(uint8_t set_value) {
	kprintf("\n===== Scan Code Set %u Test =====\n", set_value);

	// Echo check first
	ps2_send_command(PS2_CMD_ECHO);
	uint8_t echo_resp = ps2_read_response();
	if (echo_resp != PS2_CMD_ECHO) {
		kprintf("Keyboard echo failed!\n");
		return;
	}

	kprintf("Keyboard echo OK\n");

	// Set scan code set 1
	if (!ps2_set_scancode_set(set_value)) {
		kprintf("Failed to set scan code set 1\n");
		return;
	}
	kprintf("Scan code set %u sent\n", set_value);

	// Get scan code set
	int current_set = ps2_get_scancode_set();
	if (current_set < 0) {
		kprintf("Failed to read current scan code set\n");
		return;
	}

	kprintf("Current scan code set: %d\n", current_set);
	if (current_set == set_value) {
		kprintf("Scan code set verified successfully!\n");
	} else {
		kprintf("Scan code set verification failed!\n");
	}
}
