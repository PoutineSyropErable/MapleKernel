#include "add16_wrapper.h"
#include "address_getter.c"
#include "kernel.h"
#include "os_registers.c"
#include "string_helper.h"
#include "vga_terminal.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void before() {
	terminal_writestring("Before the main execution");
}

void kernel_main(void) {

	// init_paging();
	// init_page_bitmap();

	/* Initialize terminal interface */
	initialize_terminal();
	terminal_set_scroll(0);

	terminal_writestring("\n\n===== Start of Kernel=====\n\n");

	print_extern_address("The address of stack16_start: ", get_stack16_start_address);
	print_extern_address("The address of stack16_end: ", get_stack16_end_address);
	print_extern_address("The address of args16_start: ", get_args16_start_address);
	print_extern_address("The address of args16_end: ", get_args16_end_address);

	int* call_add16_address = print_extern_address("The address of call_add16: ", get_call_add16_address);
	int* add1616_address = print_extern_address("The address of add1616: ", get_add1616_start_address);
	int* resume32_address = print_extern_address("The address of resume32: ", get_resume32_start_address);
	int* resume32_end_address = print_extern_address("The address of resume32_end: ", get_resume32_end_address);

	print_extern_address("The address of misc32_s1: ", get_misc32_s1_address);
	print_extern_address("The address of misc32_s2: ", get_misc32_s2_address);

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

	print_extern_address16("\nThe value of cs: ", get_cs_selector);
	print_extern_address16("\nThe value of ss: ", get_ss_selector);
	print_extern_address16("\nThe value of ds: ", get_ds_selector);
	print_extern_address16("\nThe value of es: ", get_fs_selector);
	print_extern_address16("\nThe value of fs: ", get_fs_selector);
	print_extern_address16("\nThe value of gs: ", get_gs_selector);

	GDT_ROOT gdt_descriptor = get_gdt_root();

	// print_hex_var(&term, (int)g16);
	terminal_write_hex("gdt base address = ", (uint32_t)gdt_descriptor.base);
	terminal_write_uint("gdt size limit = ", gdt_descriptor.limit);

	terminal_write_ptr("GDT16_DESCRIPTOR = ", (void*)GDT16_ROOT);
	terminal_write_ptr("GDT16 = ", GDT16_DESCRIPTOR.base);

	GDT_ENTRY* gdt = gdt_descriptor.base;
	GDT_ENTRY* gdt16 = GDT16_DESCRIPTOR.base;
	uint16_t limit16 = GDT16_DESCRIPTOR.limit;
	terminal_writestring("\nThe gdtr values:\n");
	terminal_write_hex("gdt16[0].low = ", gdt16[0].low);
	terminal_write_hex("gdt16[0].high = ", gdt16[0].high);
	terminal_write_hex("gdt16[1].low = ", gdt16[1].low);
	terminal_write_hex("gdt16[1].high = ", gdt16[1].high);
	terminal_write_hex("gdt16[2].low = ", gdt16[2].low);
	terminal_write_hex("gdt16[2].high = ", gdt16[2].high);
	terminal_write_hex("gdt16[3].low = ", gdt16[3].low);
	terminal_write_hex("gdt16[3].high = ", gdt16[3].high);

	before();
	uint16_t result = 0;
	result = call_add16(25, 56);
	terminal_writestring("The result of add16: ");
	print_int_var(result);
	wait(25);

	return;

	// int* big_array = (int*)kmalloc(1024 * 1024 * 1024);
	// for (int i = 0; i < 100000; i++) {
	// 	big_array[i] = i;
	// }
	//
	// print_array_terminal(&term, big_array, 100000);
}
