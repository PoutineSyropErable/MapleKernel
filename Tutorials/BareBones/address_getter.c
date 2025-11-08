extern int resume32;
extern int resume32_end;
extern int protected_16;
extern int stack16_start;
extern int stack16_end;
extern int args16_end;
extern int add1616_start;

extern int misc32_s1;
extern int misc32_s2;

#include "add16.h"
#include "add16_wrapper.h"
#include "push_var_args.h"

int* get_call_add16_address(void) {
	return (int*)&call_add16;
}

int* get_add16_address(void) {
	return (int*)&add16;
}

int* get_protected16_address(void) {
	return (int*)&protected_16;
}

int* get_add1616_start_address(void) {
	return &add1616_start;
}

int* get_add1632_start_address(void) {
	return (int*)call_add16;
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
