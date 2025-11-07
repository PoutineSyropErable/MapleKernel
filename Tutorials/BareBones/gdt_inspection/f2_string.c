#include "f2_string.h"

char* getSpacelessBitString(uint32_t a, char output[33]) {

	output[32] = 0;

	for (uint8_t bitIndex = 0; bitIndex < 32; bitIndex++) {
		uint8_t c = a >> bitIndex & 1;
		output[(32 - 1) - bitIndex] = '0' + (char)c;
	}

	return output;
}

char* addSpacesEvery4bits(char* spacelessBits, char output[40]) {

	uint8_t bitIndex = 0;   // for the spacelessBits[bitIndex]
	uint8_t printIndex = 0; // for print_str[printIndex]
	while (printIndex < 39) {

		if ((printIndex + 1) % 5 == 0) {

			output[printIndex] = ' ';
		} else {
			output[printIndex] = spacelessBits[bitIndex];

			bitIndex++;
		}

		printIndex++;
	}

	output[39] = '\0';
	return output;
}

void printBinary(uint32_t binaryNumber, char* variableName) {
	char spacelessBitString[33];
	getSpacelessBitString(binaryNumber, spacelessBitString);
	char spaceSeparatedBits[40];
	addSpacesEvery4bits(spacelessBitString, spaceSeparatedBits);
	// printf("%s = %s = %x\n", variableName, spaceSeparatedBits, binaryNumber);
}
