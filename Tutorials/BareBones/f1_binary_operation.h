#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint32_t generateMask(uint8_t endBig, uint8_t startSmall);
uint32_t generateAntiMask(uint8_t endBig, uint8_t startSmall);
uint32_t getBits(uint32_t value, uint8_t endBig, uint8_t startSmall);
uint32_t getBits2(uint32_t value, uint8_t endBig, uint8_t startSmall);
uint32_t setBits(uint32_t a_dest, uint32_t b_value, uint8_t endBig, uint8_t startSmall);
void setBitsModify(uint32_t* a_dest, uint32_t b_value, uint8_t endBig, uint8_t startSmall);
