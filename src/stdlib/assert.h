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

static inline __attribute__((noreturn)) void _abort_msg(const char* message, int line, const char* file) {
	kprintf("%s\nFile: %s, Line: %d\n", message, file, line);
	_abort();
}

// The assert handler function
static inline void assert_handler(bool condition, const char* message, int line, const char* file) {
	if (!condition) {
		kprintf("Assertion Failed\n");
		_abort_msg(message, line, file);
	}
}

// The macro: automatically passes line number and file
#define assert(cond, msg) assert_handler((cond), (msg), __LINE__, __FILE__)

#define abort_msg(msg) _abort_msg(msg, __LINE__, __FILE__)
#define abort() abort_msg("abort called")
