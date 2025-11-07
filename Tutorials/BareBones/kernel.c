#include "add16_wrapper.h"
#include "address_getter.c"
#include "f2_string.h"
#include "kernel.h"
#include "os_registers.c"
#include "pit_timer.h"
#include "string_helper.h"
#include "vga_terminal.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void gdt_analize(GDT_ENTRY* gdt, size_t index) {

	terminal_write_uint_no_newline("\n===== Analize of GDT[", index);
	terminal_writestring("] =====\n\n");
	SegmentDescriptor* sd = &gdt[index];

	if (false) {
		setType(sd, 0b100);
		setDescriptorTypeS(sd, 0b1);
		setPriviledgeDPL(sd, 0b11); // system services. ring 2
		setPresent(sd, 0b0);
		setSegmentLimit(sd, 0xba2ed);
		setAVL(sd, 0);
		setLongMode(sd, 1);
		setDefaultOperationSize(sd, 0);
		setGranularity(sd, 0);
		setBaseAddress(sd, 0xfedc1234);
	}

	printBinary(sd->higher, "higher");
	printBinary(sd->lower, "lower");

	terminal_writestring("----- Parsed Fields -----\n");
	uint32_t baseAddress = getBaseAddress(sd);
	printBinary(baseAddress, "Base Address");
	printBinarySize(getSegmentLimit(sd), "SegmentLimit (bits 0-19)", 20);
	terminal_write_uint("Granularity = ", getGranularity(sd));
	terminal_write_uint("DefaultOperationSize = ", getDefaultOperationSize(sd));
	terminal_write_uint("LongMode = ", getLongMode(sd));

	printBinarySize(getPriviledgeDPL(sd), "DPL (bits 13-14)", 2);
	terminal_write_uint("Present = ", getPresent(sd));
	printBinarySize(getType(sd), "Type (bits 8-11)", 4);
	terminal_write_uint("DescriptorTypeS = ", getDescriptorTypeS(sd));
	terminal_write_uint("AVL = ", getAVL(sd));

	terminal_writestring("\n===== End of Analize of GDT=====\n\n");
}

void before() {
	terminal_writestring("Before the main execution\n");
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
	int* protected16_address = print_extern_address("The address of protected16: ", get_protected16_address);
	int* add1616_address = print_extern_address("The address of add1616: ", get_add1616_start_address);
	int* add16_address = print_extern_address("The address of add16: ", get_add16_address);
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
	GDT_ENTRY* gdt = gdt_descriptor.base;
	terminal_write_ptr("gdt base address = ", gdt_descriptor.base);
	terminal_write_uint("gdt size limit = ", gdt_descriptor.limit);

	GDT_ENTRY* gdt16 = GDT16_DESCRIPTOR.base;
	uint16_t limit16 = GDT16_DESCRIPTOR.limit;
	terminal_write_ptr("GDT16_DESCRIPTOR = ", GDT16_ROOT);
	terminal_write_ptr("GDT16 = ", GDT16_DESCRIPTOR.base);

	terminal_writestring("\nThe gdtr values:\n");
	terminal_write_hex("gdt16[0].lower = ", gdt16[0].lower);
	terminal_write_hex("gdt16[0].higher = ", gdt16[0].higher);
	terminal_write_hex("gdt16[1].lower = ", gdt16[1].lower);
	terminal_write_hex("gdt16[1].higher = ", gdt16[1].higher);
	terminal_write_hex("gdt16[2].lower = ", gdt16[2].lower);
	terminal_write_hex("gdt16[2].higher = ", gdt16[2].higher);
	terminal_write_hex("gdt16[3].lower = ", gdt16[3].lower);
	terminal_write_hex("gdt16[3].higher = ", gdt16[3].higher);

	gdt_analize(gdt16, 2);

	before();
	uint16_t result = 0;
	result = call_add16(25, 56); // This pass through 16 bit real mode. Goes to the depths of hell and come back
	terminal_writestring("The result of add16: ");
	print_int_var(result);

	result = call_add16(42, 69);
	terminal_writestring("The result of add16: ");
	print_int_var(result);

	terminal_writestring("\n\n===== End of Kernel=====\n\n");

	return;

	// int* big_array = (int*)kmalloc(1024 * 1024 * 1024);
	// for (int i = 0; i < 100000; i++) {
	// 	big_array[i] = i;
	// }
	//
	// print_array_terminal(&term, big_array, 100000);
}
