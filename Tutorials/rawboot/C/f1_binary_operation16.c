#include "f1_binary_operation_generics.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// ---------------------- 16 bit
uint16_t generateMask16(uint8_t endBig, uint8_t startSmall) {

	uint8_t width = endBig - startSmall + 1;

	uint16_t mask;
	if (width >= 16) {
		mask = -1; // All ones. FFFF
	} else {
		uint16_t mask_plus_one = 1 << (endBig - startSmall + 1);
		mask = mask_plus_one - 1;
	}

	uint16_t mask_left = mask << startSmall;
	return mask_left;
}

uint16_t generateAntiMask16(uint8_t endBig, uint8_t startSmall) {

	return ~generateMask16(endBig, startSmall);
}
// --------------------- 8 bit
uint8_t generateMask8(uint8_t endBig, uint8_t startSmall) {

	uint8_t width = endBig - startSmall + 1;

	uint8_t mask;
	if (width >= 8) {
		mask = -1; // All ones. FFFF
	} else {
		uint8_t mask_plus_one = 1 << (endBig - startSmall + 1);
		mask = mask_plus_one - 1;
	}

	uint8_t mask_left = mask << startSmall;
	return mask_left;
}

uint8_t generateAntiMask8(uint8_t endBig, uint8_t startSmall) {

	return ~generateMask8(endBig, startSmall);
}
