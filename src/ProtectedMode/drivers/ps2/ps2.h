#pragma once

enum handle_ps2_setup_status
{
	PS2_HS_ERR_one_keyboard_one_mouse = 0,
	PS2_HS_ERR_two_keyboard			  = 1,
	PS2_HS_ERR_two_mouse			  = 2,
	PS2_HS_ERR_one_port_only		  = 3,
	PS2_HS_ERR_no_port				  = 4,
	PS2_HS_ERR_unrecognized_device1	  = 5,
	PS2_HS_ERR_unrecognized_device2	  = 6,
};

enum handle_ps2_setup_status handle_ps2_setup();
