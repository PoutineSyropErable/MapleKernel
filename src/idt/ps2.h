#pragma once
#include "intrinsics.h"
#define PS2_DATA_PORT_RW 0x60   // Read and write
#define PS2_STATUS_PORT_R 0x64  // read.
#define PS2_COMMAND_PORT_W 0x64 // write

/* ==================================================================================================================================================*/
// PS/2 Controller Commands

// Most data sheets for the PS/2 controller are written from the perspective of the PS/2 device, not from the OS Developper.
// output buffer = What My OS read from the Controller
// input buffer = What my os write to the controller
typedef enum PS2_CommandByte {
	PS2_CB_read_byte_0 = 0x20,
	PS2_CB_read_byte_N_start = 0x21,
	PS2_CB_read_byte_N_end = 0x3F,

	PS2_CB_write_next_to_byte_0 = 0x60,
	PS2_CB_write_next_to_byte_N_start = 0x61,
	PS2_CB_write_next_to_byte_N_end = 0x62,

	PS2_CB_disable_second_ps2_port = 0xA7,
	PS2_CB_enable_second_ps2_port = 0xA8,
	PS2_CB_test_second_ps2_port = 0xA9,
	PS2_CB_test_ps2_controller = 0xAA,
	PS2_CB_test_first_ps2_port = 0xAB,

	PS2_CB_diagnostic_dump = 0xAC, // read all bytes from internal ram
	PS2_CB_disable_first_ps2_port = 0xAD,
	PS2_CB_enable_first_ps2_port = 0xAE,

	PS2_CB_read_controller_input_port = 0xC0,
	PS2_CB_copy_low_input_to_status = 0xC1,  // Copy bits 0 to 3 of input port to status bits 4 to 7
	PS2_CB_copy_high_input_to_status = 0xC2, // Copy bits 4 to 7 of input port to status bits 4 to 7

	PS2_CB_read_controller_output_port = 0xD0,
	PS2_CB_write_next_byte_to_controller_output_port = 0xD1, // Check if the output buffer is empty first
	PS2_CB_write_next_byte_to_first_ps2_output_buffer = 0xD2,
	PS2_CB_write_next_byte_to_second_ps2_output_buffer = 0xD3,
	PS2_CB_write_next_byte_to_second_ps2_input_buffer = 0xD4,

	PS2_CB_pulse_output_line_low_start = 0xF0, // the duration of the pulse is 6ms
	PS2_CB_pulse_output_lone_low_end = 0xFF
	// bits 3 to 0 are used as a mask. (0 = pulse line, 1 = don't pulse line) and correspond to 4 different output lines.
	// Notes: Bits 0 correspond to the "reset" line. The other output lines don't have a standard/defined purpose.

} PS2_CommandByte_t;
typedef uint8_t ps2_command_byte_t;
/* ==================================================================================================================================================*/

typedef struct [[gnu::packed]] {
	bool output_buffer_full_not_empty : 1;
	bool input_buffer_full_not_empty : 1;
	bool system_flag : 1; // cleared by reset, and set by firmare
	bool command_not_data : 1;
	// 1 = data written to input buffer is data for the PS/2 controller command.
	// 0 = data written to input buffer is for data for PS/2 device
	// =====
	bool unknown_4 : 1;     // more likely unused on modern systems
	bool unknown_5 : 1;     // may ne recieve time-out, or second PS/2 port output buffer full
	bool timeout_error : 1; // 1 = timeout error, 0 = no error
	bool parity_error : 1;  // 1 = parity error, 0 = no parity error

} PS2_StatusRegister_t;
_Static_assert(sizeof(PS2_StatusRegister_t) == 1, "ps2_satus_register_t must be of size: 1 byte");

typedef union {
	uint8_t raw;
	PS2_StatusRegister_t bits;
} ps2_satus_register_union_typesafe_t;
/* ==================================================================================================================================================*/

typedef struct [[gnu::packed]] PS2_ConfigurationByte {
	bool first_ps2_port_enabled : 1;
	bool second_ps2_port_enabled : 1;
	bool system_flag_passed_post_one : 1; // 0 should be impossible. If post wasn't pasted, then it failed to boot.
	bool zero1 : 1;                       // should be 0.
	bool first_ps2_port_clock_enabled : 1;
	bool second_ps2_port_clock_enabled : 1;
	bool first_ps2_port_translation_enabled : 1;
	bool zero2 : 1; // must be zero.
} PS2_ConfigurationByte_t;
_Static_assert(sizeof(PS2_ConfigurationByte_t) == 1, "PS2_ConfigurationByte_t must be of size: 1 byte");

typedef union {
	uint8_t raw;
	PS2_ConfigurationByte_t bits;
} ps2_configuration_byte_union_typesafe_t;
/* ==================================================================================================================================================*/

typedef struct [[gnu::packed]] PS2_ControllerOutputPort {
	bool system_reset_dont_set_infinite_loop : 1; // Must be
	bool A20_gate : 1;
	bool second_ps2_port_clock : 1;
	bool second_ps2_port_data : 1;
	bool connected_to_irq1 : 1;  // Output buffer full with byte from first PS/2 port
	bool connected_to_irq12 : 1; // Output buffer full with byte from second PS/2 port
	bool first_ps2_port_clock : 1;
	bool first_ps2_port_data : 1;
} PS2_ControllerOutputPort_t;
_Static_assert(sizeof(PS2_ControllerOutputPort_t) == 1, "PS2_ControllerOutputPort_t must be of size: 1 byte");

typedef union {
	uint8_t raw;
	PS2_ControllerOutputPort_t bits;
} ps2_controller_output_port_union_typesafe;

/* ==================================================================================================================================================*/

enum PS2_PORT_NUMBER {
	// For devices
	PORT_ONE = 1,
	PORT_TWO = 2,
};

enum ps2_os_error_code {
	PS2_ERR_NONE = 0,
	PS2_ERR_INVALID_PORT_NUMBER = -1,
	PS2_ERR_WAIT_MAX_ITT = 1,
	PS2_ERR_WAIT_MAX_ITT2 = 2,

};

/* ==================================================================================================================================================*/
/*==================== INFORMATION ======================= */
/*
The PS/2 Controller accepts commands and performs them. These commands should not be confused with bytes sent to a PS/2 device (e.g. keyboard, mouse).

To send a command to the controller, write the command byte to IO port 0x64.
If the command is 2 bytes long,
then the next byte needs to be written to IO Port 0x60 after making sure that the controller is
ready for it (by making sure bit 1 of the Status Register is clear). If there is a response byte,
then the response byte needs to be read from IO Port 0x60 after making sure it has arrived (by making sure bit 0 of the Status Register is set).

*/

/*==================GLOBAL FUNCTIONS===================== */
static inline void io_wait(void) {
	// ox80 is an unusded port. Check os dev for useful static inline functions for os developppers
	__outb(0x80, 0);
}

__attribute__((optimize("O3"))) static inline PS2_StatusRegister_t read_ps2_status() {
	ps2_satus_register_union_typesafe_t u;
	u.raw = __inb(PS2_STATUS_PORT_R);
	return u.bits;
}

// Input that it recieves, aka that we SEND from the OS.
static inline bool ps2_is_controller_ready_for_more_input(PS2_StatusRegister_t status) {
	return !status.input_buffer_full_not_empty;
}

enum ps2_os_error_code ps2_wait_till_ready_for_more_input();
enum ps2_os_error_code send_data_to_ps2_port(enum PS2_PORT_NUMBER port_number, uint8_t data);
enum ps2_os_error_code send_command_to_ps2_controller(enum PS2_CommandByte command);

void ps2_detect_devices_type();
