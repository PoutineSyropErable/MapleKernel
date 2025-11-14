#include <stdio.h>

int len(int i) {
	return i + ((i - 1) / 4) + 1;
}

int main(void) {

	printf("\n\n\n");

	for (int i = 0; i <= 32; i++) {
		printf("i:%d -> array_len = %d\n", i, len(i));
		if (!(i % 4)) {
			printf("\n");
		}
	}
}
