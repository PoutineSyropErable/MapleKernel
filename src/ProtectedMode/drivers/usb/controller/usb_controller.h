#pragma once

enum usb_cont_error_code {
	USB_CONT_ERR_none = 0
};

enum usb_cont_error_code initialize_usb_controllers();
enum usb_cont_error_code disable_legacy_usb_support();
