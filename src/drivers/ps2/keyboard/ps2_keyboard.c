#include "idt_public.h"
#include "pic.h"
#include "ps2_controller.h"
#include "ps2_keyboard.h"
#include "ps2_keyboard_public.h"

// This is used to send data to the first or second ps2 device. Port 1 = Keyboard, Port 2 = mouse
extern enum ps2_controller_error_code send_command_or_data_to_ps2_port(enum PS2_PortNumber port_number, uint8_t data);

uint8_t _number_of_keyboard;
uint8_t _single_keyboard_port;
uint8_t _single_keyboard_irq;
uint8_t _single_keyboard_idt_vector;

static inline void _set_irq_and_vector() {

	if (_single_keyboard_port == 1) {
		_single_keyboard_idt_vector = PS2_PORT1_INTERUPT_VECTOR;
		_single_keyboard_irq = PS2_PORT1_IRQ;
	} else {
		_single_keyboard_idt_vector = PS2_PORT2_INTERUPT_VECTOR;
		_single_keyboard_irq = PS2_PORT2_IRQ;
	}
}

void set_single_keyboard_port(uint8_t single_keyboard_port) {
	_single_keyboard_port = single_keyboard_port;
	_number_of_keyboard = 1;

	_set_irq_and_vector();
}

uint8_t _keyboard1_port;
uint8_t _keyboard2_port;

void set_dual_keyboard_port() {
	_keyboard1_port = 1;
	_keyboard2_port = 2;
	_number_of_keyboard = 2;
	kprintf("[PANIC-WARNING] Dual keyboard ports commands works by setting the active keyboard\n");
	kprintf("[PANIC-WARNING] No currently selected keyboard");
	// True by definition and hardware
}

void set_active_keyboard_for_commands(uint8_t keyboard_idx) {
	assert(_number_of_keyboard > 1,
	       "Makes no sense to change the active keyboard!\nThis was wrongly called with: keyboard_idx = %u, _number_of_keyboard = %u\n",
	       keyboard_idx, _number_of_keyboard);
	assert(keyboard_idx <= _number_of_keyboard, "keyboard_idx (%u) must be <= _number_of_keyboard (%u)!\n",
	       keyboard_idx, _number_of_keyboard);

	_single_keyboard_port = keyboard_idx;
	_set_irq_and_vector();
}

/*
Using this without masking irqs and then sending keyboard commands will cause a #GP 13.
*/
void disable_keyboard_interrupts() {
	disable_idt_entry(_single_keyboard_idt_vector);
}

void enable_keyboard_interrupts() {
	enable_idt_entry(_single_keyboard_idt_vector);
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
	case KCB_set_specific_to_typematic_autorepeat_only:
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
		return "Invalid command (Runtime check in static inline wrapper)";
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

// This is to send data to the ps2 keyboard. Data can be either commands
// Or real actual data.
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
			ret.response = -1;
			return ret;
		}
		c_err = wait_till_ready_for_response();
		if (c_err) {
			kprintf("The ps2_controller error: %d, |%s|\n", c_err, PS2_OS_Error_to_string(c_err));
			ret.err = PS2_KB_ERR_could_not_recieve_response;
			ret.response = -1;
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

	ret.response = -1;
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

static inline struct ps2_keyboard_verified_response send_setter_subcommand_to_ps2_keyboard(uint8_t subcommand_data) {
	return send_data_to_ps2_keyboard(subcommand_data);
}

/*
Also used to send the data of a command to the ps2 keyboard
Used as the second part of a get function.

OS: Send command
keyboard: Send Ack
Os: Send command data/subcommand (get type/get data):
Keyboard: Send Ack, then Send: the wanted "get" value.
*/
static struct ps2_keyboard_verified_response send_getter_subcommand_to_ps2_keyboard(uint8_t subcommand_data) {

	struct ps2_keyboard_verified_response verified_response;
	struct ps2_keyboard_verified_response ret;

	verified_response = send_data_to_ps2_keyboard(subcommand_data);
	if (verified_response.err) {
		ret.response = -1;
		ret.err = verified_response.err;
		return ret;
	}

	enum ps2_controller_error_code c_err;
	c_err = wait_till_ready_for_response();
	if (c_err) {
		kprintf("The ps2_controller error: %d, |%s|\n", c_err, PS2_OS_Error_to_string(c_err));
		ret.response = -1;
		ret.err = PS2_KB_ERR_could_not_recieve_response;
		return ret;
	}

	uint8_t response = ps2_recieve_raw_response();
	enum KeyboardResponseByte response_typed = response;
	ret.response = response_typed;
	ret.err = PS2_KB_ERR_none;
	return ret;
}

/* ====================================== The applications ============================ */

enum ps2_keyboard_error_code set_leds(kcb_led_state_t led_sate) {
	struct ps2_keyboard_verified_response obtained;
	obtained = send_command_to_ps2_keyboard(KCB_setLEDs);
	if (obtained.err) {
		return obtained.err;
	}
	union kcb_led_state_uts led_sate_ts = {.value = led_sate};

	obtained = send_setter_subcommand_to_ps2_keyboard(led_sate_ts.raw);
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
		kprintf("Error sending the command to set scancode set!\n");
		return obtained.err;
	}

	obtained = send_setter_subcommand_to_ps2_keyboard((uint8_t)scan_code_set);
	if (obtained.err) {
		kprintf("Error sending the special scan code set\n!");
		return obtained.err;
	}
	return PS2_KB_ERR_none;
}

