#include <stdint.h>

#include "f1_binary_operation.h"
#include "f2_string.h"
#include "f3_segment_descriptor_internals.h"

uint8_t getType(SegmentDescriptor* sd) {

	uint8_t type = getBits(sd->higher, TYPE_END, TYPE_START);
	return type;
}

void setType(SegmentDescriptor* sd, uint8_t type) {

	setBitsModify(&sd->higher, type, TYPE_END, TYPE_START);

	printBinary(type, "type");
}

uint8_t getDescriptorTypeS(SegmentDescriptor* sd) {

	uint8_t S = getBits(sd->higher, DESCRIPTOR_TYPE_S, DESCRIPTOR_TYPE_S);
	return S;
}

void setDescriptorTypeS(SegmentDescriptor* sd, uint8_t descriptorTypeS) {

	setBitsModify(&sd->higher, descriptorTypeS, DESCRIPTOR_TYPE_S, DESCRIPTOR_TYPE_S);
	printBinary(descriptorTypeS, "descriptorTypeS");
}

// Descriptor Priviledge Level
uint8_t getPriviledgeDPL(SegmentDescriptor* sd) {

	uint8_t DPL = getBits(sd->higher, DPL_END, DPL_START);
	return DPL;
}

// Descriptor Priviledge Level
void setPriviledgeDPL(SegmentDescriptor* sd, uint8_t priviledgeDPL) {

	setBitsModify(&sd->higher, priviledgeDPL, DPL_END, DPL_START);

	printBinary(priviledgeDPL, "priviledgeDPL");
}

uint8_t getPresent(SegmentDescriptor* sd) {

	uint8_t P = getBits(sd->higher, PRESENT, PRESENT);
	return P;
}

void setPresent(SegmentDescriptor* sd, uint8_t presentP) {

	setBitsModify(&sd->higher, presentP, PRESENT, PRESENT);
	printBinary(presentP, "presentP");
}

uint32_t getSegmentLimit(SegmentDescriptor* sd) {

	uint32_t segmentLimit_19_16 = getBits(sd->higher, SEGMENT_LIMIT_END_2, SEGMENT_LIMIT_START_2);
	uint32_t segmentLimit_15_0 = getBits(sd->lower, SEGMENT_LIMIT_END_1, SEGMENT_LIMIT_START_1);

	uint32_t segmentLimt_19_16_shifted = segmentLimit_19_16 << 16;
	return segmentLimt_19_16_shifted | segmentLimit_15_0;
}

void setSegmentLimit(SegmentDescriptor* sd, uint32_t segmentLimit) {

	uint32_t higher_19_16 = getBits(segmentLimit, SEGMENT_LIMIT_END_2, SEGMENT_LIMIT_START_2);
	uint32_t lower_15_0 = getBits(segmentLimit, SEGMENT_LIMIT_END_1, SEGMENT_LIMIT_START_1);

	setBitsModify(&sd->higher, higher_19_16, SEGMENT_LIMIT_END_2, SEGMENT_LIMIT_START_2);
	setBitsModify(&sd->lower, lower_15_0, SEGMENT_LIMIT_END_1, SEGMENT_LIMIT_START_1);

	printBinary(segmentLimit, "segmentLimit");
	// printBinary(higher_19_16, "higher_19_16");
	// printBinary(lower_15_0, "lower_15_0");
}

// Available for system software
uint8_t getAVL(SegmentDescriptor* sd) {

	uint8_t avl = getBits(sd->higher, AVL, AVL);
	return avl;
}

void setAVL(SegmentDescriptor* sd, uint8_t avl) {

	setBitsModify(&sd->higher, avl, AVL, AVL);
	printBinary(avl, "avl");
}

uint8_t getLongMode(SegmentDescriptor* sd) {

	uint8_t L = getBits(sd->higher, LONG_MODE, LONG_MODE);
	return L;
}

void setLongMode(SegmentDescriptor* sd, uint8_t L) {

	setBitsModify(&sd->higher, L, LONG_MODE, LONG_MODE);
	printBinary(L, "L");
}

uint8_t getDefaultOperationSize(SegmentDescriptor* sd) {

	// D slash B. D/B. D or B
	uint8_t DsB = getBits(sd->higher, DEFAULT_OPERATION_SIZE, DEFAULT_OPERATION_SIZE);
	return DsB;
}

void setDefaultOperationSize(SegmentDescriptor* sd, uint8_t DsB) {

	setBitsModify(&sd->higher, DsB, DEFAULT_OPERATION_SIZE, DEFAULT_OPERATION_SIZE);
	printBinary(DsB, "DsB");
}

uint8_t getGranularity(SegmentDescriptor* sd) {

	uint8_t granularity = getBits(sd->higher, GRANULARITY, GRANULARITY);
	return granularity;
}

void setGranularity(SegmentDescriptor* sd, uint8_t granularity) {

	setBitsModify(&sd->higher, granularity, GRANULARITY, GRANULARITY);
	printBinary(granularity, "granularity");
}

uint32_t getBaseAddress(SegmentDescriptor* sd) {
	/*
	Using magic values cause it somehow cleaner
	*/

	uint32_t base_0_15 = getBits(sd->lower, 31, 16);
	uint32_t base_16_23 = getBits(sd->higher, 7, 0);
	uint32_t base_24_31 = getBits(sd->higher, 31, 24);

	uint32_t base_0_15_shifted = base_0_15 << 0; // get optimised. But i prefer clarity
	uint32_t base_16_23_shifted = base_16_23 << 16;
	uint32_t base_24_31_shifted = base_24_31 << 24;

	uint32_t baseAddress = base_0_15_shifted | base_16_23_shifted | base_24_31_shifted;

	return baseAddress;
}

void setBaseAddress(SegmentDescriptor* sd, uint32_t baseAddress) {

	uint32_t higher_31_24 = getBits(baseAddress, 31, 24);
	uint32_t higher_7_0 = getBits(baseAddress, 23, 16);
	uint32_t lower_31_16 = getBits(baseAddress, 15, 0);

	setBitsModify(&sd->lower, lower_31_16, 31, 16);
	setBitsModify(&sd->higher, higher_7_0, 7, 0);
	setBitsModify(&sd->higher, higher_31_24, 31, 24);

	printBinary(baseAddress, "baseAddress");
	// printf("\n");
	// printBinary(higher_31_24, "higher_31_24 (Base 31-24)"); // 8
	// printBinary(higher_7_0, "higher_7_0 (Base 23-16)  ");   // 8
	//
	// printBinary(lower_31_16, "lower_31_16 (Base 15-0)  "); // 8
	//
	// printBinary(sd->lower, "sd->lower");   // 8
	// printBinary(sd->higher, "sd->higher"); // 8

	return;
}
