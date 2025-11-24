#include "address_getter.h"
#include "bit_hex_string.h"
#include "call_real16_wrapper.h"
#include "f3_segment_descriptor_internals.h"
#include "idt_master.h"
#include "intrinsics.h"
#include "kernel.h"
#include "os_registers.c"
#include "pic.h"
#include "pit_timer.h"
#include "ps2_controller.h"
#include "ps2_mouse.h"
#include "stdio.h"
#include "string_helper.h"
#include "vga_terminal.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ps2_keyboard_public.h"
#include "ps2_mouse_public.h"

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

	kprintf("int = %d, other int = %d, uint = %u, float = %f.2, bin = %b:7, hex = %h:9-final text|\n\n", 1, -7, 2, 3.0, 0b11011, 0x123bcd);

	return;

	print_extern_address("The address of stack16_start: ", get_stack16_start_address);
	print_extern_address("The address of stack16_end: ", get_stack16_end_address);
	print_extern_address("The address of args16_start: ", get_args16_start_address);
	print_extern_address("The address of args16_end: ", get_args16_end_address);

	print_extern_address("The address of the nice wrapper (>2MB): ", (address_getter_function_t*)get_call_realmode_func_with_args_address);
	[[maybe_unused]] int* pm32_to_pm16_address = print_extern_address("The address of pm32_to_pm16: ", (address_getter_function_t*)get_pm32_to_pm16_address);
	[[maybe_unused]] int* pm16_to_real16_address = print_extern_address("The address of pm16_to_real16: ", (address_getter_function_t*)get_pm16_to_real16_address);
	[[maybe_unused]] int* call_real16_function_address = print_extern_address("The address of call_real16_function_address: ", (address_getter_function_t*)get_call_real16_function_address);
	[[maybe_unused]] int* resume32_address = print_extern_address("The address of resume32: ", (address_getter_function_t*)get_resume32_start_address);
	[[maybe_unused]] int* resume32_end_address = print_extern_address("The address of resume32_end: ", get_resume32_end_address);

