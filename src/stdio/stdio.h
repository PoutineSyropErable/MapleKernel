#pragma once
#include "ppnargs.h"
#include <stdbool.h>
#include <stdint.h>

void kprintf_argc(uint32_t argc, const char* fmt, ...);

// Macro wrapper: automatically counts number of arguments
#define kprintf(...) \
	kprintf_argc(PP_NARG(__VA_ARGS__), __VA_ARGS__)

enum PRINTF_FMT_OPTION {
	FMT_OPTION_NONE = 0,
	FMT_OPTION_NOT_IMPLEMENTED = 1,
	FMT_OPTION_COULD_NOT_CONVERT = 2,
	FMT_OPTION_PAD = ':',
	FMT_OPTION_PRECISION = '.'
};

enum PRINTF_TYPE {
	PRINTF_TAG_INVALID = 0,
	PRINTF_TAG_CHAR = 'c',
	PRINTF_TAG_STRING = 's',
	PRINTF_TAG_INT = 'd',
	PRINTF_TAG_UINT32_T = 'u',
	PRINTF_TAG_FLOAT = 'f',
	PRINTF_TAG_HEX = 'h',
	PRINTF_TAG_BINARY = 'b',
};

struct PRINTF_FIELD_PROPERTIES {
	enum PRINTF_TYPE type;
	enum PRINTF_FMT_OPTION option;
	uint8_t option_num;
	uint8_t len;
	uint16_t pos;
	/*
	"%s:056"
	The option num is uint8_t 56;
	the len is 7
	the pos is the offset of % in the bigger array.
	it's a string
	and it uses padding
	*/
};

// Function to print a string to your terminal
extern void terminal_writestring(const char* str);
