#pragma once
#include "ppnargs.h"
#include <stdint.h>

void printf_argc(uint32_t argc, const char* fmt, ...);

// Macro wrapper: automatically counts number of arguments
#define printf(...) \
	printf_argc(PP_NARG(__VA_ARGS__), __VA_ARGS__)
