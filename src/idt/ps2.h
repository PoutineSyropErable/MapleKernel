#pragma once
#include "intrinsics.h"

// Compile-time check for little-endian
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#error "Big-endian systems are not supported!"
#elif !defined(__BYTE_ORDER__)
#error "Cannot detect system endianness!"
#endif

#define PS2_DATA_PORT_RW 0x60   // Read and write
#define PS2_STATUS_PORT_R 0x64  // read.
#define PS2_COMMAND_PORT_W 0x64 // write

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
} ps2_status_register_uts_t;
_Static_assert(sizeof(ps2_status_register_uts_t) == 1, "ps2_satus_register_t must be of size: 1 byte");

/* ==================================================================================================================================================*/
// PS/2 Controller Commands

/* Most data sheets for the PS/2 controller are written from the perspective of the PS/2 device, not from the OS Developper. |

Output buffer = What My OS read from the Controller. |
Input buffer = What my os write to the controller. | */
typedef enum PS2_CommandByte {
	PS2_CB_read_byte_0 = 0x20,       // response byte type : Controller configuration byte. (2nd type of response)
	PS2_CB_read_byte_N_start = 0x21, // response byte type :unknown (1st reponse type)
	PS2_CB_read_byte_N_end = 0x3F,   // response byte type: unknown

	PS2_CB_write_next_to_byte_0 = 0x60,       // response byte type: None (0th response type)
	PS2_CB_write_next_to_byte_N_start = 0x61, // response byte type: None
	PS2_CB_write_next_to_byte_N_end = 0x7F,   // response byte type: None

	PS2_CB_disable_second_ps2_port = 0xA7, // response byte type: None
	PS2_CB_enable_second_ps2_port = 0xA8,  // response byte type: None
	PS2_CB_test_second_ps2_port = 0xA9,    // response byte type: (passed = 0x00, 0x01 = clock line stuck low, 0x02 = clock line stuck high,  0x03 = data line stuck low, 0x04 = data line stuck high)
	PS2_CB_test_ps2_controller = 0xAA,     // response byte type: (0x55 = test passed, 0xFC = test failed). (4nd type of response)
	PS2_CB_test_first_ps2_port = 0xAB,     // response byte type: Same as 0xA9^^ . (3nd type of response)

	PS2_CB_diagnostic_dump = 0xAC, // response byte type : Unknown
	// read all bytes from internal ram
	PS2_CB_disable_first_ps2_port = 0xAD, //  response byte type: None
	PS2_CB_enable_first_ps2_port = 0xAE,  //  response byte type: None

	PS2_CB_read_controller_input_port = 0xC0, // response byte type: Unkown
	PS2_CB_copy_low_input_to_status = 0xC1,   // response byte type: None
	// Copy bits 0 to 3 of input port to status bits 4 to 7
	PS2_CB_copy_high_input_to_status = 0xC2, // response byte type: None
	// Copy bits 4 to 7 of input port to status bits 4 to 7

	PS2_CB_read_controller_output_port = 0xD0,                 // response byte type: Controller Output Port. (5th type of response)
	PS2_CB_write_next_byte_to_controller_output_port = 0xD1,   // Check if the output buffer is empty first
	PS2_CB_write_next_byte_to_first_ps2_output_buffer = 0xD2,  // response byte type: None
	PS2_CB_write_next_byte_to_second_ps2_output_buffer = 0xD3, // response byte type: None
	PS2_CB_write_next_byte_to_second_ps2_input_buffer = 0xD4,  // response byte type: None

	PS2_CB_pulse_output_line_low_start = 0xF0, // response byte type: None
	PS2_CB_pulse_output_line_low_end = 0xFF    // response byte type: None
	                                           // the duration of the pulse is 6ms
	                                           // bits 3 to 0 are used as a mask. (0 = pulse line, 1 = don't pulse line) and correspond to 4 different output lines.
	                                           // Notes: Bits 0 correspond to the "reset" line. The other output lines don't have a standard/defined purpose.

} PS2_CommandByte_t;
_Static_assert(sizeof(PS2_CommandByte_t) == 4, "PS2_ConfigurationByte_t is stupidly 4 bytes because enums ALWAYS use ints. BAD C!");

