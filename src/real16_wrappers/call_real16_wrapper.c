#include "bit_hex_string.h"
#include "call_real16_wrapper.h"
#include "stdlib.h"
#include "string.h"
#include "vga_terminal.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// =============== Start of mics

uint32_t min(uint32_t a, uint32_t b)
{
	if (a < b)
	{
		return a;
	}
	else
	{
		return b;
	}
}

void print_args16(const Args16 *args)
{
	terminal_writestring("==== Args16 Contents ====\n");

	terminal_writestring("GDT Root:\n");
	printBinarySize(args->gdt_root.limit, "GDT Size: ", 8);
	printBinarySize((uint32_t)args->gdt_root.base_address, "GDT Base (bits 0-32)", 32);

	terminal_writestring("\n");

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
	if (args->argc == 0)
	{
		terminal_writestring("\tNone\n");
	}
	else
	{
		terminal_write_uint16_array_newlines(args->func_args, "    arg", min(args->argc, 12));
	}
	return;
}

int print_args16_more(void)
{

	print_args16(&args16_start);

	uint16_t args16_raw[24];
	memcpy(args16_raw, &args16_start, 48);

	terminal_writestring("\nThe raw version: \n");

	terminal_write_uint16_array_newlines(args16_raw, "args16_raw", 24);
	return 0;
}

struct realmode_address get_realmode_function_address(void (*func)(void))
{

	uint32_t addr = (uint32_t)func;

	// Real mode can only access 0..0xFFFFF (1 MB)
	if (addr > 0xFFFFF)
	{
		// Return an invalid CS:IP
		return (struct realmode_address){0xFFFF, 0xFFFF};
	}

	uint16_t func_address = addr & 0xFFFF;
	uint16_t cs			  = (addr - func_address) >> 16;

	struct realmode_address rm = {.func_address = func_address, .func_cs = cs};
	return rm;
}

// ==============================  End of misc

// Internal function: explicit argc
uint16_t call_real_mode_function_with_argc(uint32_t argc, ...)
{

	va_list args;
	va_start(args, argc);

	uint32_t				func	   = va_arg(args, uint32_t);
	struct realmode_address rm_address = get_realmode_function_address((function_t *)func);
	args16_start.func				   = rm_address.func_address;
	args16_start.func_cs			   = rm_address.func_cs;

	uint8_t	 *raw_args16 = (uint8_t *)&args16_start;
	uint32_t *val		 = (uint32_t *)&raw_args16[20];

	uint8_t *start = (uint8_t *)&args16_start;
	uint8_t *end   = (uint8_t *)&args16_start.func;
	uint8_t	 diff  = end - start;
	kprintf("function offset %h\n", *val);
	kprintf("function offset (func field): %h\n", args16_start.func);
	kprintf("Diff = %u\n", diff);

	args16_start.argc = argc - 1;

	for (uint32_t i = 0; i < argc; i++)
	{
		args16_start.func_args[i] = va_arg(args, uint32_t); // read promoted uint32_t
	}

	va_end(args);
	return pm32_to_pm16();
}
