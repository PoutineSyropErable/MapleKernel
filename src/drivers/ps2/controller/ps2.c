#include "acpi.h"
#include "assert.h"
#include "ps2.h"
#include "stdio.h"
#include "usb_controller.h"

#define MAX_PS2_WAIT_LOOP 1000

/* =================================== Global Variables ================================ */
// This is more of a header defined thing, but it shouldn't be global. has 0
//  what allignement to better use for caching?
__attribute__((aligned(64))) const enum PS2_ResponseType command_to_response_type[256] = {
    [0 ... 0xFF] = PS2_RT_not_a_command, // default all to none

    // Known ranges, ignore the warnings?
    [PS2_CB_read_byte_0] = PS2_RT_controller_configuration_byte,
    [PS2_CB_read_byte_N_start... PS2_CB_read_byte_N_end] = PS2_RT_unknown,

    [PS2_CB_write_next_to_byte_0... PS2_CB_write_next_to_byte_N_end] = PS2_RT_none,

    [PS2_CB_disable_second_ps2_port] = PS2_RT_none,
    [PS2_CB_enable_second_ps2_port] = PS2_RT_none,
    [PS2_CB_test_second_ps2_port] = PS2_RT_test_port,
    [PS2_CB_test_ps2_controller] = PS2_RT_test_controller,
    [PS2_CB_test_first_ps2_port] = PS2_RT_test_port,

    [PS2_CB_diagnostic_dump] = PS2_RT_unknown,

    [PS2_CB_disable_first_ps2_port] = PS2_RT_none,
    [PS2_CB_enable_first_ps2_port] = PS2_RT_none,

    [PS2_CB_read_controller_input_port] = PS2_RT_unknown,
    [PS2_CB_copy_low_input_to_status] = PS2_RT_none,
    [PS2_CB_copy_high_input_to_status] = PS2_RT_none,

    [PS2_CB_read_controller_output_port] = PS2_RT_controller_output_port,

    [PS2_CB_write_next_byte_to_controller_output_port] = PS2_RT_none,
    [PS2_CB_write_next_byte_to_first_ps2_output_buffer] = PS2_RT_none,
    [PS2_CB_write_next_byte_to_second_ps2_output_buffer] = PS2_RT_none,
    [PS2_CB_write_next_byte_to_second_ps2_input_buffer] = PS2_RT_none,

    [PS2_CB_pulse_output_line_low_start... PS2_CB_pulse_output_line_low_end] = PS2_RT_none,

};
/* =================================== Static Inline Functions ================================ */

// Input that it recieves, aka that we SEND from the OS.
static inline bool is_ps2_controller_ready_for_more_input(PS2_StatusRegister_t status) {
	// must be clear, input buffer must be empty before os writes to it.
	return !status.input_buffer_full_not_empty;
}

static inline bool is_ps2_controller_ready_for_response(PS2_StatusRegister_t status) {
	// must be set before continuing. Output buffer must be full before writting to os.
	return status.output_buffer_full_not_empty;
}

static inline enum ps2_controller_error_code PS2_verify_configuration_byte_response(ps2_configuration_byte_uts_t configuration_byte) {
	// TODO: Check if there is any impossible responses?
	PS2_ConfigurationByte_t bits = configuration_byte.bits;
	if (bits.zero1 != 0 || bits.zero1 != 0) {
		return PS2_ERR_invalid_configuration_byte;
	} else if (bits.system_flag_passed_post_one == 0) {
		// Maybe the system flag can be 0 at the absolute start.
		// Then, the software must set it to one once?
		// If such an error happen, then debug it, and use a static state.
		return PS2_WARN_invalid_configuration_byte_post_is_zero;
	}
	return PS2_ERR_none;
}

/* ========================================== Other Helpers that should be in the header file, but not static =========================== */
enum ps2_device_super_type get_device_super_type(enum ps2_device_type dt) {
	switch (dt) {

	// --- KEYBOARDS ---
	case PS2_DT_ancient_at_keyboard:
	case PS2_DT_mf2_keyboard_1:
	case PS2_DT_mf2_keyboard_2:
	case PS2_DT_short_keyboard:
	case PS2_DT_122_key_host_connected:
	case PS2_DT_122_key:
	case PS2_DT_japanese_g_keyboard:
	case PS2_DT_japanese_p_keyboard:
	case PS2_DT_japanese_a_keyboard:
	case PS2_DT_ncd_sun_layout_keyboard:
		return PS2_DST_keyboard;

	// --- MICE ---
	case PS2_DT_standard_mouse:
	case PS2_DT_mouse_with_scroll_wheel:
	case PS2_DT_mouse_with_5_button:
		return PS2_DST_mouse;

	default:
		// Unknown device → decide how you want to handle it
		return PS2_DST_unknown;
	}
}

/* =================================== Internals Functions ================================ */

/* For the PS2 Device to be ready for more inputs. So, wait for the ps2 controller to be ready for the OS to send the next output */
enum ps2_controller_error_code wait_till_ready_for_more_input() {

	for (uint32_t i = 0; i < MAX_PS2_WAIT_LOOP; i++) {
		io_wait();
		PS2_StatusRegister_t status = read_ps2_status();
		bool is_ready = is_ps2_controller_ready_for_more_input(status);
		if (is_ready) {
			if (!PS2_verify_status_parity(status)) {
				return PS2_ERR_status_parity;
			}
			if (!PS2_verify_status_timeout(status)) {
				return PS2_ERR_status_timeout;
			}
			return PS2_ERR_none;
		}
	}
	return PS2_ERR_wait_max_itt_in;
}

/* For the PS2 Device, who is currently calculating the next output to be done with the result ready. So for the OS to read the response */
enum ps2_controller_error_code wait_till_ready_for_response() {

