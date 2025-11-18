#include "ps2.h"

#define MAX_PS2_WAIT_LOOP 1000
enum ps2_os_error_code ps2_wait_till_ready_for_more_input() {

	for (uint32_t i = 0; i < MAX_PS2_WAIT_LOOP; i++) {
		io_wait();
		PS2_StatusRegister_t status = read_ps2_status();
		bool is_ready = ps2_is_controller_ready_for_more_input(status);
		if (is_ready) {
			return PS2_ERR_NONE;
		}
	}
	return PS2_ERR_WAIT_MAX_ITT;
}

enum ps2_os_error_code send_data_to_first_ps2_port(uint8_t data) {

	enum ps2_os_error_code err = ps2_wait_till_ready_for_more_input();
	if (err) {
		return err;
	}

	__outb(PS2_DATA_PORT_RW, data);

	return 0;
}

enum ps2_os_error_code send_data_to_second_ps2_port(uint8_t data) {

	enum ps2_os_error_code err = ps2_wait_till_ready_for_more_input();
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
enum ps2_os_error_code send_data_to_ps2_port(enum PS2_PORT_NUMBER port_number, uint8_t data) {

	switch (port_number) {
	case 1:
		return send_data_to_first_ps2_port(data);
	case 2:
		return send_data_to_second_ps2_port(data);
	default:
		return PS2_ERR_INVALID_PORT_NUMBER;
	}
}

enum ps2_os_error_code send_command_to_ps2_controller(enum PS2_CommandByte command) {
	enum ps2_os_error_code err = ps2_wait_till_ready_for_more_input();
	if (err) {
		return err;
	}
}
