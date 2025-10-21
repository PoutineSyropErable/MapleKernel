#pragma once
#include <stdint.h>

char* getSpacelessBitString(uint32_t a);
char* addSpacesEvery4bits(char* spacelessBits);
void printBinary(uint32_t binaryNumber, char* variableName);
