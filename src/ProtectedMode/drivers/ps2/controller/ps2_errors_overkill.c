#include "ps2_controller.h"
#define MAX_ERR_COUNT 10
/*
This is somewhat too powerful of an error checker. I just implemented it a bit for the fun of it
And might very well completely slow down the code a lot more then it should.
But, if i want, I could track 10 nested errors.
(Only start increasing i once the first non zero is hit)
Then:
Error of first function, cause error in second function, cause error in third function.

*/
struct ps2_os_multi_error {
	uint8_t index;
	enum ps2_os_error_code errors[MAX_ERR_COUNT];
	const char* funcs[MAX_ERR_COUNT];
};

struct ps2_os_multi_error_response {
	struct ps2_os_multi_error error_list;
	uint8_t ret;
};

#define push_error(mult, errcode) \
	write_error((mult), (errcode), __func__)

static inline void
write_error(struct ps2_os_multi_error* mult, enum ps2_os_error_code err, const char* func) {
	uint8_t i = mult->index;
	assert(!(err && i == MAX_ERR_COUNT - 1), "Can't add more errors! Will buffer overflow. Should have been fixed earlier");

	if (err) {
		mult->errors[i] = err;
		mult->funcs[i] = func;
		mult->index++;
	}
}
static inline enum ps2_os_error_code latest_error(struct ps2_os_multi_error mult) {
	return mult.errors[mult.index];
}

/* =========================== Start of C File ================================= */
#define MAX_PS2_WAIT_LOOP 1000

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

// =============================== Multi error versions ====================================
struct ps2_os_multi_error_response wait_till_ready_for_more_input() {
	struct ps2_os_multi_error_response resp = {0};
	for (uint32_t i = 0; i < MAX_PS2_WAIT_LOOP; i++) {
		io_wait();
		PS2_StatusRegister_t status = read_ps2_status();
		bool is_ready = is_ps2_controller_ready_for_more_input(status);
		if (is_ready) {
			if (!PS2_verify_status_parity(status)) {
				push_error(&resp.error_list, PS2_ERR_status_parity);
				return resp;
			}
			if (!PS2_verify_status_timeout(status)) {
				push_error(&resp.error_list, PS2_ERR_status_timeout);
				return resp;
			}
			return resp; // no error
		}
	}
	push_error(&resp.error_list, PS2_ERR_wait_max_itt_in);
	return resp;
}

struct ps2_os_multi_error_response wait_till_ready_for_response() {
	struct ps2_os_multi_error_response resp = {0};
	for (uint32_t i = 0; i < MAX_PS2_WAIT_LOOP; i++) {
		io_wait();
		PS2_StatusRegister_t status = read_ps2_status();
		bool is_ready = is_ps2_controller_ready_for_response(status);
		if (is_ready) {
			return resp; // no error
		}
	}
	push_error(&resp.error_list, PS2_ERR_wait_max_itt_out);
	return resp;
}

struct ps2_os_multi_error_response send_command_to_ps2_controller(enum PS2_CommandByte command) {
	struct ps2_os_multi_error_response resp = {0};

	struct ps2_os_multi_error_response err_resp = wait_till_ready_for_more_input();
	// copy any previous errors
	for (uint8_t i = 0; i < err_resp.error_list.index; i++) {
		push_error(&resp.error_list, err_resp.error_list.errors[i]);
	}

	if (err_resp.error_list.index > 0) {
		return resp; // propagate errors
	}

	__outb(PS2_COMMAND_PORT_W, (uint8_t)command);
	return resp;
}
