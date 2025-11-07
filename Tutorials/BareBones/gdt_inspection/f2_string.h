#pragma once
#include <stdint.h>

char* getSpacelessBitString(uint32_t a, char output[33]);
char* addSpacesEvery4bits(char* spacelessBits, char output[40]);
void printBinary(uint32_t binaryNumber, char* variableName);