/*
The value is for low level access!
The enum is for switch, cases and general type safety!
This breaks on big endian!
== *MIGHT NOT BE NEEDED* ==
*/
typedef union ps2_command_byte {
	uint8_t value;
	PS2_CommandByte_t typedEnum;
} ps2_command_byte_t;
_Static_assert(sizeof(ps2_command_byte_t) == 4, "The union's size is stupidly 4 bytes because enums ALWAYS use ints. BAD C!");
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

typedef union ps2_configuration_byte_uts {
	uint8_t raw;
	PS2_ConfigurationByte_t bits;
} ps2_configuration_byte_uts_t;
_Static_assert(sizeof(ps2_configuration_byte_uts_t) == 1, "ps2_configuration_byte_uts_t must be of size: 1 byte");

/* ==================================================================================================================================================*/
// PS2_CB_test_second_ps2_port = 0xA9,    // response byte type: (passed = 0x00, 0x01 = clock line stuck low, 0x02 = clock line stuck high,  0x03 = data line stuck low, 0x04 = data line stuck high)

typedef enum PS2_TestPortResponse {
	PS2_TPR_passed = 0x00,
	PS2_TPR_clock_stuck_low = 0x01,
	PS2_TPR_clock_stuck_high = 0x02,
	PS2_TPR_data_stuck_low = 0x03,
	PS2_TPR_data_stuck_high = 0x04,
} PS2_TestPortResponse_t;

typedef enum PS2_TestControllerResponse {
	PS2_TCR_passed = 0xFF,
	PS2_TCR_clock_stuck_low = 0xFE,
} PS2_TestControllerResponse_t;

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
} ps2_controller_output_port_uts;

/* ==================================================================================================================================================*/

enum PS2_PORT_NUMBER {
	// For devices
	PS2_PN_port_one = 1,
	PS2_PN_port_2 = 2,
};

enum PS2_ReponseType {
	PS2_RT_not_a_command = 0,
	PS2_RT_none = 1,
	PS2_RT_unknown = 2,
	PS2_RT_controller_configuration_byte = 3,
	PS2_RT_test_port = 4,
	PS2_RT_test_controller = 5,
	PS2_RT_controller_output_port = 6,
};

/*
This struct should never appear in code. Only it's individual field should be used.
And even then, the polymorphic solution shouldn't be used. It's too slow.
We'll end up with a low level function, and high level at the same time.
It's purely there to have the types of all response type written next to another real quick
*/
union _anon1 {
	uint8_t not_a_command;                                    // Can't and shouldn't even send it! This case should never be inside a tagged response, an error should have happened.
	uint8_t none;                                             // Send the command, but don't even read the answer.
	uint8_t unknown;                                          // Get an answer, but don't do anything with it.
	union ps2_configuration_byte_uts configuration_byte;      // usable answer
	enum PS2_TestPortResponse test_port_response;             // usable answer
	enum PS2_TestControllerResponse test_controller_response; // usable anser
	struct PS2_ControllerOutputPort controller_output_port;   // usable answer
};

struct PS2_Tagged_Reponse {
	enum PS2_ReponseType type;
	union _anon1 value;
};

enum ps2_os_error_code_t {
	PS2_ERR_NONE = 0,
	PS2_ERR_INVALID_PORT_NUMBER = -1,
	PS2_ERR_WAIT_MAX_ITT_IN = 1,
	PS2_ERR_WAIT_MAX_ITT_OUT = 2,
	PS2_ERR_WAIT_MAX_ITT2 = 3,

};

typedef struct ps2_return_type_generic {
	uint8_t ret;
	bool has_ret; // some of them have no return
	enum ps2_os_error_code_t err;
} ps2_os_ret_generic_t;

