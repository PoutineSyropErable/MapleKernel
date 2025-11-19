#include "assert.h"
#include "ps2.h"
#include "stdio.h"

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
	return !status.input_buffer_full_not_empty;
}

static inline bool is_ps2_controller_ready_for_response(PS2_StatusRegister_t status) {
	return !status.output_buffer_full_not_empty;
}

// This function assume PS2 Controller is ready
static inline uint8_t recieve_raw_response() {
	return __inb(PS2_DATA_PORT_RW);
}

/* =================================== Internals Functions ================================ */

/* For the PS2 Device to be ready for more inputs. So, wait for the ps2 controller to be ready for the OS to send the next output */
enum ps2_os_error_code wait_till_ready_for_more_input() {

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
enum ps2_os_error_code wait_till_ready_for_response() {

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

static inline enum ps2_os_error_code send_command_to_ps2_controller(enum PS2_CommandByte command) {
	enum ps2_os_error_code err = wait_till_ready_for_more_input();
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

	uint8_t raw_response = recieve_raw_response();
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

	uint8_t raw_response = recieve_raw_response();
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
		valid = PS2_verify_configuration_byte_response(response_cb);
		if (!valid) {
			ret.tagged_response = tagged_response;
			ret.err = PS2_ERR_invalid_configuration_byte;
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

	enum ps2_os_error_code err = send_command_to_ps2_controller(command);
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
static inline enum ps2_os_error_code
send_command_none_response(enum PS2_CommandByte command) {
	return send_command_to_ps2_controller(command);
}

// Unknown response
ps2_verified_response_unknown_t send_command_unknown_response(enum PS2_CommandByte command) {
	ps2_verified_response_unknown_t ret;

	enum ps2_os_error_code err = send_command_to_ps2_controller(command);
	if (err) {
		ret.err = err;
		return ret;
	}

	err = wait_till_ready_for_response();
	if (err) {
		ret.err = err;
		return ret;
	}

	uint8_t raw_response = recieve_raw_response();
	ret.response = raw_response;
	return ret;
}

// Configuration Byte response
static inline ps2_verified_response_configuration_byte_t
send_command_configuration_byte_response(enum PS2_CommandByte command) {
	ps2_verified_response_configuration_byte_t ret;

	enum ps2_os_error_code err = send_command_to_ps2_controller(command);
	if (err) {
		ret.err = err;
		return ret;
	}

	err = wait_till_ready_for_response();
	if (err) {
		ret.err = err;
		return ret;
	}

	uint8_t raw_response = recieve_raw_response();
	ret.response.raw = raw_response;
	bool valid = PS2_verify_configuration_byte_response(ret.response);
	if (!valid) {
		ret.err = PS2_ERR_invalid_configuration_byte;
		return ret;
	}

	ret.err = PS2_ERR_none;
	return ret;
}

// Test Port response
ps2_verified_response_test_port_t send_command_test_port_response(enum PS2_CommandByte command) {
	ps2_verified_response_test_port_t ret;

	enum ps2_os_error_code err = send_command_to_ps2_controller(command);
	if (err) {
		ret.err = err;
		return ret;
	}

	err = wait_till_ready_for_response();
	if (err) {
		ret.err = err;
		return ret;
	}

	uint8_t raw_response = recieve_raw_response();
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

	enum ps2_os_error_code err = send_command_to_ps2_controller(command);
	if (err) {
		ret.err = err;
		return ret;
	}

	err = wait_till_ready_for_response();
	if (err) {
		ret.err = err;
		return ret;
	}

	uint8_t raw_response = recieve_raw_response();
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

	enum ps2_os_error_code err = send_command_to_ps2_controller(command);
	if (err) {
		ret.err = err;
		return ret;
	}

	err = wait_till_ready_for_response();
	if (err) {
		ret.err = err;
		return ret;
	}

	uint8_t raw_response = recieve_raw_response();
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
static inline enum ps2_os_error_code write_next_to_byte_0_of_interal_ram() {
	return send_command_none_response(PS2_CB_write_next_to_byte_0);
}

[[maybe_unused]] static inline enum ps2_os_error_code
write_next_to_byte_n_of_interal_ram(uint8_t byte_index) {
	assert(byte_index < (PS2_CB_write_next_to_byte_N_end - PS2_CB_write_next_to_byte_0), "must be in allowed byte range!\n");

	enum ps2_os_error_code err;
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
enum ps2_os_error_code ps2_set_configuration_byte(PS2_ConfigurationByte_t config_byte) {
	union ps2_configuration_byte_uts config_byte_uts = {.bits = config_byte};

	enum ps2_os_error_code err = write_next_to_byte_0_of_interal_ram();
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

enum ps2_os_error_code ps2_disable_first_ps2_port() {
	return send_command_to_ps2_controller(PS2_CB_disable_first_ps2_port);
}

enum ps2_os_error_code ps2_enable_first_ps2_port() {
	return send_command_to_ps2_controller(PS2_CB_enable_first_ps2_port);
}

/* Only if 2 PS/2 port are supported. Do not use this function for the test. Use the configuration byte
Set a configuration byte with it enabled, and read it back. check if it's enabled.
*/
enum ps2_os_error_code ps2_disable_second_ps2_port() {
	return send_command_to_ps2_controller(PS2_CB_disable_second_ps2_port);
}

/* Only if 2 PS/2 port are supported. Do not use this function for the test. Use the configuration byte
Set a configuration byte with it enabled, and read it back. check if it's enabled.
*/
enum ps2_os_error_code ps2_enable_second_ps2_port() {
	return send_command_to_ps2_controller(PS2_CB_enable_second_ps2_port);
}
// ===============

struct ps2_verified_response_test_port test_first_ps2_port() {
	return send_command_test_port_response(PS2_CB_test_first_ps2_port);
}

/* Only if 2 PS/2 port are supported. Do not use this function for the test. Use the configuration byte
Set a configuration byte with it enabled, and read it back. check if it's enabled.
*/
struct ps2_verified_response_test_port test_second_ps2_port() {
	return send_command_test_port_response(PS2_CB_test_second_ps2_port);
}
// ===============
struct ps2_verified_response_test_controller ps2_test_controller() {
	return send_command_test_controller_response(PS2_CB_test_ps2_controller);
}
// ===============
// diagnostic dump, idk how to implement it. A loop that reads?

// ===============

// This function is only used by manifucaturer to check if the things work.
// It shouldn't even be implemented or used
enum ps2_os_error_code ps2_copy_input_port_to_status() {
	enum ps2_os_error_code err;
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
static inline enum ps2_os_error_code
ps2_write_next_byte_to_controller_output_port() {
	return send_command_none_response(PS2_CB_write_next_byte_to_controller_output_port);
}

enum ps2_os_error_code ps2_set_controller_output_port(PS2_ControllerOutputPort_t output_port) {
	union ps2_controller_output_port_uts output_port_uts = {.bits = output_port};
	enum ps2_os_error_code err;
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
enum ps2_os_error_code write_next_byte_to_first_ps2_output_port() {
	return send_command_none_response(PS2_CB_write_next_byte_to_first_ps2_output_buffer);
}

enum ps2_os_error_code write_next_byte_to_second_ps2_output_port() {
	return send_command_none_response(PS2_CB_write_next_byte_to_second_ps2_output_buffer);
}
enum ps2_os_error_code fake_ps2_keyboard_byte(uint8_t byte) {
	enum ps2_os_error_code err;
	err = write_next_byte_to_first_ps2_output_port();
	if (err) {
		return err;
	}

	__outb(PS2_DATA_PORT_RW, byte);
	return PS2_ERR_none;
}
enum ps2_os_error_code fake_ps2_mouse_byte(uint8_t byte) {
	enum ps2_os_error_code err;
	err = write_next_byte_to_second_ps2_output_port();
	if (err) {
		return err;
	}

	__outb(PS2_DATA_PORT_RW, byte);
	return PS2_ERR_none;
}

// ===============
enum ps2_os_error_code
send_data_to_first_ps2_port(uint8_t data) {

	enum ps2_os_error_code err = wait_till_ready_for_more_input();
	if (err) {
		return err;
	}

	__outb(PS2_DATA_PORT_RW, data);

	return PS2_ERR_none;
}

enum ps2_os_error_code send_data_to_second_ps2_port(uint8_t data) {

	enum ps2_os_error_code err = wait_till_ready_for_more_input();
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
enum ps2_os_error_code send_data_to_ps2_port(enum PS2_PortNumber port_number, uint8_t data) {

	switch (port_number) {
	case 1:
		return send_data_to_first_ps2_port(data);
	case 2:
		return send_data_to_second_ps2_port(data);
	default:
		return PS2_ERR_invalid_port_number;
	}
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
