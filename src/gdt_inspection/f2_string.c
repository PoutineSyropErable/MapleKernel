#include "f2_string.h"
#include "stdio.h"
#include "string_helper.h"
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

uint8_t binary_size(uint32_t binary_number) {
	if (binary_number == 0)
		return 1;

	unsigned bits = 0;
	while (binary_number > 0) {
		bits++;
		binary_number >>= 1;
	}
	return bits;
}

uint8_t hexSize(uint32_t number) {

	if (binary_size(number) % 4) {
		return binary_size(number) / 4 + 1;
	}
	return binary_size(number) / 4;
}

uint8_t max(uint8_t a, uint8_t b) {
	if (a > b) {
		return a;
	}
	return b;
}

void print_binary_var_no_newline(uint32_t binaryNumber, size_t numberOfBits) {
	char spacelessBitString[33];
	getSpacelessBitString(binaryNumber, spacelessBitString);
	char spaceSeparatedBits[40];
	addSpacesEvery4bits(spacelessBitString, spaceSeparatedBits);

	uint8_t bitSize = binary_size(binaryNumber);
	numberOfBits = max(bitSize, numberOfBits);

	uint8_t first_index;
	if (numberOfBits >= 32) {
		first_index = 0;
	} else {
		first_index = 39 - (numberOfBits + numberOfBits / 4);

		if (numberOfBits % 4 == 0) {
			first_index += 1;
		}
	}

	char* str_to_print = spaceSeparatedBits + first_index;

	terminal_writestring(str_to_print);
}

/*
Return the number of char (without the null terminator) to print, given a space is added every 4 hex number
Given a hex 0x1234abcd, we need
'1234 abcd' space.
(Don't count the \0)
*/
static inline uint32_t hex_len(uint32_t i) {
	return i + ((i - 1) / 4);
	// add 1 at the end So there's always place for \0.
	// So at 5, 9, 13, (4n+1), space for ' ' is added
}

void print_hex_f(uint32_t hex_number, uint8_t numberOfHex) {
	assert(numberOfHex <= 16, "Not designed for too big hex (Bounded)");

	uint8_t hex_size = hexSize(hex_number);
	uint8_t number_of_hex_char_to_print = max(hex_size, numberOfHex);

	char output_array[number_of_hex_char_to_print + 1];
	size_t len = uint_to_hex(hex_number, output_array, false);
	assert(hex_size == len, "If hexSize isn't len, then we are fucked!\n");

	[[maybe_unused]] uint8_t total_number_of_char_to_print = hex_len(number_of_hex_char_to_print);
	// We don't need the variable above, since we are manually printing.
	// kprintf("hex_size = %u, len = %u, number_of_hex_char_to_print = %u\ntotal_number_of_char_to_print = %u\n", hex_size, len, number_of_hex_char_to_print, total_number_of_char_to_print);

	terminal_writestring("0x");
	uint8_t number_of_leading_zeros = number_of_hex_char_to_print - len;
	if (number_of_hex_char_to_print < len) {
		number_of_leading_zeros = 0;
	}
	for (uint8_t i = 0; i < number_of_leading_zeros; i++) {
		terminal_putchar('0');
		uint8_t number_of_char_left_to_print = number_of_hex_char_to_print - (i + 1);
		if (number_of_char_left_to_print % 4 == 0) {
			terminal_putchar(' ');
		}
	}

	for (uint8_t i = 0; i < len; i++) {

		char c_to_print = output_array[i];
		terminal_putchar(c_to_print);

		uint8_t j = number_of_hex_char_to_print - i;
		j -= number_of_leading_zeros;
		if ((j - 1) % 4 == 0) {
			terminal_putchar(' ');
		}
		// "1234 abcd 5678 fedc"
	}
}
