#include "ps2.h"

#define MAX_PS2_WAIT_LOOP 1000

/* =================================== Global Variables ================================ */
// This is more of a header defined thing, but it shouldn't be global. has 0
//  what allignement to better use for caching?
__attribute__((aligned(64))) enum PS2_ReponseType command_has_return[256] = {
    [0 ... 0xFF] = PS2_RT_not_a_command, // default all to none

    // Known ranges, ignore the warning?
    [0x20] = PS2_RT_controller_configuration_byte,
    [0x21 ... 0x3F] = PS2_RT_unknown,

    [0xA9] = PS2_RT_test_port,
    [0xAB] = PS2_RT_test_port,
    [0xAB] = PS2_RT_unknown,
    [0xAA] = PS2_RT_test_controller,
    [0xD0] = PS2_RT_controller_output_port,
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
