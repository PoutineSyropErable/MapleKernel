#include "f2_string32.h"
#include "stdio.h"
#include <stdlib.h>

char* getSpacelessBitString32(uint32_t a) {
	char* print_string = (char*)malloc(33);
	print_string[32] = 0;

	for (uint8_t bitIndex = 0; bitIndex < 32; bitIndex++) {
		uint8_t c = a >> bitIndex & 1;
		print_string[(32 - 1) - bitIndex] = '0' + (char)c;
	}

	return print_string;
}

char* addSpacesEvery4bits32(char* spacelessBits) {
	char* print_str = malloc(40);

	uint8_t bitIndex = 0;   // for the spacelessBits[bitIndex]
	uint8_t printIndex = 0; // for print_str[printIndex]
	while (printIndex < 39) {

		if ((printIndex + 1) % 5 == 0) {

			print_str[printIndex] = ' ';
		} else {
			print_str[printIndex] = spacelessBits[bitIndex];

			bitIndex++;
		}

		printIndex++;
	}

	print_str[39] = '\0';
	return print_str;
}

void printBinary32(uint32_t binaryNumber, char* variableName) {
	char* spacelessBitString = getSpacelessBitString32(binaryNumber);
	char* spaceSeparatedBits = addSpacesEvery4bits32(spacelessBitString);
	printf("%s = %s = %x\n", variableName, spaceSeparatedBits, binaryNumber);
	free(spaceSeparatedBits);
	free(spacelessBitString);
}
