#pragma once
#include "stdio.h"
#include "vga_terminal.h"

static inline __attribute__((noreturn)) void _abort(void) {
	terminal_writestring("ABORTED\n");

	// Halt loop
	while (true) {
		__asm__ volatile("cli; hlt");
	}
}

static inline __attribute__((noreturn)) void _abort_msg(const char* message, const char* file, const char* function_name, int line) {
	kprintf("%s\nFile: %s, Function: %s, Line: %d\n", message, file, function_name, line);

	_abort();
}

// The assert handler function
static inline void assert_handler(bool condition, const char* message, const char* file, const char* function_name, int line) {
	if (!condition) {
		kprintf("Assertion Failed\n");
		_abort_msg(message, file, function_name, line);
	}
}

// The macro: automatically passes line number and file
#define assert(cond, msg) assert_handler((cond), (msg), __FILE__, __func__, __LINE__)

#define abort_msg(msg) _abort_msg(msg, __FILE__, __func__, __LINE__)
#define abort() abort_msg("abort called")
