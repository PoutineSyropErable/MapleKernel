#include "idt_master.h"
#include "idt_ps2.h"
#include "pic.h"
#include "ps2.h"
#include "ps2_controller.h"
#include "ps2_keyboard_public.h"
#include "ps2_mouse_public.h"
#include "stdio.h"

// =================================== PS2 Setup Handling ==================================
enum handle_ps2_setup_errors handle_ps2_setup_one_keyboard_one_mouse(struct ps2_initialize_device_state device_sates);
enum handle_ps2_setup_errors handle_ps2_setup_one_port_only(struct ps2_initialize_device_state device_sates);
enum handle_ps2_setup_errors handle_ps2_setup_two_keyboard(struct ps2_initialize_device_state device_sates);
enum handle_ps2_setup_errors handle_ps2_setup_two_mouse(struct ps2_initialize_device_state device_sates);
enum handle_ps2_setup_errors handle_ps2_setup_no_port();

void quick_enable_mouse();

enum handle_ps2_setup_errors handle_ps2_setup() {
    struct ps2_initialize_device_state device_sates = setup_ps2_controller();
    kprintf("\n ===== Handling Result ===== \n");

    switch (device_sates.ps2_state_err) {
    case PS2_ID_ERR_none: return handle_ps2_setup_one_keyboard_one_mouse(device_sates);

    case PS2_ID_ERR_two_keyboard: return handle_ps2_setup_two_keyboard(device_sates);

    case PS2_ID_ERR_two_mouse: return handle_ps2_setup_two_mouse(device_sates);

    case PS2_ID_ERR_no_second_port: return handle_ps2_setup_one_port_only(device_sates);

    case PS2_ID_ERR_could_not_init:
    case PS2_ID_ERR_ps2_controller_does_not_exist:
    case PS2_ID_ERR_controller_self_test_failed:
    case PS2_ID_ERR_first_port_self_test_failed:
    case PS2_ID_ERR_second_port_self_test_failed:
    case PS2_ID_ERR_could_not_reset_device1:
    case PS2_ID_ERR_could_not_reset_device2:
    case PS2_ID_ERR_usb_error: return handle_ps2_setup_no_port();

    default: abort_msg("Impossible initialize device error!\n");
    }
}

/* ------------------ Helper Functions ------------------ */

enum handle_ps2_setup_errors handle_ps2_setup_one_keyboard_one_mouse(struct ps2_initialize_device_state device_sates) {
    kprintf("One Keyboard, One Mouse\n");
    enum ps2_device_type port1_type = device_sates.port_one_device_type;
    enum ps2_device_type port2_type = device_sates.port_two_device_type;

    enum ps2_device_super_type port1_super_type = get_device_super_type(port1_type);
    enum ps2_device_super_type port2_super_type = get_device_super_type(port2_type);

    if (port1_super_type == PS2_DST_unknown)
	return PS2_HS_ERR_unrecognized_device1;
    if (port2_super_type == PS2_DST_unknown)
	return PS2_HS_ERR_unrecognized_device2;

    assert(port1_super_type != port2_super_type, "One keyboard, one mouse. Both can't be the same!\n");

    uint8_t              keyboard_port, mouse_port;
    enum ps2_device_type keyboard_type, mouse_type;
    uint8_t              keyboard_interrupt_vector, mouse_interrupt_vector;

    if (port1_super_type == PS2_DST_keyboard && port2_super_type == PS2_DST_mouse) {
	keyboard_port             = 1;
	mouse_port                = 2;
	keyboard_type             = port1_type;
	mouse_type                = port2_type;
	keyboard_interrupt_vector = PS2_PORT1_INTERUPT_VECTOR;
	mouse_interrupt_vector    = PS2_PORT2_INTERUPT_VECTOR;
    } else {
	keyboard_port             = 2;
	mouse_port                = 1;
	keyboard_type             = port2_type;
	mouse_type                = port1_type;
	keyboard_interrupt_vector = PS2_PORT2_INTERUPT_VECTOR;
	mouse_interrupt_vector    = PS2_PORT1_INTERUPT_VECTOR;
    }

    kprintf("\nKeyboard type: |%s|, Mouse type: |%s|\n\n", ps2_device_type_to_string(keyboard_type), ps2_device_type_to_string(mouse_type));

    quick_enable_mouse();

    struct idt_init_ps2_fields       args;
    struct idt_fields_keyboard_mouse args_value;
    args_value.keyboard_port = keyboard_port;
    args_value.mouse_port    = mouse_port;
    args_value.keyboard_type = keyboard_type;
    args_value.mouse_type    = mouse_type;

    args.type                          = ITT_one_keyboard_one_mouse;
    args.value.info_keyboard_and_mouse = args_value;