struct ps2_keyboard_verified_scan_code_set get_scan_code_set() {

	struct ps2_keyboard_verified_response obtained;
	struct ps2_keyboard_verified_scan_code_set ret;
	obtained = send_command_to_ps2_keyboard(KCB_get_set_ScanCodeSet);
	if (obtained.err) {
		ret.response = -1;
		ret.err = obtained.err;
		return ret;
	}

	obtained = send_getter_subcommand_to_ps2_keyboard(SCS_get_current);
	if (obtained.err) {
		ret.response = -1;
		ret.err = obtained.err;
		return ret;
	}

	enum ScanCodeSet scan_code_set = (enum ScanCodeSet)obtained.response;
	if (scan_code_set == SCS_get_current || scan_code_set > SCS_set_scan_code_set_3) {
		ret.response = -1;
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

enum ps2_keyboard_error_code set_typematic(kcb_typematic_rate_and_delay_t rate_and_delay) {
	struct ps2_keyboard_verified_response obtained;
	obtained = send_command_to_ps2_keyboard(KCB_set_typematic_rate_delay);
	if (obtained.err) {
		return obtained.err;
	}

	union {
		kcb_typematic_rate_and_delay_t bits;
		uint8_t raw;
	} typesafe = {.bits = rate_and_delay};
	obtained = send_setter_subcommand_to_ps2_keyboard(typesafe.raw);

	if (obtained.err) {
		return obtained.err;
	}

	return PS2_KB_ERR_none;
}

enum ps2_keyboard_error_code enable_scanning() {
	struct ps2_keyboard_verified_response obtained;
	obtained = send_command_to_ps2_keyboard(KCB_enable_scanning);
	if (obtained.err) {
		return obtained.err;
	}

	return PS2_KB_ERR_none;
}

// Keyboard wills top sending scan code
// May also restore default parameter
enum ps2_keyboard_error_code disable_scanning() {
	struct ps2_keyboard_verified_response obtained;
	obtained = send_command_to_ps2_keyboard(KCB_enable_scanning);
	if (obtained.err) {
		return obtained.err;
	}

	return PS2_KB_ERR_none;
}

enum ps2_keyboard_error_code set_default_parameters() {
	struct ps2_keyboard_verified_response obtained;
	obtained = send_command_to_ps2_keyboard(KCB_set_default_parameters);
	if (obtained.err) {
		return obtained.err;
	}

	return PS2_KB_ERR_none;
}

enum ps2_keyboard_error_code set_all_keycode_to_make_release() {
	struct ps2_keyboard_verified_response obtained;
	obtained = send_command_to_ps2_keyboard(KCB_set_all_to_make_release);
	if (obtained.err) {
		return obtained.err;
	}

	return PS2_KB_ERR_none;
}

enum ps2_keyboard_error_code set_all_keycode_to_make_only() {
#define SC3_ONLY // comment this out to deactivate
#ifdef SC3_ONLY
	assert(get_scan_code_set().response == 3, "Need to be in scancode 3\n");
#endif

	struct ps2_keyboard_verified_response obtained;
	obtained = send_command_to_ps2_keyboard(KCB_set_all_to_make_only);
	if (obtained.err) {
		return obtained.err;
	}

	return PS2_KB_ERR_none;
}

// Precondition:
// Must be in scancode 3
enum ps2_keyboard_error_code set_all_keycode_to_typematic_autorepeat_make_release() {
	struct ps2_keyboard_verified_response obtained;
	obtained = send_command_to_ps2_keyboard(KCB_set_all_to_typematic_autorepeat_make_release);
	if (obtained.err) {
		return obtained.err;
	}

	return PS2_KB_ERR_none;
}

enum ps2_keyboard_error_code set_specific_key_to_typematic_autorepeat_only(uint8_t key_scancode) {
	struct ps2_keyboard_verified_response obtained;
	obtained = send_command_to_ps2_keyboard(KCB_set_specific_to_typematic_autorepeat_only);
	if (obtained.err) {
		return obtained.err;
	}

	obtained = send_setter_subcommand_to_ps2_keyboard(key_scancode);
	if (obtained.err) {
		return obtained.err;
	}

	return PS2_KB_ERR_none;
}

enum ps2_keyboard_error_code set_specific_key_to_make_release(uint8_t key_scancode) {
	struct ps2_keyboard_verified_response obtained;
	obtained = send_command_to_ps2_keyboard(KCB_set_specific_to_make_release);
	if (obtained.err) {
		return obtained.err;
	}

	obtained = send_setter_subcommand_to_ps2_keyboard(key_scancode);
	if (obtained.err) {
		return obtained.err;
	}

	return PS2_KB_ERR_none;
}

enum ps2_keyboard_error_code set_specific_key_to_make_only(uint8_t key_scancode) {
	struct ps2_keyboard_verified_response obtained;
	obtained = send_command_to_ps2_keyboard(KCB_set_specific_to_make_only);
	if (obtained.err) {
		return obtained.err;
	}

	obtained = send_setter_subcommand_to_ps2_keyboard(key_scancode);
	if (obtained.err) {
		return obtained.err;
	}

	return PS2_KB_ERR_none;
}

struct ps2_keyboard_verified_response resend_last_byte() {
	struct ps2_keyboard_verified_response obtained;
	struct ps2_keyboard_verified_response ret;

	obtained = send_command_to_ps2_keyboard(KCB_resend_last_byte);
	if (obtained.err) {
		ret.err = obtained.err;
		ret.response = -1;
		return ret;
	}

	ret.response = obtained.response;
	ret.err = PS2_KB_ERR_none;
	return ret;
}

struct ps2_keyboard_verified_response reset_and_self_test() {
	struct ps2_keyboard_verified_response obtained;
	struct ps2_keyboard_verified_response ret;

	obtained = send_command_to_ps2_keyboard(KCB_reset_and_self_test);
	if (obtained.err) {
		ret.err = obtained.err;
		ret.response = -1;
		return ret;
	}

	enum ps2_controller_error_code c_err;
	c_err = wait_till_ready_for_response();
	if (c_err) {
		kprintf("The ps2_controller error: %d, |%s|\n", c_err, PS2_OS_Error_to_string(c_err));
		ret.response = -1;
		ret.err = PS2_KB_ERR_could_not_recieve_response;
		return ret;
	}

	uint8_t response = ps2_recieve_raw_response();
	if (response != KRB_self_test_passed) {
		ret.err = PS2_KB_ERR_self_test_failed;
		ret.response = response;
		// not -1, maybe the response and the kinda failure is useful.
		return ret;
	}

	ret.response = response;
	ret.err = PS2_KB_ERR_none;
	return ret;
}

/* ================================ Step function ========================= */

void disable_keyboard() {
	kprintf("Disabling keyboard\n");
	IRQ_set_mask(_single_keyboard_irq);
	disable_keyboard_interrupts(); // This seems to not be the way to do it
	disable_scanning();
}

void enable_keyboard() {
	kprintf("Enabling keyboard\n");
	enable_keyboard_interrupts();
	IRQ_clear_mask(_single_keyboard_irq);
	enable_scanning();
}
