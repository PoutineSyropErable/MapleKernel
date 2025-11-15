#include "address_getter.h"

int* get_stack16_start_address(void) {
	return &stack16_start;
}

int* get_stack16_end_address(void) {
	return &stack16_end;
}

int* get_args16_start_address(void) {
	return (int*)&args16_start;
}
int* get_args16_end_address(void) {
	return &args16_end;
}

int* get_misc32_s1_address(void) {
	return &misc32_s1;
}

int* get_misc32_s2_address(void) {
	return &misc32_s2;
}

int* get_resume32_end_address(void) {
	return &resume32_end;
}

function_t* get_pm16_to_real16_address(void) {
	return &pm16_to_real16;
}

function_t* get_call_real16_function_address(void) {
	return &call_real16_function;
}

function_t* get_resume32_start_address(void) {
	return &resume32;
}

function_t* get_call_realmode_func_with_args_address() {
	return (function_t*)&call_real_mode_function_with_argc;
}

function_t* get_pm32_to_pm16_address(void) {
	return (function_t*)&pm32_to_pm16;
}

function_t* get_add16_address(void) {
	return (function_t*)&add16;
}
