#pragma once
#include <stdint.h>

#define size16_t uint16_t
#define size8_t uint16_t

uint16_t generateMask16(size8_t endBig, size8_t startSmall);
uint16_t generateAntiMask16(size8_t endBig, size8_t startSmall);
uint16_t getBits16(uint16_t value, size8_t endBig, size8_t startSmall);
uint16_t setBits16(uint16_t a_dest, uint16_t b_value, size8_t endBig, size8_t startSmall);
void setBitsModify16(uint16_t* a_dest, uint16_t b_value, size8_t endBig, size8_t startSmall);

uint8_t generateMask8(size8_t endBig, size8_t startSmall);
uint8_t generateAntiMask8(size8_t endBig, size8_t startSmall);
uint8_t getBits8(uint8_t value, size8_t endBig, size8_t startSmall);
uint8_t setBits8(uint8_t a_dest, uint8_t b_value, size8_t endBig, size8_t startSmall);
void setBitsModify8(uint8_t* a_dest, uint8_t b_value, size8_t endBig, size8_t startSmall);
