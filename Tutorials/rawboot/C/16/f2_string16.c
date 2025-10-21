#include "f2_string16.h"
#include "stdio.h"
#include <stdlib.h>

char* getSpacelessBitString16(uint16_t a) {
	char* print_string = (char*)malloc(17);
	print_string[16] = 0;

	for (uint8_t bitIndex = 0; bitIndex < 16; bitIndex++) {
		uint8_t c = a >> bitIndex & 1;
		print_string[(16 - 1) - bitIndex] = '0' + (char)c;
	}

	return print_string;
}

char* addSpacesEvery4bits16(char* spacelessBits) {
	char* print_str = malloc(20);

	uint8_t bitIndex = 0;   // for the spacelessBits[bitIndex]
	uint8_t printIndex = 0; // for print_str[printIndex]
	while (printIndex < 19) {

		if ((printIndex + 1) % 5 == 0) {

			print_str[printIndex] = ' ';
		} else {
			print_str[printIndex] = spacelessBits[bitIndex];

			bitIndex++;
		}

		printIndex++;
	}

	print_str[19] = '\0';
	return print_str;
}

void printBinary16(uint16_t binaryNumber, char* variableName) {
	char* spacelessBitString = getSpacelessBitString16(binaryNumber);
	char* spaceSeparatedBits = addSpacesEvery4bits16(spacelessBitString);
	printf("%s = %s = %x\n", variableName, spaceSeparatedBits, binaryNumber);
	free(spaceSeparatedBits);
	free(spacelessBitString);
}

// --------------------- 8 bits ----------------------

char* getSpacelessBitString8(uint8_t a) {
	char* print_string = (char*)malloc(9);
	print_string[8] = 0;

	for (uint8_t bitIndex = 0; bitIndex < 8; bitIndex++) {
		uint8_t c = a >> bitIndex & 1;
		print_string[(8 - 1) - bitIndex] = '0' + (char)c;
	}

	return print_string;
}

char* addSpacesEvery4bits8(char* spacelessBits) {
	char* print_str = malloc(10);

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

	print_str[9] = '\0';
	return print_str;
}

void printBinary8(uint8_t binaryNumber, char* variableName) {
	char* spacelessBitString = getSpacelessBitString8(binaryNumber);
	char* spaceSeparatedBits = addSpacesEvery4bits8(spacelessBitString);
	printf("%s = %s = %x\n", variableName, spaceSeparatedBits, binaryNumber);
	free(spaceSeparatedBits);
	free(spacelessBitString);
}
