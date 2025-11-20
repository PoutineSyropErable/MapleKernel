#include "assert.h"
#include "ps2.h"
#include "ps2_mouse.h"

void quick_enable_mouse() {
	// Let's enable the mouse.
	send_data_to_second_ps2_port(0xf4);
	enum ps2_os_error_code err = wait_till_ready_for_response();
	if (err) {
		abort_msg("Error\n");
	}
	uint8_t res = recieve_raw_response();
	assert(res = 0xFA, "mouse must work");
}
