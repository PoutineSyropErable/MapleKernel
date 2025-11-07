#include "f1_binary_operation.h"
#include <stdint.h>

uint32_t generateMask(uint8_t endBig, uint8_t startSmall) {

	uint8_t width = endBig - startSmall + 1;

	uint32_t mask;
	if (width >= 32) {
		mask = 0xFFFFFFFF;
	} else {
		uint32_t mask_plus_one = 1 << (endBig - startSmall + 1);
		mask = mask_plus_one - 1;
	}

	uint32_t mask_left = mask << startSmall;
	return mask_left;
}

uint32_t generateAntiMask(uint8_t endBig, uint8_t startSmall) {

	return ~generateMask(endBig, startSmall);
}

uint32_t getBits(uint32_t value, uint8_t endBig, uint8_t startSmall) {

	uint32_t mask_left = generateMask(endBig, startSmall);
	uint32_t value_left = value & mask_left;
	uint32_t return_value = value_left >> startSmall;
	return return_value;
}

uint32_t getBits2(uint32_t value, uint8_t endBig, uint8_t startSmall) {
	if (endBig < startSmall) {
		printf("[ERROR]: big >= small ");
		abort();
	}

	uint32_t shifted_value = value >> startSmall;
	uint32_t mask_plus_one = 1 << (endBig - startSmall + 1);
	uint32_t mask = mask_plus_one - 1;

	return shifted_value & mask;
}

uint32_t setBits(uint32_t a_dest, uint32_t b_value, uint8_t endBig, uint8_t startSmall) {
	/*
	Example:


	setBits(&a, b, 6, 4);

	we want to touch from x to y.

	return c;
	     x y
	    7654 3210
	a = 1100 1010
	b =  111
	c = 1111 1010
	     xxx

	*/

	uint32_t b_shifted = b_value << startSmall;
	uint32_t a_reverse_mask = generateAntiMask(endBig, startSmall);

	return a_dest & a_reverse_mask | b_shifted;
}

/*
    Dest: The destination
    value: The value to write.
    end big, startSmall: The bit values to write.

    Example:
    Let's replace the E with a D for x = 0xFACEB007;

    Dest = &x.
    EB007
    43210
    5*4-1, 4*4.
    20, 16
    19. 16
    19, 18, 17, 16

      F    A   C     E   B     0   0    7
    1111 1010 1100 1110 1101 0000 0000 0111

    setBits(&x, 0xD, 19, 16);

*/
void setBitsModify(uint32_t* a_dest, uint32_t b_value, uint8_t endBig, uint8_t startSmall) {

	*a_dest = setBits(*a_dest, b_value, endBig, startSmall);
}
