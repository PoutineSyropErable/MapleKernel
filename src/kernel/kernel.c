#include "address_getter.h"
#include "bit_hex_string.h"
#include "call_real16_wrapper.h"
#include "f3_segment_descriptor_internals.h"
#include "idt.h"
#include "idt_test.h"
#include "kernel.h"
#include "os_registers.c"
#include "pit_timer.h"
#include "stdio.h"
#include "string_helper.h"
#include "vga_terminal.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

GDT_ROOT* GDT16_ROOT = &GDT16_DESCRIPTOR;

void gdt_analize(GDT_ENTRY* gdt, size_t index) {

	// terminal_write_uint_no_newline("\n===== Analize of GDT[", index);
	// terminal_writestring("] =====\n\n");
	SegmentDescriptor* sd = &gdt[index];

#ifdef SET_TEST
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
#endif

	printBinary(sd->higher, "higher");
	printBinary(sd->lower, "lower");

	terminal_writestring("----- Parsed Fields -----\n");
	uint32_t baseAddress = getBaseAddress(sd);
	printBinary(baseAddress, "Base Address");
	printBinarySize(getSegmentLimit(sd), "SegmentLimit (bits 0-19)", 20);

	terminal_writestring("\n");
	printFlags(sd);
	printAccess(sd);
	terminal_writestring("\n");

	terminal_write_uint("Granularity = ", getGranularity(sd));
	terminal_write_uint("DefaultOperationSize = ", getDefaultOperationSize(sd));
	terminal_write_uint("LongMode = ", getLongMode(sd));
	terminal_write_uint("AVL = ", getAVL(sd));

	terminal_writestring("\n");
	terminal_write_uint("Present = ", getPresent(sd));
	printBinarySize(getPriviledgeDPL(sd), "DPL (bits 13-14)", 2);
	terminal_write_uint("DescriptorTypeS = ", getDescriptorTypeS(sd));
	printBinarySize(getType(sd), "Type (bits 8-11)", 4);

	// terminal_writestring("\n===== End of Analize of GDT=====\n\n");
}

struct CodeAddressesToAnalyse {
	int* pm32_to_pm16_address;
	int* pm16_to_real16_address;
	int* call_real16_function_address;
	int* add16_address;
	int* resume32_address;
	int* resume32_end_address;
};

void analyse_code(struct CodeAddressesToAnalyse code) {

	terminal_writestring("\n");
	terminal_writestring("The value of code at pm16_to_real16_address: \n");
	for (int i = 0; i < 50; i++) {
		print_hex_var(code.pm32_to_pm16_address[i]);
	}

	terminal_writestring("\n");
	terminal_writestring("The value of code at pm16_to_real16_address: \n");
	for (int i = 0; i < 50; i++) {
		print_hex_var(code.pm16_to_real16_address[i]);
	}

	terminal_writestring("\n");
	terminal_writestring("The value of the code at call_real16_function_address: \n");
	for (int i = 0; i < 50; i++) {
		print_hex_var(code.call_real16_function_address[i]);
	}

	terminal_writestring("\n");
	terminal_writestring("The value of the code at 0xB0A8: \n");
	for (int i = 0; i < 50; i++) {
		print_hex_var(code.resume32_address[i]);
	}
}

void before() {
	terminal_writestring("Before the main execution\n");
}

