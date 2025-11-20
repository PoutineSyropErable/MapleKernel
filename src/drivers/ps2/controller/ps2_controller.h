#pragma once
#include "intrinsics.h"
#include "stdint.h"
#ifndef PS2_DRIVER_PUBLIC
#define PS2_DRIVER_PUBLIC
#endif

#ifdef PS2_DRIVER_INTERNAL
#error "Should not use the public and internal header. Pick one or the other"
#endif

#define PS2_DATA_PORT_RW 0x60   // Read and write
#define PS2_STATUS_PORT_R 0x64  // read.
#define PS2_COMMAND_PORT_W 0x64 // write

enum ps2_os_error_code {
	PS2_ERR_none = 0,
	PS2_ERR_invalid_port_number = -1,
	PS2_ERR_wait_max_itt_in = 1,
	PS2_ERR_wait_max_itt_out = 2,
	PS2_ERR_invalid_test_port_response = 3,
	PS2_ERR_invalid_test_controller_response = 4,
	PS2_ERR_invalid_configuration_byte = 5,
	PS2_ERR_status_parity = 6,
	PS2_ERR_status_timeout = 7,

	// Warnings
	PS2_WARN_A20_line_not_set = 1000,
	PS2_WARN_n_is_zero = 999,

};

// This function assume PS2 Controller is ready
static inline uint8_t recieve_raw_response() {
	return __inb(PS2_DATA_PORT_RW);
}

enum ps2_os_error_code send_data_to_second_ps2_port(uint8_t data);
