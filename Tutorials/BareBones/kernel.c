#include "add16_wrapper.h"
#include "address_getter.c"
#include "kernel.h"
#include "os_registers.c"
#include "string_helper.h"
#include "vga_terminal.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static inline void wait(float seconds) {

	volatile unsigned long count;
	const unsigned long loops_per_sec = 150000000UL; // tuned for ~1s per unit

	// total loops = seconds * loops_per_sec
	unsigned long total_loops = (unsigned long)(seconds * loops_per_sec);

	for (unsigned long i = 0; i < total_loops; i++) {
		__asm__ volatile("nop");
	}
}

void print_array_terminal(TerminalContext* term, int* arr, size_t n) {
	char buf[12];
	terminal_writestring(term, "["); // start bracket

	for (size_t i = 0; i < n; i++) {
		itoa(arr[i], buf);
		terminal_writestring(term, buf);

		if (i != n - 1) {
			terminal_writestring(term, ", ");
		}

		// Insert newline every 10 elements to handle scrolling
		if ((i + 1) % 10 == 0) {
			terminal_writestring(term, "\n");
			wait(0.1);
		}
	}

	terminal_writestring(term, "]\n"); // end bracket with newline
}

void print_int_var(TerminalContext* term, int var) {

	char res_buff[12];
	size_t len = itoa(var, res_buff);
	res_buff[len] = '\n';     // replace the null terminator with newline
	res_buff[len + 1] = '\0'; // add new null terminator
	terminal_writestring(term, res_buff);
}

void print_hex_var(TerminalContext* term, int var) {

	char res_buff[12];
	size_t len = int_to_hex(var, res_buff, 1);
	res_buff[len] = '\n';     // replace the null terminator with newline
	res_buff[len + 1] = '\0'; // add new null terminator
	terminal_writestring(term, res_buff);
}

void print_hex_ptr(TerminalContext* term, void* ptr) {

	char res_buff[12];
	size_t len = ptr_to_hex(ptr, res_buff, 1);
	res_buff[len] = '\n';     // replace the null terminator with newline
	res_buff[len + 1] = '\0'; // add new null terminator
	terminal_writestring(term, res_buff);
}

void print_uint_var(TerminalContext* term, uint32_t var) {

	char res_buff[12];
	size_t len = uitoa(var, res_buff);
	res_buff[len] = '\n';     // replace the null terminator with newline
	res_buff[len + 1] = '\0'; // add new null terminator
	terminal_writestring(term, res_buff);
}

void print_something(TerminalContext* term) {

	char buf[12];
	for (int i = 0; i < 10; i++) {
		size_t len = itoa(i, buf);
		buf[len] = '\n';     // replace the null terminator with newline
		buf[len + 1] = '\0'; // add new null terminator
		terminal_writestring(term, buf);
		wait(0.1);
	}
}

int* print_extern_address(TerminalContext* term, char* str, int* func()) {

	terminal_writestring(term, str);
	int* address_value = func();
	print_hex_var(term, (int)address_value);
	terminal_writestring(term, "\n");
	return address_value;
}

uint32_t read32(uint32_t addr) {
	return *(volatile uint32_t*)addr;
}

void terminal_write_uint(TerminalContext* term, char* str, uint32_t val) {

	terminal_writestring(term, str);
	print_uint_var(term, (int)val);
}

void terminal_write_hex(TerminalContext* term, char* str, uint32_t val) {

	terminal_writestring(term, str);
	print_hex_var(term, (int)val);
}

void terminal_write_ptr(TerminalContext* term, char* str, void* val) {

	terminal_writestring(term, str);
	print_hex_ptr(term, val);
}

void print_extern_address16(TerminalContext* term, char* str, uint16_t func()) {

	terminal_writestring(term, str);
	uint16_t address_value = func();
	print_int_var(term, (int)address_value);
	// terminal_writestring(term, "\n");
}

void before(TerminalContext* term) {
	terminal_writestring(term, "Before the main execution");
}

