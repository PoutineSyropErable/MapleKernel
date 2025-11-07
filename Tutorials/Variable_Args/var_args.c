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

float average_ints(int num, ...) {

	va_list args;
	va_start(args, num);

	float sum = 0;
	int count = 0;

	for (int i = 0; i < num; i++) {
		int value = va_arg(args, int);
		sum += value;
		count += 1;
	}
	sum /= count;

	return sum;
}

float sum_ints(int num, ...) {

	va_list args;
	va_start(args, num);

	float sum_ = 0;

	for (int i = 0; i < num; i++) {
		int value = va_arg(args, int);
		sum_ += value;
	}

	return sum_;
}

int main(void) {

	printf("\n====Start of program======\n");
	print_ints(5, 1, 2, 7, 4, 5);
	printf("sum = %f\n", sum_ints(3, 0, 2, 4));
	printf("average = %f\n", average_ints(3, 0, 2, 4));

	return 0;
}
