#include "string.c"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// void write_string(const char* str);
extern void init_com1(void);

uint8_t rounded_down_log(uintptr_t number, uint8_t base) {

	uint8_t ret = 0;

	while (number >= base) {
		number /= base;
		ret++;
	}

	return ret;
}

#define assert_c(cond, msg_true, msg_false) \
	do {                                    \
		if (cond) {                         \
			write_string_c(msg_true);       \
		} else {                            \
			write_string_c(msg_false);      \
		}                                   \
	} while (0)

void test_rl() {

	// write_string("Inside test roundup log\n");
	uint8_t res = rounded_down_log(11, 10);
	char* nums = "0123456789";
	write_string_c("The result is ");
	write_com1(nums[res]);
	write_string_c("\n\n");

	// Edge cases
	assert_c(rounded_down_log(0, 10) == 0, "pass 0 10 0\n", "fail 0 10 0\n");
	assert_c(rounded_down_log(1, 10) == 0, "pass 1 10 0\n", "fail 1 10 0\n");
	assert_c(rounded_down_log(9, 10) == 0, "pass 9 10 0\n", "fail 9 10 0\n");
	assert_c(rounded_down_log(10, 10) == 1, "pass 10 10 1\n", "fail 10 10 1\n");
	assert_c(rounded_down_log(11, 10) == 1, "pass 11 10 1\n", "fail 11 10 1\n");

	write_string_c("\n");

	// Powers of 2
	assert_c(rounded_down_log(1, 2) == 0, "pass 1 2 0\n", "fail 1 2 0\n");
	assert_c(rounded_down_log(2, 2) == 1, "pass 2 2 1\n", "fail 2 2 1\n");
	assert_c(rounded_down_log(3, 2) == 1, "pass 3 2 1\n", "fail 3 2 1\n");
	assert_c(rounded_down_log(4, 2) == 2, "pass 4 2 2\n", "fail 4 2 2\n");
	assert_c(rounded_down_log(15, 2) == 3, "pass 15 2 3\n", "fail 15 2 3\n");
	assert_c(rounded_down_log(16, 2) == 4, "pass 16 2 4\n", "fail 16 2 4\n");

	write_string_c("\n");
	// Random tests
	assert_c(rounded_down_log(100, 10) == 2, "pass 100 10 2\n", "fail 100 10 2\n");
	assert_c(rounded_down_log(99, 10) == 1, "pass 99 10 1\n", "fail 99 10 1\n");
	assert_c(rounded_down_log(1024, 2) == 10, "pass 1024 2 10\n", "fail 1024 2 10\n");
	assert_c(rounded_down_log(1023, 2) == 9, "pass 1023 2 9\n", "fail 1023 2 9\n");

	write_string_c("\n");
	// Edge cases
	assert_c(rounded_down_log(0, 16) == 0, "pass 0 16 0\n", "fail 0 16 0\n");
	assert_c(rounded_down_log(1, 16) == 0, "pass 1 16 0\n", "fail 1 16 0\n");
	assert_c(rounded_down_log(15, 16) == 0, "pass 15 16 0\n", "fail 15 16 0\n");
	assert_c(rounded_down_log(16, 16) == 1, "pass 16 16 1\n", "fail 16 16 1\n");
	assert_c(rounded_down_log(17, 16) == 1, "pass 17 16 1\n", "fail 17 16 1\n");

	write_string_c("\n");
	// Powers of 16
	assert_c(rounded_down_log(16, 16) == 1, "pass 16 16 1\n", "fail 16 16 1\n");
	assert_c(rounded_down_log(255, 16) == 1, "pass 255 16 1\n", "fail 255 16 1\n");
	assert_c(rounded_down_log(256, 16) == 2, "pass 256 16 2\n", "fail 256 16 2\n");
	assert_c(rounded_down_log(257, 16) == 2, "pass 257 16 2\n", "fail 257 16 2\n");
	assert_c(rounded_down_log(4095, 16) == 2, "pass 4095 16 2\n", "fail 4095 16 2\n");
	assert_c(rounded_down_log(4096, 16) == 3, "pass 4096 16 3\n", "fail 4096 16 3\n");

	write_string_c("\n");
	// Random numbers
	assert_c(rounded_down_log(1234, 16) == 2, "pass 1234 16 2\n", "fail 1234 16 2\n");
	assert_c(rounded_down_log(65535, 16) == 3, "pass 65535 16 3\n", "fail 65535 16 3\n");
	assert_c(rounded_down_log(65536, 16) == 4, "pass 65536 16 4\n", "fail 65536 16 4\n");
}

uintptr_t mpow(uint8_t base, uint8_t power) {
	uintptr_t ret = 1;
	for (uint8_t i = 0; i < power; i++) {
		ret *= base;
	}
	return ret;
}

char u64_to_str_buf[50] = {0};
char* u64_to_str(uintptr_t number) {

	for (uint8_t i = 0; i < 50; i++) {
		u64_to_str_buf[i] = 0;
	}

	if (number == 0) {
		u64_to_str_buf[0] = '0';
		return u64_to_str_buf;
	}

	uint8_t num_to_write = rounded_down_log(number, 10);

	return 0;
}

int kernel_main(void* ptr) {
	init_com1();

	write_string_c("\n\n============Sign of Life=============\n\n");
	// for (uintptr_t i = 0; i < 30000000; i++) {
	// 	__asm__ volatile("pause");
	// }
	write_string_c("\n\n============Start of kernel (64)=============\n\n");
	write_string_c("Hello\n");
	test_rl();

	write_string_c("End of kernel\n");

	return 0;
}