/* ==================================================================================================================================================*/

/*==================== C Compiler INFORMATION ======================= */
/*
1. Only signed values are sign extended.
    - So, if I have
    ```
    uint8_t input = 129; (Bigger then 127, so last bit is 1.)
    int output = input;
    // output == 129. It will not sign extend the uint.

    but
    int8_t input = - 120;
    int32_t output = input;
    // output == -120; (sign extension only happen if the source is a signed type)
    . So uint8_t and enums (with positive values) can safely be:  Enum <-----> uint8_t
*/
/*==================== DESIGN INFORMATION ======================= */
/*
1. Only the lowercase types should be used as inputs and outputs of functions.
    - Exception, if we use the enum, or the struct, not the _t. Then the upper case can be used.
2. When typesafe is inside the name of an union, it means that it's size hasn't been changed from what's expected.
    - Aka, if the expected lower level/hardware size(type) should be 8bits (1byte), then the union did not change it's type.
    - And you can read or write to whatever element.
    - This is better then the 'evil bit hack trick':  `type2 y =  * (type2 *) &x`
3. Enums stupidly use ints for implementation, and unlike in C++, this isn't changeable, even in C23!
    - So, a union is created, and this union HAS A SIZE OF 4 BYTES. So, this union should be used for function input and output?
    - But, what is passed to hardware stuff should be the raw uint8_t value.
    - However, enum, unlike struct, can be trivially casted to uint8_t, explecitely and STATICALLY!
    - So, it's fine to not use it. (Remember, that integers are signed, and uint8_t etc are not!)
4. Related to 2 and 1. UTS = union typesafe.
    - uts means union typesafe
    - uts definition
    - meaning of uts
    - Other search Keywords. (Which that too is a keyword for simply unknown words)

    ```


*/

/* ==================================================================================================================================================*/
/*==================== OTHER EXTERNAL INFORMATION ======================= */
/*
The PS/2 Controller accepts commands and performs them. These commands should not be confused with bytes sent to a PS/2 device (e.g. keyboard, mouse).

To send a command to the controller, write the command byte to IO port 0x64.
If the command is 2 bytes long,
then the next byte needs to be written to IO Port 0x60 after making sure that the controller is
ready for it (by making sure bit 1 of the Status Register is clear). If there is a response byte,
then the response byte needs to be read from IO Port 0x60 after making sure it has arrived (by making sure bit 0 of the Status Register is set).

*/

/*=================== Type and Enum -> String Function ===============*/

static inline const char* PS2_ResponseType_to_string(enum PS2_ReponseType response_type) {
	// The string is located inside the .data section, this return a pointer to that section
	switch (response_type) {
	case PS2_RT_none:
		return "None";
	case PS2_RT_unknown:
		return "Unknown";
	case PS2_RT_controller_configuration_byte:
		return "Controller config";
	case PS2_RT_test_port:
		return "Test port";
	case PS2_RT_test_controller:
		return "Test controller";
	case PS2_RT_controller_output_port:
		return "Controller output";
	case PS2_RT_not_a_command:
		return "Not a command";
	default:
		return "Invalid";
	}
}

/*==================GLOBAL FUNCTIONS===================== */
static inline void
io_wait(void) {
	// ox80 is an unusded port. Check os dev for useful static inline functions for os developppers
	__outb(0x80, 0);
}

__attribute__((optimize("O3"))) static inline PS2_StatusRegister_t read_ps2_status() {
	ps2_status_register_uts_t u;
	u.raw = __inb(PS2_STATUS_PORT_R);
	return u.bits;
}

enum ps2_os_error_code_t ps2_wait_till_ready_for_more_input();
enum ps2_os_error_code_t send_data_to_ps2_port(enum PS2_PORT_NUMBER port_number, uint8_t data);
enum ps2_os_error_code_t send_command_to_ps2_controller(enum PS2_CommandByte command);

void ps2_detect_devices_type();

/*============= TESTS ============ */
void test_command_array(void);