    set_single_keyboard_port(keyboard_port);
    set_single_mouse_port(mouse_port);
    idt_init(args);
    PIC_remap(PIC_1_OFFSET, PIC_2_OFFSET);
    initialize_irqs();
    IRQ_clear_mask(PS2_PORT1_IRQ);
    IRQ_clear_mask(PS2_PORT2_BRIDGE_IRQ);
    IRQ_clear_mask(PS2_PORT2_IRQ);

    return PS2_HS_ERR_none;
}

enum handle_ps2_setup_errors handle_ps2_setup_one_port_only(struct ps2_initialize_device_state device_sates) {
    kprintf("One Port only\n");

    enum ps2_device_type       only_port_type       = device_sates.port_one_device_type;
    enum ps2_device_super_type only_port_super_type = get_device_super_type(only_port_type);

    if (only_port_super_type == PS2_DST_unknown)
	return PS2_HS_ERR_unrecognized_device1;

    switch (only_port_super_type) {
    case PS2_DST_keyboard: {
	struct idt_init_ps2_fields   args;
	struct idt_fields_1_keyboard args_value;
	args_value.keyboard_type   = only_port_type;
	args.type                  = ITT_one_keyboard;
	args.value.info_1_keyboard = args_value;
	set_single_keyboard_port(1);
	idt_init(args);
	PIC_remap(PIC_1_OFFSET, PIC_2_OFFSET);
	initialize_irqs();
	IRQ_clear_mask(PS2_PORT1_IRQ);
	return PS2_HS_ERR_one_port_only;
    }
    case PS2_DST_mouse: {
	struct idt_init_ps2_fields args;
	struct idt_fields_1_mouse  args_value;
	args_value.mouse_type   = only_port_type;
	args.type               = ITT_one_mouse;
	args.value.info_1_mouse = args_value;
	set_single_mouse_port(1);
	idt_init(args);
	PIC_remap(PIC_1_OFFSET, PIC_2_OFFSET);
	initialize_irqs();
	IRQ_clear_mask(PS2_PORT1_IRQ);
	return PS2_HS_ERR_one_port_only;
    }
    default: abort_msg("Not treated for now!, treat as a bug!\n"); return PS2_HS_ERR_unrecognized_device1;
    }
}

enum handle_ps2_setup_errors handle_ps2_setup_two_keyboard(struct ps2_initialize_device_state device_sates) {
    kprintf("Two keyboards\n");
    enum ps2_device_type keyboard1_type = device_sates.port_one_device_type;
    enum ps2_device_type keyboard2_type = device_sates.port_two_device_type;

    struct idt_init_ps2_fields   args;
    struct idt_fields_2_keyboard args_value = {.keyboard1_port = 1, .keyboard2_port = 2};
    args_value.keyboard1_type               = keyboard1_type;
    args_value.keyboard2_type               = keyboard2_type;
    args.type                               = ITT_two_keyboard;
    args.value.info_2_keyboard              = args_value;

    set_dual_keyboard_port();
    idt_init(args);
    PIC_remap(PIC_1_OFFSET, PIC_2_OFFSET);
    initialize_irqs();
    IRQ_clear_mask(PS2_PORT1_IRQ);
    IRQ_clear_mask(PS2_PORT2_BRIDGE_IRQ);
    IRQ_clear_mask(PS2_PORT2_IRQ);

    return PS2_HS_ERR_two_keyboard;
}

enum handle_ps2_setup_errors handle_ps2_setup_two_mouse(struct ps2_initialize_device_state device_sates) {
    kprintf("Two Mouse\n");
    enum ps2_device_type mouse1_type = device_sates.port_one_device_type;
    enum ps2_device_type mouse2_type = device_sates.port_two_device_type;

    struct idt_init_ps2_fields args;
    struct idt_fields_2_mouse  args_value = {.mouse1_port = 1, .mouse2_port = 2};
    args_value.mouse1_type                = mouse1_type;
    args_value.mouse2_type                = mouse2_type;
    args.type                             = ITT_two_mouse;
    args.value.info_2_mouse               = args_value;

    set_dual_mouse_port();
    idt_init(args);
    PIC_remap(PIC_1_OFFSET, PIC_2_OFFSET);
    initialize_irqs();
    IRQ_clear_mask(PS2_PORT1_IRQ);
    IRQ_clear_mask(PS2_PORT2_BRIDGE_IRQ);
    IRQ_clear_mask(PS2_PORT2_IRQ);

    return PS2_HS_ERR_two_mouse;
}

enum handle_ps2_setup_errors handle_ps2_setup_no_port() {
    kprintf("No PS2 Devices\n");
    return PS2_HS_ERR_no_port;
}