// #define ANALIZE_CODE
#ifdef ANALIZE_CODE
	analyse_code((struct CodeAddressesToAnalyse){
	    .pm32_to_pm16_address = pm32_to_pm16_address,
	    .pm16_to_real16_address = pm16_to_real16_address,
	    .call_real16_function_address = call_real16_function_address,
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

// Macro to call all interrupts in the X-Macro

enum handle_ps2_setup_errors {
	PS2_HS_ERR_none = 0,
	PS2_HS_ERR_no_port = 1,
	PS2_HS_ERR_two_keyboard = 2,
	PS2_HS_ERR_two_mouse = 3,
	PS2_HS_ERR_one_port_only = 4,
	PS2_HS_ERR_unrecognized_device1 = 5,
	PS2_HS_ERR_unrecognized_device2 = 6,
};

enum handle_ps2_setup_errors handle_ps2_setup() {
	struct ps2_initialize_device_state device_sates = setup_ps2_controller();
	kprintf("\n ===== Handling Result ===== \n");

	switch (device_sates.ps2_state_err) {
	case PS2_ID_ERR_none:
		goto one_keyboard_one_mouse;
	case PS2_ID_ERR_two_keyboard:
		goto two_keyboard;
	case PS2_ID_ERR_two_mouse:
		goto two_mouse;
	case PS2_ID_ERR_no_second_port:
		goto one_port_only;

	// rather have this then default
	case PS2_ID_ERR_could_not_init:
		goto no_port;
	case PS2_ID_ERR_ps2_controller_does_not_exist:
		goto no_port;
	case PS2_ID_ERR_controller_self_test_failed:
		goto no_port;
	case PS2_ID_ERR_first_port_self_test_failed:
		goto no_port;
	case PS2_ID_ERR_second_port_self_test_failed:
		goto no_port;
	case PS2_ID_ERR_could_not_reset_device1:
		goto no_port;
	case PS2_ID_ERR_could_not_reset_device2:
		goto no_port;
	case PS2_ID_ERR_usb_error:
		goto no_port;
	default:
		abort_msg("Impossible initialize device error!\n");
	}

one_keyboard_one_mouse:
	kprintf("One Keyboard, One Mouse\n");
	enum ps2_device_type port1_type = device_sates.port_one_device_type;
	enum ps2_device_type port2_type = device_sates.port_two_device_type;

	enum ps2_device_super_type port1_super_type = get_device_super_type(port1_type);
	enum ps2_device_super_type port2_super_type = get_device_super_type(port2_type);
	if (port1_super_type == PS2_DST_unknown) {
		abort_msg("Not treated for now!, treat as a bug!\n");
		return PS2_HS_ERR_unrecognized_device1;
	}
	if (port2_super_type == PS2_DST_unknown) {
		abort_msg("Not treated for now!, treat as a bug!\n");
		return PS2_HS_ERR_unrecognized_device2;
	}
	// Assert because this is a logic bug that created an impossible scenario.
	// It should not be handled. The code should be change so it never happens
	assert(port1_super_type != port2_super_type, "It should be one keyboard, one mouse. Both can't be the same!\n");

	uint8_t keyboard_port, mouse_port;
	enum ps2_device_type mouse_type;
	enum ps2_device_type keyboard_type;
	uint8_t keyboard_interrupt_vector;
	uint8_t mouse_interrupt_vector;

	if (port1_super_type == PS2_DST_keyboard && port2_super_type == PS2_DST_mouse) {
		keyboard_port = 1;
		mouse_port = 2;

		keyboard_type = port1_type;
		mouse_type = port2_type;

		keyboard_interrupt_vector = PS2_PORT1_INTERUPT_VECTOR;
		mouse_interrupt_vector = PS2_PORT2_INTERUPT_VECTOR;
	} else if (port1_super_type == PS2_DST_mouse && port2_super_type == PS2_DST_keyboard) {
		mouse_port = 1;
		keyboard_port = 2;

		mouse_type = port1_type;
		keyboard_type = port2_type;

		mouse_interrupt_vector = PS2_PORT1_INTERUPT_VECTOR;
		keyboard_interrupt_vector = PS2_PORT2_INTERUPT_VECTOR;
	} else {
		abort_msg("Impossible scenario! Should be bug fixed and prevented\n!");
	}

	kprintf("\nKeyboard type: |%s|, Mouse type: |%s|\n\n", ps2_device_type_to_string(keyboard_type), ps2_device_type_to_string(mouse_type));

	// TODO: replace the quick enable mouse by the actual enable mouse that will be implemented
	quick_enable_mouse();
	struct idt_init_ps2_fields args;
	struct idt_fields_keyboard_mouse args_value;
	args_value.keyboard_port = keyboard_port;
	args_value.mouse_port = mouse_port;
	args_value.keyboard_type = keyboard_type;
	args_value.mouse_type = keyboard_type;

	args.type = ITT_one_keyboard_one_mouse;
	args.value.info_keyboard_and_mouse = args_value;

	set_single_keyboard_port(keyboard_port);
	set_single_mouse_port(mouse_port);
	idt_init(args);
	PIC_remap(PIC_1_OFFSET, PIC_2_OFFSET);
	initialize_irqs();
	IRQ_clear_mask(PS2_PORT1_IRQ);
	IRQ_clear_mask(PS2_PORT2_BRIDGE_IRQ);
	IRQ_clear_mask(PS2_PORT2_IRQ);
	kprintf("Normally set the stuff\n");
	return PS2_HS_ERR_none;

one_port_only:
	kprintf("One Port only\n");

	enum ps2_device_type only_port_type = device_sates.port_one_device_type;
	enum ps2_device_super_type only_port_super_type = get_device_super_type(only_port_type);
	if (only_port_super_type == PS2_DST_unknown) {
		abort_msg("Not treated for now!, treat as a bug!\n");
		return PS2_HS_ERR_unrecognized_device1;
	}
	switch (only_port_super_type) {
	case PS2_DST_keyboard:
		enum ps2_device_type keyboard_type = device_sates.port_one_device_type;
		struct idt_init_ps2_fields args_keyboard;
		struct idt_fields_1_keyboard args_keyboard_value;
		args_keyboard_value.keyboard_type = keyboard_type;

		args_keyboard.type = ITT_one_keyboard;
		args_keyboard.value.info_1_keyboard = args_keyboard_value;
		set_single_keyboard_port(1);
		idt_init(args_keyboard);
		PIC_remap(PIC_1_OFFSET, PIC_2_OFFSET);
		initialize_irqs();
		IRQ_clear_mask(PS2_PORT1_IRQ);
		return PS2_HS_ERR_one_port_only;

	case PS2_DST_mouse:
		enum ps2_device_type mouse_type = device_sates.port_one_device_type;
		struct idt_init_ps2_fields args_mouse;
		struct idt_fields_1_mouse args_mouse_value;
		args_mouse_value.mouse_type = mouse_type;

		args_mouse.type = ITT_one_mouse;
		args_mouse.value.info_1_mouse = args_mouse_value;
		set_single_mouse_port(1);
		idt_init(args_mouse);
		PIC_remap(PIC_1_OFFSET, PIC_2_OFFSET);
		initialize_irqs();
		IRQ_clear_mask(PS2_PORT1_IRQ);

		return PS2_HS_ERR_one_port_only;
	case PS2_DST_unknown:
		abort_msg("Not treated for now!, treat as a bug!\n");
		return PS2_HS_ERR_unrecognized_device1;
	}

	return PS2_HS_ERR_one_port_only;

two_keyboard:

	kprintf("Two keyboards\n");
	enum ps2_device_type keyboard1_type = device_sates.port_one_device_type;
	enum ps2_device_type keyboard2_type = device_sates.port_two_device_type;
	enum ps2_device_super_type keyboard1_super_type = get_device_super_type(keyboard1_type);
	enum ps2_device_super_type keyboard2_super_type = get_device_super_type(keyboard2_type);
	assert(keyboard1_super_type == keyboard2_super_type && keyboard1_super_type == PS2_DST_keyboard,
	       "Inconsistent super types. Should be two keyboard. This is an impossible scenario and should never happen. If it does, it needs to be bug fixed. Not error treated \n");

	struct idt_init_ps2_fields args_two_keyboard;
	struct idt_fields_2_keyboard args_two_keyboard_value = {.keyboard1_port = 1, .keyboard2_port = 2};
	args_two_keyboard_value.keyboard1_type = keyboard1_type;
	args_two_keyboard_value.keyboard2_type = keyboard2_type;
	// Kinda obvious that the keyboard 1 is in keyboard port 1...
	// hmm... the function itself could assume

	args_two_keyboard.type = ITT_two_keyboard;
	args_two_keyboard.value.info_2_keyboard = args_two_keyboard_value;
	set_dual_keyboard_port();
	idt_init(args_two_keyboard);
	PIC_remap(PIC_1_OFFSET, PIC_2_OFFSET);
	initialize_irqs();
	IRQ_clear_mask(PS2_PORT1_IRQ);
	IRQ_clear_mask(PS2_PORT2_BRIDGE_IRQ);
	IRQ_clear_mask(PS2_PORT2_IRQ);
	return PS2_HS_ERR_two_keyboard;

two_mouse:
	kprintf("Two Mouse\n");
	enum ps2_device_type mouse1_type = device_sates.port_one_device_type;
	enum ps2_device_type mouse2_type = device_sates.port_one_device_type;
	enum ps2_device_super_type mouse1_super_type = get_device_super_type(mouse1_type);
	enum ps2_device_super_type mouse2_super_type = get_device_super_type(mouse2_type);
	assert(mouse1_super_type == mouse2_super_type && mouse1_super_type == PS2_DST_keyboard,
	       "Inconsistent super types. Should be two mouse. Not treated for now, assumed as a bug!\n");

	struct idt_init_ps2_fields args_two_mouse;
	struct idt_fields_2_mouse args_two_mouse_value = {.mouse1_port = 1, .mouse2_port = 2};
	args_two_mouse_value.mouse1_type = mouse1_type;
	args_two_mouse_value.mouse2_type = mouse2_type;

	args_two_mouse.type = ITT_two_keyboard;
	args_two_mouse.value.info_2_mouse = args_two_mouse_value;
	set_dual_mouse_port();
	idt_init(args_two_mouse);
	PIC_remap(PIC_1_OFFSET, PIC_2_OFFSET);
	initialize_irqs();
	IRQ_clear_mask(PS2_PORT1_IRQ);
	IRQ_clear_mask(PS2_PORT2_BRIDGE_IRQ);
	IRQ_clear_mask(PS2_PORT2_IRQ);
	return PS2_HS_ERR_two_mouse;

no_port:
	// Already done, no need for a todo
	kprintf("No PS2 Devices\n");
	return PS2_HS_ERR_no_port;
}

void kernel_main(void) {

	// init_paging();
	// init_page_bitmap();

	/* Initialize terminal interface */
	initialize_terminal();
	terminal_set_scroll(0);

	kernel_test();
	test_printf();
	test_assert(); // gd, and set to false and play with it

	/* Some day the future, it might be important to know the state here (hence err_discard). But today is not that day*/
	[[gnu::unused]] enum handle_ps2_setup_errors
	    err_discard = handle_ps2_setup();
	kprintf("err_discard : %d\n", err_discard);

	// test_ps2_keyboard_commands();
	setup_keyboard();

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
