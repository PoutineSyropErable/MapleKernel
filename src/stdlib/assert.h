#pragma once
#include "stdio.h"
#include "vga_terminal.h"
#include <stdarg.h>

static inline __attribute__((noreturn)) void _abort(void) {
	terminal_writestring("ABORTED\n");

	while (true) {
		__asm__ volatile("cli; hlt");
	}
}

static inline __attribute__((noreturn)) void _abort_msg_fmt(const char* file, const char* func, int line,
                                                            const char* fmt, va_list args) {
	kprintf("Assertion Failed\n");

	// Print user message (formatted)
	vkprintf(fmt, args); // <-- You MUST provide kvprintf(fmt, va_list)
	kprintf("\n");

	// Print location
	kprintf("File: %s, Function: %s, Line: %d\n", file, func, line);

	_abort();
}

static inline __attribute__((noreturn)) void abort_msg_fmt(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	_abort_msg_fmt(__FILE__, __func__, __LINE__, fmt, args);

	va_end(args);
}

static inline void assert_handler_fmt(bool cond,
                                      const char* file,
                                      const char* func,
                                      int line,
                                      const char* fmt, ...) {
	if (!cond) {
		va_list args;
		va_start(args, fmt);

		_abort_msg_fmt(file, func, line, fmt, args);

		va_end(args);
	}
}

// PUBLIC MACROS — user interface
#define assert(cond, fmt, ...) \
	assert_handler_fmt((cond), __FILE__, __func__, __LINE__, (fmt), ##__VA_ARGS__)

#define abort_msg(fmt, ...) \
	abort_msg_fmt((fmt), ##__VA_ARGS__)

#define abort() \
	abort_msg("abort() called")
