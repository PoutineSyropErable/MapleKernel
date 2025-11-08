#include "f2_string.h"
#include "vga_terminal.h"

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

	terminal_writestring(variableName);
	terminal_writestring(" = ");
	terminal_writestring(spaceSeparatedBits);
	terminal_writestring(" | ");
	print_hex_var(binaryNumber);
	terminal_writestring("\n");
}

void printBinarySize(uint32_t binaryNumber, char* variableName, size_t numberOfBits) {
	char spacelessBitString[33];
	getSpacelessBitString(binaryNumber, spacelessBitString);
	char spaceSeparatedBits[40];
	addSpacesEvery4bits(spacelessBitString, spaceSeparatedBits);

	uint8_t first_index;
	if (numberOfBits >= 32) {
		first_index = 0;
	} else {
		first_index = 39 - (numberOfBits + numberOfBits / 4);
	}

	terminal_writestring(variableName);
	terminal_writestring(" = ");
	terminal_writestring(spaceSeparatedBits + first_index);
	terminal_writestring(" | ");
	print_hex_var(binaryNumber);
	// terminal_writestring("\n");
}
