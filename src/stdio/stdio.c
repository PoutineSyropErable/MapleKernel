#include "stdio.h"
#include "string_helper.h"
#include "vga_terminal.h"
#include <stdarg.h>

/*
%f : float
%d : int
%u : uint32_t
%c : char
%s : string
%p : (uint32_t) -> hex
%b : (uint32_t) -> binary


%p:4 (pad to show >=4 hex)
%f.4 (show at least 4 digits of precision)


*/

uint8_t count_char(const char* str, char delimiter);
uint8_t get_single_format_size(const char* str);

static inline int is_alpha(char c) {
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

static inline int is_digit(char c) {
	return (c >= '0' && c <= '9');
}

static inline int is_alnum(char c) {
	return is_alpha(c) || is_digit(c);
}

enum FMT_OPTION {
	FMT_OPTION_NONE = 0,
	FMT_OPTION_NOT_IMPLEMENTED = 1,
	FMT_OPTION_COULD_NOT_CONVERT = 2,
	FMT_OPTION_PAD = ':',
	FMT_OPTION_PRECISION = '.'
};

enum FMT_OPTION char_to_fmt_option(char c) {
	switch (c) {
	case ':':
		return FMT_OPTION_PAD;
	case '.':
		return FMT_OPTION_PRECISION;
	default:
		return FMT_OPTION_NONE;
	}
}

enum PRINTF_TAG {
	PRINTF_TAG_INVALID = 0,
	PRINTF_TAG_CHAR = 'c',
	PRINTF_TAG_STRING = 's',
	PRINTF_TAG_INT = 'i',
	PRINTF_TAG_UINT32_T = 'u',
	PRINTF_TAG_FLOAT = 'f',
	PRINTF_TAG_HEX = 'h',
	PRINTF_TAG_BINARY = 'b',
};

enum PRINTF_TAG char_to_printf_tag(char c) {
	switch (c) {
	case 'c':
		return PRINTF_TAG_CHAR;
	case 's':
		return PRINTF_TAG_STRING;
	case 'i':
		return PRINTF_TAG_INT;
	case 'u':
		return PRINTF_TAG_UINT32_T;
	case 'f':
		return PRINTF_TAG_FLOAT;
	case 'h':
		return PRINTF_TAG_HEX;
	case 'b':
		return PRINTF_TAG_BINARY;
	default:
		return PRINTF_TAG_INVALID;
	}
}

struct PRINTF_POS_TAG {
	enum PRINTF_TAG type;
	enum FMT_OPTION option;
	uint16_t pos;
	uint8_t len;
	uint8_t option_num;
};

struct PRINTF_POS_TAG get_single_format_properties(const char* fmt) {
	uint8_t size = 0;

	const char* fmt_start = fmt + 1;
	// after the %
	char fmt_type = *(fmt_start);

	struct PRINTF_POS_TAG properties;
	properties.type = char_to_printf_tag(fmt_type);
	char possible_option = *(fmt + 2);
	properties.option = char_to_fmt_option(possible_option);

	if (properties.option == FMT_OPTION_NONE) {
		properties.len = 2; // % + <option_char>. Example: len(%s) =2, len (%h) =2
	}

	const char* option_number = fmt + 3; // ("%p:2")
	if (!is_digit(*option_number)) {
		properties.option = FMT_OPTION_NOT_IMPLEMENTED;
	} else {
		uint8_t len = 3;
		char len_char[8] = {0};
		while (is_digit(*option_number)) {
			len_char[len - 3] = *option_number;
			option_number++;
			len++;
		}
		properties.len = len;
		len_char[len - 3] = '\0';
		uint32_t len32 = string_to_uint(len_char);
		if (len32 == (uint32_t)-1) {
			properties.option = FMT_OPTION_COULD_NOT_CONVERT;
		}
		uint8_t len8 = (uint8_t)(len32 & 0xFF);
		properties.option_num = len8;
	}

	return properties;
}

void set_types_and_pos(const char* fmt, struct PRINTF_POS_TAG* out, uint8_t percent_count) {

	const char* char_array = fmt;
	uint8_t count = 0;
	uint16_t offset = 0; // the offset of the start of the "%f:03" (where the % start)

	bool in_type = false;

	while (char_array[offset]) {
		if (char_array[offset] == '%') {

			const char* this_format = &char_array[offset];
			struct PRINTF_POS_TAG properties = get_single_format_properties(this_format);
			out[count].len = properties.len;
			out[count].option = properties.option;
			out[count].type = properties.type;
			out[count].pos = offset;
			count++;
		}
		if (in_type) {
		}
		offset++;
	}
}

void printf_argc(uint32_t argc, const char* fmt, ...) {

	uint8_t len = argc - 1;
	// we don't count the format

	uint8_t len_check = count_char(fmt, '%');

	if (len != len_check) {
		terminal_writestring("printf failed. arg count is different\n");
		terminal_write_uint("len = ", len);
		terminal_write_uint("len_check = ", len_check);
	}
	struct PRINTF_POS_TAG types_and_positions[len];
	set_types_and_pos(fmt, (struct PRINTF_POS_TAG*)&types_and_positions, len);

	va_list args;
	va_start(args, argc - 1);

	for (uint8_t i = 0; i < len; i++) {
		int arg_i = va_arg(args, int); // read promoted uint32_t
		print_int_var(arg_i);
	}

	va_end(args);
}

uint8_t count_char(const char* str, char delimiter) {
	const char* char_ptr = str;
	uint8_t count = 0;
	while (*char_ptr) {
		if (*char_ptr == delimiter) {
			count++;
		}
		char_ptr++;
	}
	return count;
}
