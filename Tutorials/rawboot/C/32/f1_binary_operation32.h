#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint32_t generateMask32(uint8_t endBig, uint8_t startSmall);
uint32_t generateAntiMask32(uint8_t endBig, uint8_t startSmall);
uint32_t getBits32(uint32_t value, uint8_t endBig, uint8_t startSmall);
uint32_t setBits32(uint32_t a_dest, uint32_t b_value, uint8_t endBig, uint8_t startSmall);
void setBitsModify32(uint32_t* a_dest, uint32_t b_value, uint8_t endBig, uint8_t startSmall);
