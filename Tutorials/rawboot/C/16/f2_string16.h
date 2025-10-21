#pragma once
#include <stdint.h>

char* getSpacelessBitString16(uint16_t a);
char* addSpacesEvery4bits16(char* spacelessBits);
void printBinary16(uint16_t binaryNumber, char* variableName);

char* getSpacelessBitString8(uint8_t a);
char* addSpacesEvery4bits8(char* spacelessBits);
void printBinary8(uint8_t binaryNumber, char* variableName);