	for (uint32_t i = 0; i < MAX_PS2_WAIT_LOOP; i++) {
		io_wait();
		PS2_StatusRegister_t status = read_ps2_status();
		bool is_ready = is_ps2_controller_ready_for_response(status);
		if (is_ready) {
			return PS2_ERR_none;
		}
	}
	return PS2_ERR_wait_max_itt_out;
}

static inline enum ps2_controller_error_code send_command_to_ps2_controller(enum PS2_CommandByte command) {
	enum ps2_controller_error_code err = wait_till_ready_for_more_input();
	if (err) {
		return err;
	}

	__outb(PS2_COMMAND_PORT_W, (uint8_t)command);
	return PS2_ERR_none;
}

/* ================================ Generic Function =========================================== */
// These shouldn't be used. They are present because it's good practice

// This function assume PS2 Controller is ready
[[gnu::unused, gnu::deprecated("It's too powerful, and both low and high level. Don't use this. Use the specific versions for each commands")]]
struct PS2_Tagged_Reponse recieve_generic_response(enum PS2_CommandByte command) {
	enum PS2_ResponseType response_type = command_to_response_type[command];
	assert(response_type != PS2_RT_not_a_command, "Asking a response to an Invalid Command!");
	assert(response_type != PS2_RT_none, "Invalid a response to a command without it");

	uint8_t raw_response = ps2_recieve_raw_response();
	struct PS2_Tagged_Reponse ret;
	ret.type = response_type;

	// This whole switch is technically useless. It should be compiled away!
	switch (response_type) {
	case PS2_RT_test_port:
		ret.value.g_test_port_response = raw_response;
		return ret;
	case PS2_RT_test_controller:
		ret.value.g_test_controller_response = raw_response;
		return ret;
	case PS2_RT_controller_configuration_byte:
		ret.value.g_configuration_byte.raw = raw_response;
		return ret;
	case PS2_RT_controller_output_port:
		ret.value.g_controller_output_port.raw = raw_response;
		return ret;
	case PS2_RT_unknown:
		ret.value.___unknown = raw_response;
	default:
		kprintf("The assert failed!, This code should never be executed at all!\n");
		kprintf("It means my assert function is broken!\n");
		_abort();
	}
}

// This function assume PS2 Controller is ready
[[gnu::unused, gnu::deprecated("It's too powerful, and both low and high level. Don't use this. Use the specific versions for each commands")]]
struct ___ps2_typeless_return recieve_generic_verified_response(enum PS2_ResponseType response_type) {
	assert(response_type != PS2_RT_not_a_command, "Asking a response to an Invalid Command!");
	assert(response_type != PS2_RT_none, "Invalid a response to a command without it");
	struct ___ps2_typeless_return ret;

	uint8_t raw_response = ps2_recieve_raw_response();
	struct PS2_Tagged_Reponse tagged_response;
	tagged_response.type = response_type;

	bool valid;
	// It's a lot of code, but it's not doing much, since it's just doing repeated implicit casting
	// But, I prefer to fake a stronger type system
	switch (response_type) {
	case PS2_RT_test_port:
		enum PS2_TestPortResponse response_tp = raw_response;
		tagged_response.value.g_test_port_response = response_tp;
		valid = PS2_verify_test_port_response(response_tp);
		if (!valid) {
			ret.tagged_response = tagged_response;
			ret.err = PS2_ERR_invalid_test_port_response;
			return ret;
		}
		ret.tagged_response = tagged_response;
		ret.err = PS2_ERR_none;
		return ret;

	case PS2_RT_test_controller:
		enum PS2_TestControllerResponse response_tc = raw_response;
		tagged_response.value.g_test_controller_response = response_tc;
		valid = PS2_verify_test_controller_response(response_tc);
		if (!valid) {
			ret.tagged_response = tagged_response;
			ret.err = PS2_ERR_invalid_test_controller_response;
			return ret;
		}
		ret.tagged_response = tagged_response;
		ret.err = PS2_ERR_none;
		return ret;

	case PS2_RT_controller_configuration_byte:
		union ps2_configuration_byte_uts response_cb = {.raw = raw_response};
		tagged_response.value.g_configuration_byte = response_cb;
		enum ps2_controller_error_code valid_err = PS2_verify_configuration_byte_response(response_cb);
		if (valid_err) {
			ret.tagged_response = tagged_response;
			ret.err = valid_err;
			return ret;
		}
		ret.tagged_response = tagged_response;
		ret.err = PS2_ERR_none;
		return ret;

	case PS2_RT_controller_output_port:
		union ps2_controller_output_port_uts response_op = {.raw = raw_response};
		tagged_response.value.g_controller_output_port = response_op;
		valid = PS2_verify_controller_output_port_response(response_op.bits);
		if (!valid) {
			ret.tagged_response = tagged_response;
			ret.err = PS2_WARN_A20_line_not_set;
			return ret;
		}
		ret.tagged_response = tagged_response;
		ret.err = PS2_ERR_none;
		return ret;

	case PS2_RT_unknown:
		tagged_response.value.___unknown = raw_response;
		ret.tagged_response = tagged_response;
		ret.err = PS2_ERR_none;
		return ret;
	default:

		kprintf("The assert failed!, This code should never be executed at all!\n");
		kprintf("It means my assert function is broken!\n");
		_abort();
	}
}

[[gnu::unused, gnu::deprecated("It's too powerful, and both low and high level. Don't use this. Use the specific versions for each commands")]]
struct ___ps2_typeless_return send_command_and_recieve_response(enum PS2_CommandByte command) {
	struct ___ps2_typeless_return ret;
	enum PS2_ResponseType response_type = command_to_response_type[command];
	assert(response_type != PS2_RT_not_a_command, "Sending an invalid command");
	ret.tagged_response.type = response_type;

	enum ps2_controller_error_code err = send_command_to_ps2_controller(command);
	if (err) {
		ret.err = err;
		return ret;
	}

