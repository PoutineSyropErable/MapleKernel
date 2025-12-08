#include "address_getter.h"
#include "bit_hex_string.h"
#include "call_real16_wrapper.h"
#include "f3_segment_descriptor_internals.h"
#include "idt_master.h"
#include "intrinsics.h"
#include "kernel.h"
#include "kernel_cpp.h"
#include "kernel_zig.h"
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

#include "framebuffer.h"
#include "framebuffer_multiboot.h"
#include "multiboot.h"

#include "kernel_helper.h"

GDT_ROOT *GDT16_ROOT = &GDT16_DESCRIPTOR;

void kernel_main(uint32_t mb2_info_addr, uint32_t magic, uint32_t is_proper_multiboot_32)
{

    initialize_terminal();
    terminal_set_scroll(0);

    kprintf("\n===========Terminal Initialized=============\n");
    kprintf("addr = %u, magic = %h, is_proper_multiboot_32 = %u\n", mb2_info_addr, magic, is_proper_multiboot_32);

    bool is_proper_multiboot = is_proper_multiboot_32;
    if (is_proper_multiboot)
    {
        kprintf("It's proper multiboot\n");
    }
    else
    {
        abort_msg("Not proper multiboot\n");
        // The abbort might not be needed anyway.
    }

    struct rsdp_tagged_c rsdp_tagged       = get_rsdp(mb2_info_addr);
    char                *rsdp_type_names[] = {"NULL", "OLD", "NEW"};
    char                *name              = rsdp_type_names[rsdp_tagged.new_or_old];
    kprintf("RSDP Type = %s\n", name);
    kprintf("NULL = 0, OLD = 1, NEW = 2. The type of rsdp: %d\n", rsdp_tagged.new_or_old);
    void *rsdp = rsdp_tagged.rsdp;
    kprintf("rsdp = %h\n", rsdp);

    bool val = validate_rsdp_c(rsdp);
    kprintf("Valid = %b\n", val);

    struct framebuffer_info_t framebuffer = get_framebuffer(mb2_info_addr);

    do_test_c(framebuffer);
    return;

    // init_paging();
    // init_page_bitmap();

    /* Initialize terminal interface */

    kernel_test();
    test_printf();
    test_assert(); // gd, and set to false and play with it

    /* Some day the future, it might be important to know the state here (hence
     * err_discard). But today is not that day*/
    [[gnu::unused]] enum handle_ps2_setup_errors err_discard = handle_ps2_setup();
    kprintf("err_discard : %d\n", err_discard);

    // test_ps2_keyboard_commands();
    setup_keyboard();

    terminal_writestring("\n====kernel main entering loop====\n");

    cpp_main();
    zig_main();

    while (true)
    {
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