void kernel_test() {

	terminal_writestring("\n\n===== Start of Kernel Test=====\n\n");

	kprintf("int = %d, uint = %u, float = %f.2, bin = %b:7, hex = %h:9-final text|\n\n", 1, 2, 3.0, 0b11011, 0x123bcd);

	print_extern_address("The address of stack16_start: ", get_stack16_start_address);
	print_extern_address("The address of stack16_end: ", get_stack16_end_address);
	print_extern_address("The address of args16_start: ", get_args16_start_address);
	print_extern_address("The address of args16_end: ", get_args16_end_address);

	print_extern_address("The address of the nice wrapper (>2MB): ", (address_getter_function_t*)get_call_realmode_func_with_args_address);
	[[maybe_unused]] int* pm32_to_pm16_address = print_extern_address("The address of pm32_to_pm16: ", (address_getter_function_t*)get_pm32_to_pm16_address);
	[[maybe_unused]] int* pm16_to_real16_address = print_extern_address("The address of pm16_to_real16: ", (address_getter_function_t*)get_pm16_to_real16_address);
	[[maybe_unused]] int* call_real16_function_address = print_extern_address("The address of call_real16_function_address: ", (address_getter_function_t*)get_call_real16_function_address);
	[[maybe_unused]] int* add16_address = print_extern_address("The address of add16: ", (address_getter_function_t*)get_add16_address);
	[[maybe_unused]] int* resume32_address = print_extern_address("The address of resume32: ", (address_getter_function_t*)get_resume32_start_address);
	[[maybe_unused]] int* resume32_end_address = print_extern_address("The address of resume32_end: ", get_resume32_end_address);

// #define ANALIZE_CODE
#ifdef ANALIZE_CODE
	analyse_code((struct CodeAddressesToAnalyse){
	    .pm32_to_pm16_address = pm32_to_pm16_address,
	    .pm16_to_real16_address = pm16_to_real16_address,
	    .call_real16_function_address = call_real16_function_address,
	    .add16_address = add16_address,
	    .resume32_address = resume32_address,
	    .resume32_end_address = resume32_end_address});
#endif

	print_extern_address("The address of misc32_s1: ", get_misc32_s1_address);
	print_extern_address("The address of misc32_s2: ", get_misc32_s2_address);

	print_extern_address16("\nThe value of cs: ", get_cs_selector);
	print_extern_address16("\nThe value of ss: ", get_ss_selector);
	print_extern_address16("\nThe value of ds: ", get_ds_selector);
	print_extern_address16("\nThe value of es: ", get_fs_selector);
	print_extern_address16("\nThe value of fs: ", get_fs_selector);
	print_extern_address16("\nThe value of gs: ", get_gs_selector);

	GDT_ROOT gdt_descriptor = get_gdt_root();
	GDT_ENTRY* gdt32 = gdt_descriptor.base;
	terminal_write_ptr("gdt base address = ", gdt_descriptor.base);
	terminal_write_uint("gdt size limit = ", gdt_descriptor.limit);

	GDT_ENTRY* gdt16 = GDT16_DESCRIPTOR.base;
	[[maybe_unused]] uint16_t limit16 = GDT16_DESCRIPTOR.limit;
	terminal_write_ptr("GDT16_DESCRIPTOR = ", GDT16_ROOT);
	terminal_write_ptr("GDT16 = ", GDT16_DESCRIPTOR.base);
	terminal_write_uint("GDT16 = ", GDT16_DESCRIPTOR.limit);

	// terminal_writestring("\nThe gdt32 values:\n");
	// terminal_write_hex("gdt[0].lower = ", gdt32[0].lower);
	// terminal_write_hex("gdt[0].higher = ", gdt32[0].higher);
	// terminal_write_hex("gdt[1].lower = ", gdt32[1].lower);
	// terminal_write_hex("gdt[1].higher = ", gdt32[1].higher);
	// terminal_write_hex("gdt[2].lower = ", gdt32[2].lower);
	// terminal_write_hex("gdt[2].higher = ", gdt32[2].higher);
	// terminal_write_hex("gdt[3].lower = ", gdt32[3].lower);
	// terminal_write_hex("gdt[3].higher = ", gdt32[3].higher);

	terminal_writestring("\n\n====== Start of GDT32 ======\n");
	terminal_writestring("\n-------Code Segment: \n");
	gdt_analize(gdt32, 2);
	terminal_writestring("\n-------Data Segment: \n");
	gdt_analize(gdt32, 3);
	terminal_writestring("\n====== End of GDT32 ======\n\n");

	// terminal_writestring("\n\nThe gdt16 values:\n");
	// terminal_write_hex("gdt16[0].lower = ", gdt16[0].lower);
	// terminal_write_hex("gdt16[0].higher = ", gdt16[0].higher);
	// terminal_write_hex("gdt16[1].lower = ", gdt16[1].lower);
	// terminal_write_hex("gdt16[1].higher = ", gdt16[1].higher);
	// terminal_write_hex("gdt16[2].lower = ", gdt16[2].lower);
	// terminal_write_hex("gdt16[2].higher = ", gdt16[2].higher);
	// terminal_write_hex("gdt16[3].lower = ", gdt16[3].lower);
	// terminal_write_hex("gdt16[3].higher = ", gdt16[3].higher);

	terminal_writestring("\n\n====== Start of GDT16 ======\n");
	terminal_writestring("\n-------Code Segment: \n");
	gdt_analize(gdt16, 2);
	terminal_writestring("\n-------Data Segment: \n");
	gdt_analize(gdt16, 3);
	terminal_writestring("\n====== End of GDT16 ======\n\n");

	before();

	[[gnu::unused]] uint16_t result5 = call_real_mode_function(ret_5); // argc automatically calculated

	print_args16(&args16_start);
	kprintf("\n====ok====\n");

	uint16_t result = call_real_mode_function(add16_ref, 104, 201); // argc automatically calculated
	print_args16(&args16_start);
	terminal_write_uint("\nThe result of the real mode call is: ", result);

	uint16_t result2 = call_real_mode_function(complex_operation, 104, 201, 305, 43); // argc automatically calculated
	print_args16(&args16_start);
	terminal_write_uint("\nThe result of the real mode call is: ", result2);

	terminal_writestring("\n\n===== End of Kernel Test=====\n\n");
}

extern void test_printf(void);

// Macro to call all interrupts in the X-Macro

void kernel_main(void) {

	// init_paging();
	// init_page_bitmap();

	/* Initialize terminal interface */
	initialize_terminal();
	terminal_set_scroll(0);

	// kprintf("first test var = %d\n", 23);
	/*
	0x002007ae <+27>:    push   0x17
	0x002007b0 <+29>:    push   0x20553e
	0x002007b5 <+34>:    push   0x2
	0x002007b7 <+36>:    call   0x2025e2 <kprintf_argc>
   */

	// test_printf();

	idt_init();
	// test_all_ints();

	// kprintf("Calling interrupt handler %d\n", 34);
	// __int(69);

	kernel_test();

	terminal_writestring("\n====kernel main entering loop====\n");

	while (true) {
		// kernel main loop
	}

	return;

	// int* big_array = (int*)kmalloc(1024 * 1024 * 1024);
	// for (int i = 0; i < 100000; i++) {
	// 	big_array[i] = i;
	// }
	//
	// print_array_terminal(&term, big_array, 100000);
}
