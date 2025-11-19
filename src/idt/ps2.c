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

enum ps2_os_error_code send_data_to_first_ps2_port(uint8_t data) {

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
		return PS2_ERR_wait_maxx_itt_in2;
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

enum ps2_os_error_code send_command_to_ps2_controller(enum PS2_CommandByte command) {
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
			ret.err = PS2_ERR_A20_line_not_set;
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
enum ps2_os_error_code send_command_none_response(enum PS2_CommandByte command) {
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
ps2_verified_response_configuration_byte_t send_command_configuration_byte_response(enum PS2_CommandByte command) {
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
ps2_verified_response_test_controller_t send_command_test_controller_response(enum PS2_CommandByte command) {
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
ps2_verified_response_controller_output_port_t send_command_test_controller_output_port_response(enum PS2_CommandByte command) {
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
		ret.err = PS2_ERR_A20_line_not_set;
		return ret;
	}

	ret.err = PS2_ERR_none;
	return ret;
}

/* ========================================= steps functions =================================== */
// These are functions that represent concrete steps. They might actually be better put in another file, I do not know.

ps2_verified_response_configuration_byte_t ps2_read_byte_0_from_internal_ram() {
	return send_command_configuration_byte_response(PS2_CB_read_byte_0);
}

ps2_verified_response_unknown_t ps2_read_byte_n_from_internal_ram(uint8_t byte_index) {
	// should I allow reading byte 0? Let's allow it, but return an error.
	// But this may return an error, depending on hardware?
	assert(byte_index < (PS2_CB_read_byte_N_end - PS2_CB_read_byte_0), "must be in allowed byte range!\n");
	assert(byte_index != 0, "0 byte might cause errors?");

	ps2_verified_response_unknown_t ret;
	enum PS2_CommandByte command_byte = PS2_CB_read_byte_0 + byte_index;
	ret = send_command_unknown_response(command_byte);

	if (byte_index == 0) {
		// Maybe remove this?
		ret.err = PS2_ERR_n_is_zero;
	}
	return ret;
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
