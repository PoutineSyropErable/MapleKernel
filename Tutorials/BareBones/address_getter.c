extern int resume32;
extern int resume32_end;
extern int protected_16;
extern int stack16_start;
extern int stack16_end;
extern int args16_end;
extern int call_real16_function;

extern int misc32_s1;
extern int misc32_s2;

#include "push_var_args.h"
#include "realmode_functions.h"

int* get_to_pm16_address(void) {
	return (int*)&to_pm16;
}

int* get_add16_address(void) {
	return (int*)&add16;
}

int* get_protected16_address(void) {
	return (int*)&protected_16;
}

int* get_call_real16_function_address(void) {
	return &call_real16_function;
}

int* get_to_pm16_start_address(void) {
	return (int*)to_pm16;
}

int* get_resume32_start_address(void) {
	return &resume32;
}

int* get_resume32_end_address(void) {
	return &resume32_end;
}

int* get_misc32_s1_address(void) {
	return &misc32_s1;
}

int* get_misc32_s2_address(void) {
	return &misc32_s2;
}

int* get_stack16_start_address(void) {
	return &stack16_start;
}

int* get_stack16_end_address(void) {
	return &stack16_end;
}

int* get_args16_start_address(void) {
	return &args16_start;
}
int* get_args16_end_address(void) {
	return &args16_end;
}