	if (response_type == PS2_RT_none) {
		ret.err = PS2_ERR_none;
		return ret;
	}

	err = wait_till_ready_for_response();
	if (err) {
		ret.err = err;
		return ret;
	}

	// Obtained will have the correct response type.
	struct ___ps2_typeless_return obtained = recieve_generic_verified_response(response_type);
	return obtained;
}

/* ================================= Specific Functions ============================================= */
// Here, I will implement the functions to recieve a specific type of response. These functions will need their own unique output types of (err, value)
// These are really concrete wrapper, used as atomic functions.

// None response
static inline enum ps2_controller_error_code
send_command_none_response(enum PS2_CommandByte command) {
	return send_command_to_ps2_controller(command);
}

// Unknown response
ps2_verified_response_unknown_t send_command_unknown_response(enum PS2_CommandByte command) {
	ps2_verified_response_unknown_t ret;

	enum ps2_controller_error_code err = send_command_to_ps2_controller(command);
	if (err) {
		ret.err = err;
		return ret;
	}

	err = wait_till_ready_for_response();
	if (err) {
		ret.err = err;
		return ret;
	}

	uint8_t raw_response = ps2_recieve_raw_response();
	ret.response = raw_response;
	return ret;
}

// Configuration Byte response
static inline ps2_verified_response_configuration_byte_t
send_command_configuration_byte_response(enum PS2_CommandByte command) {
	ps2_verified_response_configuration_byte_t ret;

	enum ps2_controller_error_code err = send_command_to_ps2_controller(command);
	if (err) {
		ret.err = err;
		return ret;
	}

	err = wait_till_ready_for_response();
	if (err) {
		ret.err = err;
		return ret;
	}

	uint8_t raw_response = ps2_recieve_raw_response();
	ret.response.raw = raw_response;
	enum ps2_controller_error_code valid_err = PS2_verify_configuration_byte_response(ret.response);
	if (valid_err) {
		ret.err = valid_err;
		return ret;
	}

	ret.err = PS2_ERR_none;
	return ret;
}

// Test Port response
ps2_verified_response_test_port_t send_command_test_port_response(enum PS2_CommandByte command) {
	ps2_verified_response_test_port_t ret;

	enum ps2_controller_error_code err = send_command_to_ps2_controller(command);
	if (err) {
		ret.err = err;
		return ret;
	}

	err = wait_till_ready_for_response();
	if (err) {
		ret.err = err;
		return ret;
	}

	uint8_t raw_response = ps2_recieve_raw_response();
	ret.response = raw_response;
	bool valid = PS2_verify_test_port_response(ret.response);
	if (!valid) {
		ret.err = PS2_ERR_invalid_test_port_response;
		return ret;
	}

	ret.err = PS2_ERR_none;
	return ret;
}

// Test controller Response
static inline ps2_verified_response_test_controller_t
send_command_test_controller_response(enum PS2_CommandByte command) {
	ps2_verified_response_test_controller_t ret;

	enum ps2_controller_error_code err = send_command_to_ps2_controller(command);
	if (err) {
		ret.err = err;
		return ret;
	}

	err = wait_till_ready_for_response();
	if (err) {
		ret.err = err;
		return ret;
	}

	uint8_t raw_response = ps2_recieve_raw_response();
	ret.response = raw_response;
	bool valid = PS2_verify_test_controller_response(ret.response);
	if (!valid) {
		ret.err = PS2_ERR_invalid_test_controller_response;
		return ret;
	}

	ret.err = PS2_ERR_none;
	return ret;
}

// Test controller Output Port Response
static inline ps2_verified_response_controller_output_port_t
send_command_test_controller_output_port_response(enum PS2_CommandByte command) {
	ps2_verified_response_controller_output_port_t ret;

	enum ps2_controller_error_code err = send_command_to_ps2_controller(command);
	if (err) {
		ret.err = err;
		return ret;
	}

	err = wait_till_ready_for_response();
	if (err) {
		ret.err = err;
		return ret;
	}

	uint8_t raw_response = ps2_recieve_raw_response();
	ret.response.raw = raw_response;
	bool valid = PS2_verify_controller_output_port_response(ret.response.bits);
	if (!valid) {
		ret.err = PS2_WARN_A20_line_not_set;
		return ret;
	}

	ret.err = PS2_ERR_none;
	return ret;
}

/* ========================================= steps functions =================================== */
// These are functions that represent concrete steps. They might actually be better put in another file, I do not know.

static inline ps2_verified_response_configuration_byte_t ps2_read_byte_0_from_internal_ram() {
	return send_command_configuration_byte_response(PS2_CB_read_byte_0);
}

static inline ps2_verified_response_unknown_t ps2_read_byte_n_from_internal_ram(uint8_t byte_index) {
	// should I allow reading byte 0? Let's allow it, but return an error.
	// But this may return an error, depending on hardware?
	assert(byte_index < (PS2_CB_read_byte_N_end - PS2_CB_read_byte_0), "must be in allowed byte range!\n");
	assert(byte_index != 0, "0 byte might cause errors?");
	// no, if byte index is 0, it will just send the above command

	ps2_verified_response_unknown_t ret;
	enum PS2_CommandByte command_byte = PS2_CB_read_byte_0 + byte_index;
	ret = send_command_unknown_response(command_byte);

	if (byte_index == 0 && ret.err == PS2_ERR_none) {
		// Not a dangerous error. Maybe create a warning type?
		// Error are low values. Warning are high values going down?
		ret.err = PS2_WARN_n_is_zero;
	}
	return ret;
}

ps2_verified_response_configuration_byte_t ps2_get_configuration_byte() {
	return send_command_configuration_byte_response(PS2_CB_read_byte_0);
}

//=======
static inline enum ps2_controller_error_code write_next_to_byte_0_of_interal_ram() {
	return send_command_none_response(PS2_CB_write_next_to_byte_0);
}

