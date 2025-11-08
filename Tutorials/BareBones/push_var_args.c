#include "f2_string.h"
#include "push_var_args.h"
#include "vga_terminal.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

GDT_ROOT get_gdt_root(void) {

	GDT_ROOT gdt_root;
	__asm__ volatile("sgdt %0" : "=m"(gdt_root));

	return gdt_root;
}

void* memcpy(void* dest, const void* src, size_t n) {
	// Cast to unsigned char pointers for byte-wise copy
	unsigned char* d = (unsigned char*)dest;
	const unsigned char* s = (const unsigned char*)src;

	for (size_t i = 0; i < n; i++) {
		d[i] = s[i];
	}

	return dest; // mimic standard memcpy return value
}

struct realmode_address get_realmode_function_address(void (*func)(void));

// Internal function: explicit argc
uint16_t call_real_mode_function_with_argc(uint32_t argc, ...) {

	bool optional = false;
	if (optional) {
		// This is done later anyway. But might as well for now
		GDT_ROOT gdt_root = get_gdt_root();
		args16_start.gdt_root = gdt_root;

		uint32_t esp_value;
		__asm__ volatile("mov %%esp, %0" : "=r"(esp_value));
		args16_start.esp = esp_value;
	}

	va_list args;
	va_start(args, argc);

	uint32_t func = va_arg(args, uint32_t);
	struct realmode_address rm_address = get_realmode_function_address((func_ptr_t)func);
	args16_start.func = rm_address.func_address;
	args16_start.func_cs = rm_address.func_cs;

	args16_start.argc = argc - 1;

	for (uint32_t i = 0; i < argc; i++) {
		args16_start.func_args[i] = va_arg(args, uint32_t); // read promoted uint32_t
	}

	va_end(args);
	return to_pm16();
}

uint32_t min(uint32_t a, uint32_t b) {
	if (a < b) {
		return a;
	} else {
		return b;
	}
}

void print_args16(const Args16* args) {
	terminal_writestring("==== Args16 Contents ====\n");

	terminal_writestring("GDT Root:\n");
	printBinarySize(args->gdt_root.limit, "GDT Size: ", 8);
	printBinarySize((uint32_t)args->gdt_root.base, "GDT Base (bits 0-32)", 32);

	terminal_writestring("\n");

	if (args->gdt_root.base) {
		printBinarySize((uint32_t)args->gdt_root.base->lower, "Base Descriptor Lower (bits 0-32)", 32);
		printBinarySize((uint32_t)args->gdt_root.base->higher, "Base Descriptor Higher (bits 33-64)", 32);
		terminal_writestring("\n");
	}

	terminal_write_hex("ESP: ", args->esp);
	terminal_writestring("\n");

	terminal_write_uint("Ret1: ", args->ret1);
	terminal_write_uint("Ret2: ", args->ret2);
	terminal_writestring("\n");

	terminal_write_hex("Function address offset (ip): ", args->func);
	terminal_write_hex("Function address segment (cs): ", args->func_cs);
	terminal_writestring("\n");

	terminal_write_uint("Argc: ", args->argc);
	terminal_writestring("\n");

	terminal_writestring("Function Arguments:\n");
	terminal_write_uint16_array_newlines(args->func_args, "    arg", min(args->argc, 12));

	// printf("========================\n");
}

int print_args16_more(void) {

	print_args16(&args16_start);

	uint16_t args16_raw[24];
	memcpy(args16_raw, &args16_start, 48);

	terminal_writestring("\nThe raw version: \n");

	terminal_write_uint16_array_newlines(args16_raw, "args16_raw", 24);
	return 0;
}

struct realmode_address get_realmode_function_address(void (*func)(void)) {

	uint32_t addr = (uint32_t)func;

	// Real mode can only access 0..0xFFFFF (1 MB)
	if (addr > 0xFFFFF) {
		// Return an invalid CS:IP
		return (struct realmode_address){0xFFFF, 0xFFFF};
	}

	uint16_t func_address = addr & 0xFFFF;
	uint16_t cs = (addr - func_address) >> 16;

	struct realmode_address rm = {.func_address = func_address, .func_cs = cs};
	return rm;
}
