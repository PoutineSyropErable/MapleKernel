#include "ps2.h"
#include "stdio.h"

#define MAX_PS2_WAIT_LOOP 1000

/* =================================== Global Variables ================================ */
// This is more of a header defined thing, but it shouldn't be global. has 0
//  what allignement to better use for caching?
__attribute__((aligned(64))) const enum PS2_ReponseType command_has_return[256] = {
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
static inline bool ps2_is_controller_ready_for_more_input(PS2_StatusRegister_t status) {
	return !status.input_buffer_full_not_empty;
}

static inline bool ps2_is_controller_ready_for_response(PS2_StatusRegister_t status) {
	return !status.output_buffer_full_not_empty;
}

/* =================================== Internals Functions ================================ */

/* For the PS2 Device to be ready for more inputs. So for the OS to the the next output */
enum ps2_os_error_code_t ps2_wait_till_ready_for_more_input() {

	for (uint32_t i = 0; i < MAX_PS2_WAIT_LOOP; i++) {
		io_wait();
		PS2_StatusRegister_t status = read_ps2_status();
		bool is_ready = ps2_is_controller_ready_for_more_input(status);
		if (is_ready) {
			return PS2_ERR_NONE;
		}
	}
	return PS2_ERR_WAIT_MAX_ITT_IN;
}

/* For the PS2 Device, who is currently calculating the next output to be done with the result ready. So for the OS to read the response */
enum ps2_os_error_code_t ps2_wait_till_ready_for_response() {

	for (uint32_t i = 0; i < MAX_PS2_WAIT_LOOP; i++) {
		io_wait();
		PS2_StatusRegister_t status = read_ps2_status();
		bool is_ready = ps2_is_controller_ready_for_response(status);
		if (is_ready) {
			return PS2_ERR_NONE;
		}
	}
	return PS2_ERR_WAIT_MAX_ITT_OUT;
}

enum ps2_os_error_code_t send_data_to_first_ps2_port(uint8_t data) {

	enum ps2_os_error_code_t err = ps2_wait_till_ready_for_more_input();
	if (err) {
		return err;
	}

	__outb(PS2_DATA_PORT_RW, data);

	return 0;
}

enum ps2_os_error_code_t send_data_to_second_ps2_port(uint8_t data) {

	enum ps2_os_error_code_t err = ps2_wait_till_ready_for_more_input();
	if (err) {
		return err;
	}

	__outb(PS2_COMMAND_PORT_W, PS2_CB_write_next_byte_to_second_ps2_input_buffer);
	err = ps2_wait_till_ready_for_more_input();
	if (err) {
		return PS2_ERR_WAIT_MAX_ITT2;
	}
	__outb(PS2_DATA_PORT_RW, data);

	return 0;
}

// This is used to send data to the first or second ps2 device. Port 1 = Keyboard, Port 2 = mouse
enum ps2_os_error_code_t send_data_to_ps2_port(enum PS2_PORT_NUMBER port_number, uint8_t data) {

	switch (port_number) {
	case 1:
		return send_data_to_first_ps2_port(data);
	case 2:
		return send_data_to_second_ps2_port(data);
	default:
		return PS2_ERR_INVALID_PORT_NUMBER;
	}
}

enum ps2_os_error_code_t send_command_to_ps2_controller(enum PS2_CommandByte command) {
	enum ps2_os_error_code_t err = ps2_wait_till_ready_for_more_input();
	if (err) {
		return err;
	}

	__outb(PS2_COMMAND_PORT_W, (uint8_t)command);
	return 0;
}

/* ============ TESTS  =============== */
void test_command_array(void) {
	kprintf("Testing command_has_return array...\n\n");

	for (int i = 0; i < 256; i++) {
		enum PS2_ReponseType val = command_has_return[i];
		const char* str = PS2_ResponseType_to_string(val);
		kprintf("%h:03 -> Value: %s\n", i, str); // :03 pads to 3 digits
	}

	// Check known important values
	struct {
		int index;
		enum PS2_ReponseType expected;
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
		enum PS2_ReponseType actual = command_has_return[checks[i].index];
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
