#include <stdarg.h>
#include <stdio.h>

void print_ints(int num, ...) {

	va_list args;
	va_start(args, num);

	for (int i = 0; i < num; i++) {
		int value = va_arg(args, int);
		printf("%d: %d\n", i, value);
	}
}

int main(void) {

	printf("\n====Start of program======\n");

	return 0;
}