[[maybe_unused]] static inline enum ps2_controller_error_code
write_next_to_byte_n_of_interal_ram(uint8_t byte_index) {
	assert(byte_index < (PS2_CB_write_next_to_byte_N_end - PS2_CB_write_next_to_byte_0), "must be in allowed byte range!\n");

	enum ps2_controller_error_code err;
	enum PS2_CommandByte command_byte = PS2_CB_write_next_to_byte_0 + byte_index;
	err = send_command_none_response(command_byte);

	if (byte_index == 0 && err == PS2_ERR_none) {
		// Not a dangerous error. Maybe create a warning type?
		// Error are low values. Warning are high values going down?
		err = PS2_WARN_n_is_zero;
	}
	return err;
}

// Should the public api allow these functions? If so, maybe use verifed response none, rather then error code?
enum ps2_controller_error_code ps2_set_configuration_byte(PS2_ConfigurationByte_t config_byte) {
	union ps2_configuration_byte_uts config_byte_uts = {.bits = config_byte};

	enum ps2_controller_error_code err = write_next_to_byte_0_of_interal_ram();
	if (err) {
		return err;
	}
	err = wait_till_ready_for_more_input();
	if (err) {
		return err;
	}
	__outb(PS2_DATA_PORT_RW, config_byte_uts.raw);
	err = PS2_ERR_none;
	return err;
}
//==========

enum ps2_controller_error_code ps2_disable_first_ps2_port() {
	return send_command_to_ps2_controller(PS2_CB_disable_first_ps2_port);
}

/* Only if 2 PS/2 port are supported. Do not use this function for the test. Use the configuration byte
Set a configuration byte with it enabled, and read it back. check if it's enabled.
*/
enum ps2_controller_error_code ps2_disable_second_ps2_port() {
	return send_command_to_ps2_controller(PS2_CB_disable_second_ps2_port);
}

enum ps2_controller_error_code ps2_enable_first_ps2_port() {
	return send_command_to_ps2_controller(PS2_CB_enable_first_ps2_port);
}

/* Only if 2 PS/2 port are supported. Do not use this function for the test. Use the configuration byte
Set a configuration byte with it enabled, and read it back. check if it's enabled.
*/
enum ps2_controller_error_code ps2_enable_second_ps2_port() {
	return send_command_to_ps2_controller(PS2_CB_enable_second_ps2_port);
}
// ===============

struct ps2_verified_response_test_port ps2_test_first_ps2_port() {
	return send_command_test_port_response(PS2_CB_test_first_ps2_port);
}

/* Only if 2 PS/2 port are supported. Do not use this function for the test. Use the configuration byte
Set a configuration byte with it enabled, and read it back. check if it's enabled.
*/
struct ps2_verified_response_test_port ps2_test_second_ps2_port() {
	return send_command_test_port_response(PS2_CB_test_second_ps2_port);
}
// ===============
struct ps2_verified_response_test_controller ps2_perform_controller_self_test() {
	return send_command_test_controller_response(PS2_CB_test_ps2_controller);
}

// ===============
// diagnostic dump, idk how to implement it. A loop that reads?

// ===============

// This function is only used by manifucaturer to check if the things work.
// It shouldn't even be implemented or used
enum ps2_controller_error_code ps2_copy_input_port_to_status() {
	enum ps2_controller_error_code err;
	err = send_command_none_response(PS2_CB_copy_high_input_to_status);
	if (err) {
		kprintf("Failed during high copy\n");
		return err;
	}
	err = send_command_none_response(PS2_CB_copy_low_input_to_status);
	if (err) {
		kprintf("Failed during low copy\n");
		return err;
	}
	return PS2_ERR_none;
}
// ===============

struct ps2_verified_response_unknown ps2_read_controller_input_port() {
	return send_command_unknown_response(PS2_CB_read_controller_input_port);
}
struct ps2_verified_response_controller_output_port ps2_read_controller_output_port() {
	return send_command_test_controller_output_port_response(PS2_CB_read_controller_output_port);
}
// ===============
static inline enum ps2_controller_error_code
ps2_write_next_byte_to_controller_output_port() {
	return send_command_none_response(PS2_CB_write_next_byte_to_controller_output_port);
}

enum ps2_controller_error_code ps2_set_controller_output_port(PS2_ControllerOutputPort_t output_port) {
	union ps2_controller_output_port_uts output_port_uts = {.bits = output_port};
	enum ps2_controller_error_code err;
	err = ps2_write_next_byte_to_controller_output_port();
	if (err) {
		return err;
	}

	// Special case here, since we write to the controller output port.
	// We need to check if we can write to output port.
	// The function name here is just, wait till output buffer is empty.
	// But since it's the only case where it's not for a response, I won't change the function name to the more general case
	err = wait_till_ready_for_response();
	if (err) {
		return err;
	}

	__outb(PS2_DATA_PORT_RW, output_port_uts.raw);
	return PS2_ERR_none;
}

// ===============
// Fake ps2 keyboard and mouse (port 1 and port 2) outputs.
enum ps2_controller_error_code write_next_byte_to_first_ps2_output_port() {
	return send_command_none_response(PS2_CB_write_next_byte_to_first_ps2_output_buffer);
}

enum ps2_controller_error_code write_next_byte_to_second_ps2_output_port() {
	return send_command_none_response(PS2_CB_write_next_byte_to_second_ps2_output_buffer);
}
enum ps2_controller_error_code fake_ps2_keyboard_byte(uint8_t byte) {
	enum ps2_controller_error_code err;
	err = write_next_byte_to_first_ps2_output_port();
	if (err) {
		return err;
	}

	__outb(PS2_DATA_PORT_RW, byte);
	return PS2_ERR_none;
}
enum ps2_controller_error_code fake_ps2_mouse_byte(uint8_t byte) {
	enum ps2_controller_error_code err;
	err = write_next_byte_to_second_ps2_output_port();
	if (err) {
		return err;
	}

