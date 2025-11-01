extern int resume32;
extern int add1616_start;
extern int stack16_start;
extern int stack16_end;
extern int args16_start;
extern int args16_end;

int* get_add1616_start_address(void) {
	return &add1616_start;
}

int* get_resume32_start_address(void) {
	return &resume32;
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
