#pragma once
#include <stddef.h>
#include <stdint.h>

char* getSpacelessBitString(uint32_t a, char output[33]);
char* addSpacesEvery4bits(char* spacelessBits, char output[40]);
void printBinary(uint32_t binaryNumber, char* variableName);

void printBinarySize(uint32_t binaryNumber, char* variableName, size_t numberOfBits);

void print_binary_var_no_newline(uint32_t binaryNumber, size_t numberOfBits);

uint8_t binSize(uint32_t number);
