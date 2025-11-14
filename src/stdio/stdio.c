#include "f2_string.h"
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

[[maybe_unused]] static inline int is_alnum(char c) {
	return is_alpha(c) || is_digit(c);
}

static inline enum PRINTF_FMT_OPTION char_to_fmt_option(char c) {
	switch (c) {
	case ':':
		return FMT_OPTION_PAD;
	case '.':
		return FMT_OPTION_PRECISION;
	default:
		return FMT_OPTION_NONE;
	}
}

static inline enum PRINTF_TYPE char_to_printf_tag(char c) {
	switch (c) {
	case 'c':
		return PRINTF_TAG_CHAR;
	case 's':
		return PRINTF_TAG_STRING;
	case 'd':
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

const char* printf_fmt_option_to_str(enum PRINTF_FMT_OPTION opt) {
	switch (opt) {
	case FMT_OPTION_NONE:
		return "FMT_OPTION_NONE";
	case FMT_OPTION_NOT_IMPLEMENTED:
		return "FMT_OPTION_NOT_IMPLEMENTED";
	case FMT_OPTION_COULD_NOT_CONVERT:
		return "FMT_OPTION_COULD_NOT_CONVERT";
	case FMT_OPTION_PAD:
		return "FMT_OPTION_PAD";
	case FMT_OPTION_PRECISION:
		return "FMT_OPTION_PRECISION";
	default:
		return "FMT_DEFAULT";
	}
}

const char* printf_type_to_str(enum PRINTF_TYPE tag) {
	switch (tag) {
	case PRINTF_TAG_CHAR:
		return "PRINTF_TAG_CHAR";
	case PRINTF_TAG_STRING:
		return "PRINTF_TAG_STRING";
	case PRINTF_TAG_INT:
		return "PRINTF_TAG_INT";
	case PRINTF_TAG_UINT32_T:
		return "PRINTF_TAG_UINT32_T";
	case PRINTF_TAG_FLOAT:
		return "PRINTF_TAG_FLOAT";
	case PRINTF_TAG_HEX:
		return "PRINTF_TAG_HEX";
	case PRINTF_TAG_BINARY:
		return "PRINTF_TAG_BINARY";
	case PRINTF_TAG_INVALID:
		return "PRINTF_TAG_INVALID";
	default:
		return "PRINTF_TAG_UNKNOWN";
	}
}

void print_info(struct PRINTF_FIELD_PROPERTIES properties) {
	terminal_writestring("Type = ");
	terminal_writestring(printf_type_to_str(properties.type));
	terminal_writestring("\n");

	terminal_writestring("Option = ");
	terminal_writestring(printf_fmt_option_to_str(properties.option));
	terminal_writestring("\n");

	terminal_write_uint("Option num = ", properties.option_num);
	terminal_write_uint("len = ", properties.len);
	terminal_write_uint("pos = ", properties.pos);
}

struct PRINTF_FIELD_PROPERTIES get_single_format_properties(const char* fmt) {
	uint8_t size = 0;

	const char* fmt_start = fmt + 1;
	// after the %
	char fmt_type = *(fmt_start);

	struct PRINTF_FIELD_PROPERTIES properties;
	properties.type = char_to_printf_tag(fmt_type);
	char possible_option = *(fmt + 2); // "%p:"
	properties.option = char_to_fmt_option(possible_option);

	if (properties.option == FMT_OPTION_NONE) {
		properties.len = 2; // % + <option_char>. Example: len(%s) =2, len (%h) =2
		properties.option_num = 0;
		return properties;
	}

	const char* option_number_char_ptr = fmt + 3; // ("%p:2")
	if (!is_digit(*option_number_char_ptr)) {
		properties.option = FMT_OPTION_NOT_IMPLEMENTED;
	} else {
		uint8_t len = 3;
		char len_char[8] = {0};
		while (is_digit(*option_number_char_ptr)) {
			len_char[len - 3] = *option_number_char_ptr;
			option_number_char_ptr++;
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

void set_types_and_pos(const char* fmt, struct PRINTF_FIELD_PROPERTIES* printf_args_properties, uint8_t percent_count) {

	const char* char_array = fmt;
	uint8_t count = 0;
	uint16_t offset = 0; // the offset of the start of the "%f:03" (where the % start)

	bool in_type = false;

	while (char_array[offset]) {
		if (char_array[offset] == '%') {

			if (percent_count == count) {
				terminal_write_uint("percent_count = ", percent_count);
				terminal_write_uint("count = ", count);
				abort();
			}

			const char* this_format = &char_array[offset];
			struct PRINTF_FIELD_PROPERTIES properties = get_single_format_properties(this_format);
			printf_args_properties[count].len = properties.len;
			printf_args_properties[count].option = properties.option;
			printf_args_properties[count].type = properties.type;
			printf_args_properties[count].option_num = properties.option_num;
			printf_args_properties[count].pos = offset;
			count++;
		}
		if (in_type) {
		}
		offset++;
	}
}

static inline void nop() {}

struct startAndEnd {
	uint16_t start;
	uint16_t end;
};

/* end not included */
void terminal_write_offsets(const char* str, uint16_t start, uint16_t end) {
	if (end <= start)
		return; // sanity check

	for (uint16_t i = start; i < end; i++) {
		char c = str[i];
		terminal_putchar(str[i]);
	}
}

void kprintf_argc(uint32_t argc, const char* fmt, ...) {

	uint8_t len = argc - 1;
	// we don't count the format

	uint8_t len_check = count_char(fmt, '%');

	if (len != len_check) {
		terminal_writestring("len check failed\n");
		terminal_write_uint("len = ", len);
		terminal_write_uint("len_check = ", len_check);
		abort();
	}
	struct PRINTF_FIELD_PROPERTIES printf_information[len];
	set_types_and_pos(fmt, (struct PRINTF_FIELD_PROPERTIES*)&printf_information, len);

	uint16_t format_len = strlen(fmt);
	uint16_t last_fmt_end = printf_information[len - 1].pos + printf_information[len - 1].len;
	if (format_len == last_fmt_end) {
		// Then,  "abc %d %f" <- Finish with format specifier
		// Else, "abc, %d %f\n" <- something else after
	}

	struct startAndEnd start_and_ends[argc];
	start_and_ends[0].start = 0;
	start_and_ends[0].end = printf_information[0].pos - 1;

	for (uint8_t i = 1; i < len; i++) {
		start_and_ends[i].start = printf_information[i - 1].pos + printf_information[i - 1].len;
		start_and_ends[i].end = printf_information[i].pos - 1;
	}

	if (len != 0) {
		start_and_ends[len].start = printf_information[len - 1].pos + printf_information[len - 1].len;
		start_and_ends[len].end = format_len - 1;
	}

// #define TEST_PRINTF
#ifdef TEST_PRINTF
	terminal_writestring("\n==Start of info===\n");
	for (uint8_t i = 0; i < len; i++) {
		print_info(printf_information[i]);
		terminal_writestring("--------\n");
	}
	terminal_writestring("\n===End of info==\n");

	for (uint8_t i = 0; i < argc; i++) {
		terminal_write_uint("i = ", i);
		terminal_write_uint("start = ", start_and_ends[i].start);
		terminal_write_uint("end = ", start_and_ends[i].end);
		terminal_putchar('|');
		terminal_write_offsets(fmt, start_and_ends[i].start, start_and_ends[i].end + 1);
		terminal_putchar('|');
		terminal_writestring("\n======\n");
	}
#endif

	// terminal_writestring("\n==Showing the real text==\n");

	terminal_write_offsets(fmt, start_and_ends[0].start, start_and_ends[0].end + 1);

	va_list args;
	va_start(args, fmt);

	for (uint8_t i = 0; i < len; i++) {

		switch (printf_information[i].type) {
		case PRINTF_TAG_CHAR: {
			char arg_i = (char)va_arg(args, int); // read promoted int, cast to char
			// terminal_writestring("char: ");
			terminal_putchar(arg_i);
			// terminal_writestring("\n");
			break;
		}

		case PRINTF_TAG_STRING: {
			const char* arg_i = va_arg(args, const char*); // pointer stays pointer
			// terminal_writestring("string: ");
			terminal_writestring(arg_i);
			// terminal_writestring("\n");
			break;
		}

		case PRINTF_TAG_INT: {
			int arg_i = va_arg(args, int); // int is passed as int
			// terminal_writestring("int: ");
			print_int_var_no_newline(arg_i);
			// terminal_writestring("\n");
			break;
		}

		case PRINTF_TAG_UINT32_T: {
			uint32_t arg_i = (uint32_t)va_arg(args, uint32_t); // uint32_t promoted to unsigned int

			print_uint_var_no_newline(arg_i);
			break;
		}

		case PRINTF_TAG_FLOAT: {
			float arg_i = (float)va_arg(args, double); // float promoted to double
			struct PRINTF_FIELD_PROPERTIES information = printf_information[i];

			if (information.option == FMT_OPTION_PRECISION) {
				// terminal_writestring("float: ");
				print_float_var_no_newline_precision(arg_i, information.option_num);
				// terminal_writestring("\n");
			} else {
				// terminal_writestring("float: ");
				print_float_var_no_newline(arg_i);
				// terminal_writestring("\n");
			}
			break;
		}

		case PRINTF_TAG_HEX: {
			uint32_t arg_i = (uint32_t)va_arg(args, unsigned int); // same as UINT32_T
			struct PRINTF_FIELD_PROPERTIES information = printf_information[i];
			if (information.option == FMT_OPTION_PAD) {
				print_hex_f(arg_i, information.option_num);
			} else {
				print_hex_f(arg_i, 0);
			}
			break;
		}

		case PRINTF_TAG_BINARY: {
			uint32_t arg_i = (uint32_t)va_arg(args, uint32_t); // same as UINT32_T
			struct PRINTF_FIELD_PROPERTIES information = printf_information[i];
			if (information.option == FMT_OPTION_PAD) {
				// terminal_writestring("binary_padded:");
				print_binary_var_no_newline(arg_i, information.option_num);
				// terminal_writestring("\n");
			} else {

				// terminal_writestring("binary:");
				print_binary_var_no_newline(arg_i, 0);
				// terminal_writestring("\n");
			}

			break;
		}

		case PRINTF_TAG_INVALID:
		default: {
			nop(); // invalid behavior
			break;
		}
		}

		terminal_write_offsets(fmt, start_and_ends[i + 1].start, start_and_ends[i + 1].end + 1);
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