	__outb(PS2_DATA_PORT_RW, byte);
	return PS2_ERR_none;
}

// ===============
enum ps2_controller_error_code send_command_to_first_ps2_port(uint8_t data) {

	enum ps2_controller_error_code err = wait_till_ready_for_more_input();
	if (err) {
		return err;
	}

	__outb(PS2_DATA_PORT_RW, data);

	return PS2_ERR_none;
}

enum ps2_controller_error_code send_command_to_second_ps2_port(uint8_t data) {

	enum ps2_controller_error_code err = wait_till_ready_for_more_input();
	if (err) {
		return err;
	}
	__outb(PS2_COMMAND_PORT_W, PS2_CB_write_next_byte_to_second_ps2_input_buffer);

	err = wait_till_ready_for_more_input();
	if (err) {
		return err;
	}
	__outb(PS2_DATA_PORT_RW, data);

	return PS2_ERR_none;
}

// This is used to send data to the first or second ps2 device. Port 1 = Keyboard, Port 2 = mouse
enum ps2_controller_error_code send_command_or_data_to_ps2_port(enum PS2_PortNumber port_number, uint8_t data) {

	switch (port_number) {
	case 1:
		return send_command_to_first_ps2_port(data);
	case 2:
		return send_command_to_second_ps2_port(data);
	default:
		return PS2_ERR_invalid_port_number;
	}
}

// ========================

struct ps2_device_type_verified reset_port_and_get_device_type(enum PS2_PortNumber portnumber) {
	struct ps2_device_type_verified ret;
	enum ps2_controller_error_code err;

