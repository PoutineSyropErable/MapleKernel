#include "f1_binary_operation16.h"

uint8_t generateMask8(size8_t endBig, size8_t startSmall) {

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

uint16_t generateMask16(size8_t endBig, size8_t startSmall) {

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

uint8_t generateAntiMask8(size8_t endBig, size8_t startSmall) {

	return ~generateMask8(endBig, startSmall);
}

uint16_t generateAntiMask16(size8_t endBig, size8_t startSmall) {

	return ~generateMask16(endBig, startSmall);
}

uint8_t getBits8(uint8_t value, size8_t endBig, size8_t startSmall) {

	uint8_t mask_left = generateMask8(endBig, startSmall);
	uint8_t value_left = value & mask_left;
	uint8_t return_value = value_left >> startSmall;
	return return_value;
}

uint16_t getBits16(uint16_t value, size8_t endBig, size8_t startSmall) {

	uint16_t mask_left = generateMask8(endBig, startSmall);
	uint16_t value_left = value & mask_left;
	uint16_t return_value = value_left >> startSmall;
	return return_value;
}

uint8_t setBits8(uint8_t a_dest, uint8_t b_value, size8_t endBig, size8_t startSmall) {
	uint8_t b_shifted = b_value << startSmall;
	uint8_t a_reverse_mask = generateAntiMask8(endBig, startSmall);

	return a_dest & a_reverse_mask | b_shifted;
}

uint16_t setBits16(uint16_t a_dest, uint16_t b_value, size8_t endBig, size8_t startSmall) {
	uint16_t b_shifted = b_value << startSmall;
	uint16_t a_reverse_mask = generateAntiMask16(endBig, startSmall);

	return a_dest & a_reverse_mask | b_shifted;
}

void setBitsModify8(uint8_t* a_dest, uint8_t b_value, size8_t endBig, size8_t startSmall) {

	*a_dest = setBits8(*a_dest, b_value, endBig, startSmall);
}

void setBitsModify16(uint16_t* a_dest, uint16_t b_value, size8_t endBig, size8_t startSmall) {

	*a_dest = setBits16(*a_dest, b_value, endBig, startSmall);
}
