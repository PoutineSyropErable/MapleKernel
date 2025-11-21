#include "assert.h"
#include "ps2_controller.h"
#include "ps2_mouse.h"

uint8_t _number_of_mouse;
uint8_t _single_mouse_port;

uint8_t _mouse1_port;
uint8_t _mouse2_port;

void set_single_mouse_port(uint8_t single_mouse_port) {
	_single_mouse_port = single_mouse_port;
	_number_of_mouse = 1;
}

void set_dual_mouse_port() {
	_mouse1_port = 1;
	_mouse2_port = 2;
	_number_of_mouse = 2;
	// True by definition and hardware
}

void quick_enable_mouse() {
	// Let's enable the mouse.
	send_command_to_second_ps2_port(0xf4);
	enum ps2_controller_error_code err = wait_till_ready_for_response();
	if (err) {
		abort_msg("Error\n");
	}
	uint8_t res = ps2_recieve_raw_response();
	assert(res = 0xFA, "mouse must work");
}