void kernel_main(void) {

	// init_paging();
	// init_page_bitmap();

	/* Initialize terminal interface */
	initialize_terminal(&term);
	terminal_set_scroll(&term, 0);

	terminal_writestring(&term, "\n\n===== Start of Kernel=====\n\n");

	print_extern_address(&term, "The address of stack16_start: ", get_stack16_start_address);
	print_extern_address(&term, "The address of stack16_end: ", get_stack16_end_address);
	print_extern_address(&term, "The address of args16_start: ", get_args16_start_address);
	print_extern_address(&term, "The address of args16_end: ", get_args16_end_address);

	int* call_add16_address = print_extern_address(&term, "The address of call_add16: ", get_call_add16_address);
	int* add1616_address = print_extern_address(&term, "The address of add1616: ", get_add1616_start_address);
	int* resume32_address = print_extern_address(&term, "The address of resume32: ", get_resume32_start_address);
	int* resume32_end_address = print_extern_address(&term, "The address of resume32_end: ", get_resume32_end_address);

	print_extern_address(&term, "The address of misc32_s1: ", get_misc32_s1_address);
	print_extern_address(&term, "The address of misc32_s2: ", get_misc32_s2_address);

	// terminal_writestring(&term, "\n");
	// terminal_writestring(&term, "The value of the code at 0xb040: \n");
	// for (int i = 0; i < 50; i++) {
	// 	print_hex_var(&term, add1616_address[i]);
	// }
	//
	// terminal_writestring(&term, "\n");
	// terminal_writestring(&term, "The value of the code at 0xB0A8: \n");
	// for (int i = 0; i < 50; i++) {
	// 	print_hex_var(&term, resume32_address[i]);
	// }

	print_extern_address16(&term, "\nThe value of cs: ", get_cs_selector);
	print_extern_address16(&term, "\nThe value of ss: ", get_ss_selector);
	print_extern_address16(&term, "\nThe value of ds: ", get_ds_selector);
	print_extern_address16(&term, "\nThe value of es: ", get_fs_selector);
	print_extern_address16(&term, "\nThe value of fs: ", get_fs_selector);
	print_extern_address16(&term, "\nThe value of gs: ", get_gs_selector);

	GDT_ROOT gdt_descriptor = get_gdt_root();

	// print_hex_var(&term, (int)g16);
	terminal_write_hex(&term, "gdt base address = ", (uint32_t)gdt_descriptor.base);
	terminal_write_uint(&term, "gdt size limit = ", gdt_descriptor.limit);

	terminal_write_ptr(&term, "GDT16_DESCRIPTOR = ", (void*)GDT16_ROOT);
	terminal_write_ptr(&term, "GDT16 = ", GDT16_DESCRIPTOR.base);

	GDT_ENTRY* gdt = gdt_descriptor.base;
	GDT_ENTRY* gdt16 = GDT16_DESCRIPTOR.base;
	uint16_t limit16 = GDT16_DESCRIPTOR.limit;
	terminal_writestring(&term, "\nThe gdtr values:\n");
	terminal_write_hex(&term, "gdt16[0].low = ", gdt16[0].low);
	terminal_write_hex(&term, "gdt16[0].high = ", gdt16[0].high);
	terminal_write_hex(&term, "gdt16[1].low = ", gdt16[1].low);
	terminal_write_hex(&term, "gdt16[1].high = ", gdt16[1].high);
	terminal_write_hex(&term, "gdt16[2].low = ", gdt16[2].low);
	terminal_write_hex(&term, "gdt16[2].high = ", gdt16[2].high);
	terminal_write_hex(&term, "gdt16[3].low = ", gdt16[3].low);
	terminal_write_hex(&term, "gdt16[3].high = ", gdt16[3].high);

	before(&term);
	uint16_t result = 0;
	result = call_add16(25, 56);
	terminal_writestring(&term, "The result of add16: ");
	print_int_var(&term, result);
	wait(25);

	return;

	// int* big_array = (int*)kmalloc(1024 * 1024 * 1024);
	// for (int i = 0; i < 100000; i++) {
	// 	big_array[i] = i;
	// }
	//
	// print_array_terminal(&term, big_array, 100000);
}
