#pragma once
#include <stdint.h>

char* getSpacelessBitString32(uint32_t a);
char* addSpacesEvery4bits32(char* spacelessBits);
void printBinary32(uint32_t binaryNumber, char* variableName);
