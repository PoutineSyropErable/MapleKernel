#pragma once

void kernel_test();

enum handle_ps2_setup_errors
{
    PS2_HS_ERR_none                 = 0,
    PS2_HS_ERR_no_port              = 1,
    PS2_HS_ERR_two_keyboard         = 2,
    PS2_HS_ERR_two_mouse            = 3,
    PS2_HS_ERR_one_port_only        = 4,
    PS2_HS_ERR_unrecognized_device1 = 5,
    PS2_HS_ERR_unrecognized_device2 = 6,
};

enum handle_ps2_setup_errors handle_ps2_setup();