	err = send_command_or_data_to_ps2_port(portnumber, PS2_CB_reset_device);
	if (err) {
		ret.err = err;
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

// ====================== Prints for the structs ==================
void print_ps2_configuration_byte(PS2_ConfigurationByte_t config) {
	kprintf("first_ps2_port_enabled = %b\n", config.first_ps2_port_enabled);
	kprintf("second_ps2_port_enabled = %b\n", config.second_ps2_port_enabled);
	kprintf("system_flag_passed_post_one = %b\n", config.system_flag_passed_post_one);
	kprintf("zero1 = %b\n", config.zero1);
	kprintf("first_ps2_port_clock_disabled = %b\n", config.first_ps2_port_clock_disabled);
	kprintf("second_ps2_port_clock_disabled = %b\n", config.second_ps2_port_clock_disabled);
	kprintf("first_ps2_port_translation_enabled = %b\n", config.first_ps2_port_translation_enabled);
	kprintf("zero2 = %b\n", config.zero2);
}

// ================================================= The main important ones, Like setting it up, and enabling mouse ================

struct ps2_initialize_device_state setup_ps2_controller() {

	struct ps2_initialize_device_state ret;

	// Step 1:
	enum usb_cont_error_code usb_ret;
	usb_ret = initialize_usb_controllers();
	if (usb_ret) {
		ret.ps2_state_err = PS2_ID_ERR_usb_error;
		return ret;
	}
	usb_ret = disable_legacy_usb_support();
	if (usb_ret) {
		ret.ps2_state_err = PS2_ID_ERR_usb_error;
		return ret;
	}

	// Step 2:
	bool exist = does_ps2_controller_exist();
	if (!exist) {
		ret.ps2_state_err = PS2_ID_ERR_ps2_controller_does_not_exist;
		return ret;
	}

	// Step 3:
	enum ps2_controller_error_code err;
	err = ps2_disable_first_ps2_port();
	if (err) {
		kprintf("\n[PANIC] Error in Step 3 of initializing the PS2 Controller. Could not disable first ps2 port.\n");
		kprintf("The error value: %u\n", err);
		kprintf("The error: %s\n", PS2_OS_Error_to_string(err));
		ret.ps2_state_err = PS2_ID_ERR_could_not_init;
		ret.internal_err = err;
		return ret;
	}
	err = ps2_disable_second_ps2_port();
	if (err) {
		kprintf("\n[PANIC] Error in Step 3 of initializing the PS2 Controller. Could not disable second ps2 port.\n");
		kprintf("The error value: %u\n", err);
		kprintf("The error: %s\n", PS2_OS_Error_to_string(err));
		ret.ps2_state_err = PS2_ID_ERR_could_not_init;
		ret.internal_err = err;
		return ret;
	}

	// Step 4: Flush the Output Buffer (by reading from the data port == recieving a response)
	[[gnu::unused]] uint8_t discard = __inb(PS2_DATA_PORT_RW); // Also recieve_raw_response. Idk which i prefer here.

	// Step 5: Set the configuration Byte
	ps2_verified_response_configuration_byte_t vr_cb = ps2_get_configuration_byte();
	if (vr_cb.err) {
		if (vr_cb.err == PS2_WARN_invalid_configuration_byte_post_is_zero) {

#ifndef QEMU // assuming this is a qemu bug, then if not in qemu, we must treat it accordingly
			kprintf("\n[PANIC] Error in Step 5 of initializing the PS2 Controller. Could not get the configuration byte - failed to post?\n");
			kprintf("The error value: %u\n", vr_cb.err);
			kprintf("The error: %s\n", PS2_OS_Error_to_string(vr_cb.err));
			ret.ps2_state_err = PS2_ID_ERR_could_not_init;
			ret.internal_err = vr_cb.err;
			return ret;
#endif
			kprintf("It seems a qemu bug happned where the configuration byte POST bit hasn't been set\n");
			kprintf("Maybe this is a catastrophic state on a regular machine\n");
		} else {
			kprintf("\n[PANIC] Error in Step 5 of initializing the PS2 Controller. Could not get the configuration byte\n");
			kprintf("The error value: %u\n", vr_cb.err);
			kprintf("The error: %s\n", PS2_OS_Error_to_string(vr_cb.err));
			ret.ps2_state_err = PS2_ID_ERR_could_not_init;
			ret.internal_err = vr_cb.err;
			return ret;
		}
	}

	PS2_ConfigurationByte_t config_byte = vr_cb.response.bits;
	kprintf("\n\n");
	print_ps2_configuration_byte(config_byte);
	kprintf("\n\n");

#ifdef QEMU
	config_byte.system_flag_passed_post_one = true; // qemu bug
#endif
	config_byte.first_ps2_port_enabled = false;             // disable first
	config_byte.first_ps2_port_translation_enabled = false; // disable first irq, this also set the keyboard to scanset 2. So, we must set it back with the keyboard driver
	config_byte.first_ps2_port_clock_disabled = false;      // enable clock signal

	err = ps2_set_configuration_byte(config_byte);
	if (err) {
		kprintf("\n[PANIC] Error in Step 5 of initializing the PS2 Controller. Could not set the configuration byte\n");
		kprintf("The error value: %u\n", err);
		kprintf("The error: %s\n", PS2_OS_Error_to_string(err));
		ret.ps2_state_err = PS2_ID_ERR_could_not_init;
		ret.internal_err = err;
		return ret;
	}

	// Step 6: Perform controller self test
	struct ps2_verified_response_test_controller tc_vr = ps2_perform_controller_self_test();
	if (tc_vr.err) {
		kprintf("\n[PANIC] Error in Step 6 of initializing the PS2 Controller. The PS2 Controler could not perform self test\n");
		kprintf("The error value: %u\n", tc_vr.err);
		kprintf("The error: %s\n", PS2_OS_Error_to_string(tc_vr.err));
		ret.ps2_state_err = PS2_ID_ERR_could_not_init;
		ret.internal_err = tc_vr.err;
		return ret;
	}
	if (tc_vr.response != PS2_TCR_passed) {
		kprintf("\n[PANIC] Error in Step 6 of initializing the PS2 Controller. The PS2 Controler self test didn't pass\n");

		ret.ps2_state_err = PS2_ID_ERR_controller_self_test_failed;
		return ret;
	}
	// Step 7: Determine if there are two channels.
	err = ps2_enable_second_ps2_port();
	if (err) {
		kprintf("\n[PANIC] Error in Step 7 of initializing the PS2 Controller. Could not enable second ps2 port\n");
		kprintf("The error value: %u\n", err);
		kprintf("The error: %s\n", PS2_OS_Error_to_string(err));
		ret.ps2_state_err = PS2_ID_ERR_could_not_init;
		ret.internal_err = err;
		return ret;
	}

	vr_cb = ps2_get_configuration_byte();
	config_byte = vr_cb.response.bits;
	if (vr_cb.err) {
		kprintf("\n[PANIC] Error in Step 7 of initializing the PS2 Controller. Could not get configuration byte\n");
		kprintf("\nThe error value: %u\n", vr_cb.err);
		kprintf("The error: %s\n", PS2_OS_Error_to_string(vr_cb.err));
		ret.ps2_state_err = PS2_ID_ERR_could_not_init;
		ret.internal_err = vr_cb.err;
		return ret;
	}
	print_ps2_configuration_byte(config_byte);

	// OS Dev say to check bit 5, not bit 1
	bool second_port_supported = !config_byte.second_ps2_port_clock_disabled;

	err = ps2_disable_second_ps2_port();
	if (err) {
		kprintf("\n[PANIC] Error in Step 7 of initializing the PS2 Controller. Could not disable second ps2 port\n");
		kprintf("\nError disabling the second ps2_port");
		kprintf("The error value: %u\n", err);
		kprintf("The error: %s\n", PS2_OS_Error_to_string(err));
		ret.ps2_state_err = PS2_ID_ERR_could_not_init;
		ret.internal_err = err;
		return ret;
	}

	if (second_port_supported) {
		vr_cb = ps2_get_configuration_byte();
		if (vr_cb.err) {
			kprintf("\n[PANIC] Error in Step 7 of initializing the PS2 Controller. Could not get configuration byte for cleanup\n");
			kprintf("\nThe error value: %u\n", vr_cb.err);
			kprintf("The error: %s\n", PS2_OS_Error_to_string(vr_cb.err));
			ret.ps2_state_err = PS2_ID_ERR_could_not_init;
			ret.internal_err = vr_cb.err;
			return ret;
		}

		config_byte = vr_cb.response.bits;
		config_byte.second_ps2_port_clock_disabled = false;
		config_byte.second_ps2_port_enabled = false;

		err = ps2_set_configuration_byte(config_byte);
		if (err) {
			kprintf("\n[PANIC] Error in Step 7 of initializing the PS2 Controller. Could not set configuration byte for cleanup\n");
			kprintf("\nError disabling the second ps2_port");
			kprintf("The error value: %u\n", err);
			kprintf("The error: %s\n", PS2_OS_Error_to_string(err));
			ret.ps2_state_err = PS2_ID_ERR_could_not_init;
			ret.internal_err = err;
			return ret;
		}
	}

	// Step 8: Perform interface test
	struct ps2_verified_response_test_port test_port_vr;

	test_port_vr = ps2_test_first_ps2_port();
	if (test_port_vr.err) {
		kprintf("\n[PANIC] Error in Step 8 of initializing the PS2 Controller. First port errored during self test\n");
		kprintf("The error value: %u\n", test_port_vr.err);
		kprintf("The error: %s\n", PS2_OS_Error_to_string(test_port_vr.err));
		ret.ps2_state_err = PS2_ID_ERR_could_not_init;
		ret.internal_err = test_port_vr.err;
		return ret;
	}
	if (test_port_vr.response != PS2_TPR_passed) {
		kprintf("\n[PANIC] Error in Step 8 of initializing the PS2 Controller. First port couldn't pass test\n");
		kprintf("The reponse value: %u\n", test_port_vr.response);
		kprintf("The response name: %s\n", PS2_TestPortResponse_to_string(test_port_vr.response));
		ret.ps2_state_err = PS2_ID_ERR_first_port_self_test_failed;
		return ret;
	}

	test_port_vr = ps2_test_second_ps2_port();
	if (test_port_vr.err) {
		kprintf("\n[PANIC] Error in Step 8 of initializing the PS2 Controller. Second port errored during self test\n");
		kprintf("The error value: %u\n", test_port_vr.err);
		kprintf("The error: %s\n", PS2_OS_Error_to_string(test_port_vr.err));
		ret.ps2_state_err = PS2_ID_ERR_could_not_init;
		ret.internal_err = test_port_vr.err;
		return ret;
	}
	if (test_port_vr.response != PS2_TPR_passed) {
		kprintf("\n[PANIC] Error in Step 8 of initializing the PS2 Controller. Second port couldn't pass test\n");
		kprintf("The reponse value: %u\n", test_port_vr.response);
		kprintf("The response name: %s\n", PS2_TestPortResponse_to_string(test_port_vr.response));
		ret.ps2_state_err = PS2_ID_ERR_second_port_self_test_failed;
		return ret;
	}

	// Step 9: Enable devices:
	err = ps2_enable_first_ps2_port();
	if (err) {
		kprintf("\n[PANIC] Error in Step 9 of initializing the PS2 Controller. Could not enable first ps2 port\n");
		kprintf("The error value: %u\n", err);
		kprintf("The error: %s\n", PS2_OS_Error_to_string(err));
		ret.ps2_state_err = PS2_ID_ERR_could_not_init;
		ret.internal_err = err;
		return ret;
	}

	err = ps2_enable_second_ps2_port();
	if (err) {
		kprintf("\n[PANIC] Error in Step 9 of initializing the PS2 Controller. Could not enable second ps2 port\n");
		kprintf("The error value: %u\n", err);
		kprintf("The error: %s\n", PS2_OS_Error_to_string(err));
		ret.ps2_state_err = PS2_ID_ERR_could_not_init;
		ret.internal_err = err;
		return ret;
	}

	vr_cb = ps2_get_configuration_byte();
	if (vr_cb.err) {
		kprintf("\n[PANIC] Error in Step 9 of initializing the PS2 Controller. Could not get configuration byte\n");
		kprintf("\nThe error value: %u\n", vr_cb.err);
		kprintf("The error: %s\n", PS2_OS_Error_to_string(vr_cb.err));
		ret.ps2_state_err = PS2_ID_ERR_could_not_init;
		ret.internal_err = vr_cb.err;
		return ret;
	}
	config_byte = vr_cb.response.bits;
	config_byte.first_ps2_port_enabled = true;
	config_byte.second_ps2_port_enabled = true;

	err = ps2_set_configuration_byte(config_byte);
	if (err) {
		kprintf("\n[PANIC] Error in Step 9 of initializing the PS2 Controller. Could not set configuration byte\n");
		kprintf("The error value: %u\n", err);
		kprintf("The error: %s\n", PS2_OS_Error_to_string(err));
		ret.ps2_state_err = PS2_ID_ERR_could_not_init;
		ret.internal_err = err;
		return ret;
	}

	// Step 10: Reset devices:
	struct ps2_device_type_verified device_1 = reset_port_and_get_device_type(PS2_PN_port_one);
	if (device_1.err) {
		kprintf("\n[PANIC] Error in Step 10 of initializing the PS2 Controller. Could not reset port 1\n");
		kprintf("The error value: %u\n", err);
		kprintf("The error: %s\n", PS2_OS_Error_to_string(device_1.err));
		ret.ps2_state_err = PS2_ID_ERR_could_not_reset_device1;
		ret.internal_err = device_1.err;
		return ret;
	}

	uint8_t keyboard_count = 0;
	uint8_t mouse_count = 0;
	switch (device_1.mouse_or_keyboard) {
	case PS2_DST_keyboard:
		keyboard_count++;
		ret.port_one_device_type = device_1.type;
		break;
	case PS2_DST_mouse:
		mouse_count++;
		ret.port_one_device_type = device_1.type;
		break;
	default:
		abort_msg("Device 1: Impossible device super type!\n");
	}

	if (!second_port_supported) {
		ret.ps2_state_err = PS2_ID_ERR_no_second_port;
		ret.internal_err = PS2_ERR_none;

		return ret;
	}

	struct ps2_device_type_verified device_2_type = reset_port_and_get_device_type(PS2_PN_port_two);
	if (device_2_type.err) {
		kprintf("\n[PANIC] Error in Step 10 of initializing the PS2 Controller. Could not reset port 2\n");
		kprintf("The error value: %u\n", err);
		kprintf("The error: %s\n", PS2_OS_Error_to_string(device_2_type.err));
		ret.ps2_state_err = PS2_ID_ERR_could_not_reset_device2;
		ret.internal_err = device_2_type.err;
		return ret;
	}

	switch (device_2_type.mouse_or_keyboard) {
	case PS2_DST_keyboard:
		keyboard_count++;
		ret.port_two_device_type = device_2_type.type;
		break;
	case PS2_DST_mouse:
		mouse_count++;
		ret.port_two_device_type = device_2_type.type;
		break;
	default:
		abort_msg("Device 2: Impossible device super type!\n");
	}

	if (keyboard_count == 2) {
		ret.ps2_state_err = PS2_ID_ERR_two_keyboard;
	} else if (mouse_count == 2) {
		ret.ps2_state_err = PS2_ID_ERR_two_mouse;
	} else {
		ret.ps2_state_err = PS2_ID_ERR_none;
	}

	ret.internal_err = PS2_ERR_none;
	return ret;
}

[[gnu::unused]] struct ps2_initialize_device_state setup_ps2_controller_no_error_check() {

	// Step 1:
	initialize_usb_controllers();
	disable_legacy_usb_support();

	// Step 2
	bool exist = does_ps2_controller_exist();
	assert(exist, "If it doesn't exist, then we are fucked!\n");

	// Step 3:
	ps2_disable_first_ps2_port();
	ps2_disable_second_ps2_port();

	// Step 4:
	[[gnu::unused]] uint8_t discard = ps2_recieve_raw_response();

	// Step 5
	ps2_verified_response_configuration_byte_t vr_cb = ps2_get_configuration_byte();
	PS2_ConfigurationByte_t config_byte = vr_cb.response.bits;
	config_byte.system_flag_passed_post_one = true;         // qemu bug
	config_byte.first_ps2_port_enabled = false;             // disable first
	config_byte.first_ps2_port_translation_enabled = false; // disable first irq
	// translation enabled: Scan code set 1,
	// Translation Disabled: Scan code set 2, But, we must change the scancode set using the keyboard driver
	config_byte.first_ps2_port_clock_disabled = false; // enable clock signal
	ps2_set_configuration_byte(config_byte);

	// Step 6: Perform controller self test
	struct ps2_verified_response_test_controller tc_vr = ps2_perform_controller_self_test();
	assert(tc_vr.response == PS2_TCR_passed, "we are fucked if it didn't pass\n");

	// Step 7: Determine if there are two channels
	ps2_enable_second_ps2_port();
	vr_cb = ps2_get_configuration_byte();
	config_byte = vr_cb.response.bits;
	bool second_port_supported = !config_byte.second_ps2_port_clock_disabled; // bit 5
	assert(second_port_supported, "Otherwise we are fucked if no mouse support!\n");

	ps2_disable_second_ps2_port();
	vr_cb = ps2_get_configuration_byte();
	config_byte = vr_cb.response.bits;
	config_byte.second_ps2_port_clock_disabled = false;
	config_byte.second_ps2_port_enabled = false;
	ps2_set_configuration_byte(config_byte);

	// Step 8: Test the ps2 ports:
	struct ps2_verified_response_test_port response;
	response = ps2_test_first_ps2_port();
	assert(response.response == PS2_TPR_passed, "First port test must work\n!");
	response = ps2_test_second_ps2_port();
	assert(response.response == PS2_TPR_passed, "Second port test must work\n!");

	// Step 9: Enable devices:
	ps2_enable_first_ps2_port();
	ps2_enable_second_ps2_port();
	vr_cb = ps2_get_configuration_byte();
	config_byte = vr_cb.response.bits;
	config_byte.first_ps2_port_enabled = true;
	config_byte.second_ps2_port_enabled = true;
	ps2_set_configuration_byte(config_byte);

	// Step 10: Reset devices:
	send_command_to_first_ps2_port(PS2_CB_reset_device);
	wait_till_ready_for_response();
	uint8_t rep1k = ps2_recieve_raw_response();
	wait_till_ready_for_response();
	uint8_t rep2k = ps2_recieve_raw_response();
	enum ps2_controller_error_code err = wait_till_ready_for_response();
	if (!err) {
		uint8_t rep3k = ps2_recieve_raw_response();
		kprintf("The response from keyboard: %h, %h, %h\n", rep1k, rep2k, rep3k);
	}
	kprintf("The response from keyboard: %h, %h, None\n", rep1k, rep2k);
	assert(rep1k == 0xfa, "start of reset successful command");
	assert(rep2k == 0xaa, "end of reset successful command");

	send_command_to_second_ps2_port(PS2_CB_reset_device);
	uint8_t rep1m = ps2_recieve_raw_response();
	wait_till_ready_for_response();
	uint8_t rep2m = ps2_recieve_raw_response();
	wait_till_ready_for_response();
	uint8_t rep3m = ps2_recieve_raw_response();
	kprintf("The response from mouse   : %h, %h, %h\n", rep1m, rep2m, rep3m);
	assert(rep1m == 0xfa, "start of reset successful command");
	assert(rep2m == 0xaa, "end of reset successful command");
	// if rep3m == 0x00, then it's a standard ps2 mouse. (No scroll wheel)
}

/* ============ TESTS  =============== */
void test_command_array(void) {
	kprintf("Testing command_has_return array...\n\n");

	for (int i = 0; i < 256; i++) {
		enum PS2_ResponseType val = command_to_response_type[i];
		const char* str = PS2_ResponseType_to_string(val);
		kprintf("%h:03 -> Value: %s\n", i, str); // :03 pads to 3 digits

		// maybe create a function that takes a command, and returns a command name.
		// TODO:
	}

	// Check known important values
	struct {
		int index;
		enum PS2_ResponseType expected;
	} checks[] = {
	    {0x20, PS2_RT_controller_configuration_byte},
	    {0x21, PS2_RT_unknown},
	    {0x3F, PS2_RT_unknown},
	    {0xA9, PS2_RT_test_port},
	    {0xAB, PS2_RT_test_port},
	    {0xAA, PS2_RT_test_controller},
	    {0xD0, PS2_RT_controller_output_port},
	};

	int errors = 0;
	for (size_t i = 0; i < sizeof(checks) / sizeof(checks[0]); i++) {
		enum PS2_ResponseType actual = command_to_response_type[checks[i].index];
		if (actual != checks[i].expected) {
			kprintf("Error: %h:02 expected %s, got %s\n",
			        checks[i].index,
			        PS2_ResponseType_to_string(checks[i].expected),
			        PS2_ResponseType_to_string(actual));
			errors++;
		}
	}

	if (errors == 0) {
		kprintf("\nAll important values are correct! (Note that the current test is too weak)\n");
	} else {
		kprintf("\nTotal errors: %d\n", errors);
	}
}
