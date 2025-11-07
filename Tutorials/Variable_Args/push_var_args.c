#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

uint16_t args16_start[32];

void push_to_args16(uint16_t num, ...) {

	va_list args;
	va_start(args, num);

	for (int i = 0; i < num; i++) {
		uint16_t value = va_arg(args, int);
		args16_start[i] = value;
	}
}

int main(void) {

	printf("\n====Start of program======\n\n");

	uint16_t argc = 4;

	push_to_args16(argc, 12, 56, 259, 4269);

	for (uint16_t i = 0; i < argc; i++) {
		printf("%u : %u\n", i, args16_start[i]);
	}

	return 0;
}
