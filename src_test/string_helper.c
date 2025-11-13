#include "string_helper.h"
size_t strlen(const char* str) {
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

// Returns the length of the string written.
size_t itoa(int value, char* buffer) {
	size_t pos = 0;
	int negative = 0;

	if (value == 0) {
		buffer[pos++] = '0';
		buffer[pos] = '\0';
		return pos;
	}

	if (value < 0) {
		negative = 1;
		value = -value; // careful: INT_MIN may overflow, but for small kernel code often fine
	}

	// Write digits in reverse
	size_t start = pos;
	while (value > 0) {
		buffer[pos++] = '0' + (value % 10);
		value /= 10;
	}

	if (negative) {
		buffer[pos++] = '-';
	}

	buffer[pos] = '\0';

	// Reverse the string
	size_t end = pos - 1;
	while (start < end) {
		char tmp = buffer[start];
		buffer[start] = buffer[end];
		buffer[end] = tmp;
		start++;
		end--;
	}

	return pos;
}

size_t uitoa(uint32_t value, char* buffer) {
	size_t pos = 0;

	if (value == 0) {
		buffer[pos++] = '0';
		buffer[pos] = '\0';
		return pos;
	}

	// Write digits in reverse
	size_t start = pos;
	while (value > 0) {
		buffer[pos++] = '0' + (value % 10);
		value /= 10;
	}

	buffer[pos] = '\0';

	// Reverse the string
	size_t end = pos - 1;
	while (start < end) {
		char tmp = buffer[start];
		buffer[start] = buffer[end];
		buffer[end] = tmp;
		start++;
		end--;
	}

	return pos;
}

static inline int is_digit(char c) {
	return (c >= '0' && c <= '9');
}

uint8_t char_to_uint(const char c) {
	if (!is_digit(c)) {
		return CHAR_TO_UINT_INVALID;
	}

	return c - '0';
}

uint32_t string_to_uint(const char* str) {
	uint8_t digit_index = 0;
	uint32_t power = 1;
	uint32_t sum = 0;

	while (*str) {
		uint8_t digit = char_to_uint(*str);
		if (digit == CHAR_TO_UINT_INVALID) {
			return -1;
		}
		sum += digit * power;
		power *= 10;
		str++;
	}
	return sum;
}

size_t uint_to_hex(uint32_t value, char* buffer, int uppercase) {
	const char* digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
	int i;

	// Handle zero explicitly
	if (value == 0) {
		buffer[0] = '0';
		buffer[1] = '\0';
		return 1;
	}

	char temp[9]; // Max 8 hex digits + null terminator
	temp[8] = '\0';

	// Convert each nibble
	for (i = 7; i >= 0; i--) {
		uint8_t nibble = value & 0xF;
		temp[i] = digits[nibble];
		value >>= 4;
	}

	// Copy skipping leading zeros
	i = 0;
	while (temp[i] == '0')
		i++;
	int j = 0;
	while (temp[i] != '\0') {
		buffer[j++] = temp[i++];
	}
	buffer[j] = '\0';

	return (size_t)j;
}

size_t ptr_to_hex(void* ptr, char* buffer, int uppercase) {
	const char* digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
	uintptr_t value = (uintptr_t)ptr; // cast pointer to integer
	int i;

	// Handle null pointer explicitly
	if (value == 0) {
		buffer[0] = '0';
		buffer[1] = '\0';
		return 1;
	}

	char temp[2 * sizeof(void*) + 1]; // enough for pointer in hex + null
	temp[2 * sizeof(void*)] = '\0';

	// Fill from the end
	for (i = 2 * sizeof(void*) - 1; i >= 0; i--) {
		temp[i] = digits[value & 0xF];
		value >>= 4;
	}

	// Skip leading zeros
	i = 0;
	while (temp[i] == '0')
		i++;

	int j = 0;
	while (temp[i] != '\0') {
		buffer[j++] = temp[i++];
	}
	buffer[j] = '\0';

	return (size_t)j;
}
