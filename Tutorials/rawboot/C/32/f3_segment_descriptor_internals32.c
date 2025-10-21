#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "f1_binary_operation32.h"
#include "f2_string32.h"
#include "f3_segment_descriptor_internals32.h"

uint8_t getType32(GDT32* gdt, size_t index) {
	SegmentDescriptor32 sd = gdt->segmentsInfo[index];

	uint8_t type = getBits32(sd.higher, TYPE_END_32, TYPE_START_32);

	return type;
}

void setType32(GDT32* gdt, size_t index, uint8_t type) {
	SegmentDescriptor32* sd = &gdt->segmentsInfo[index];

	setBitsModify32(&sd->higher, type, TYPE_END_32, TYPE_START_32);

	printBinary32(type, "type");
}

uint8_t getDescriptorTypeS32(GDT32* gdt, size_t index) {
	SegmentDescriptor32 sd = gdt->segmentsInfo[index];

	uint8_t S = getBits32(sd.higher, DESCRIPTOR_TYPE_S_32, DESCRIPTOR_TYPE_S_32);
	return S;
}

void setDescriptorTypeS32(GDT32* gdt, size_t index, uint8_t descriptorTypeS) {
	SegmentDescriptor32* sd = &gdt->segmentsInfo[index];

	setBitsModify32(&sd->higher, descriptorTypeS, DESCRIPTOR_TYPE_S_32, DESCRIPTOR_TYPE_S_32);
	printBinary32(descriptorTypeS, "descriptorTypeS");
}

// Descriptor Priviledge Level
uint8_t getPriviledgeDPL32(GDT32* gdt, size_t index) {
	SegmentDescriptor32 sd = gdt->segmentsInfo[index];

	uint8_t DPL = getBits32(sd.higher, DPL_END_32, DPL_START_32);
	return DPL;
}

// Descriptor Priviledge Level
void setPriviledgeDPL32(GDT32* gdt, size_t index, uint8_t priviledgeDPL) {
	SegmentDescriptor32* sd = &gdt->segmentsInfo[index];

	setBitsModify32(&sd->higher, priviledgeDPL, DPL_END_32, DPL_START_32);

	printBinary32(priviledgeDPL, "priviledgeDPL");
}

uint8_t getPresent32(GDT32* gdt, size_t index) {
	SegmentDescriptor32 sd = gdt->segmentsInfo[index];

	uint8_t P = getBits32(sd.higher, PRESENT_32, PRESENT_32);
	return P;
}

void setPresent32(GDT32* gdt, size_t index, uint8_t presentP) {
	SegmentDescriptor32* sd = &gdt->segmentsInfo[index];

	setBitsModify32(&sd->higher, presentP, PRESENT_32, PRESENT_32);

	printBinary32(presentP, "presentP");
}

uint32_t getSegmentLimit32(GDT32* gdt, size_t index) {
	SegmentDescriptor32 sd = gdt->segmentsInfo[index];
	uint32_t segmentLimit_19_16 = getBits32(sd.higher, SEGMENT_LIMIT_END_2_32, SEGMENT_LIMIT_START_2_32);

	uint32_t segmentLimit_15_0 = getBits32(sd.lower, SEGMENT_LIMIT_END_1_32, SEGMENT_LIMIT_START_1_32);

	uint32_t segmentLimt_19_16_shifted = segmentLimit_19_16 << 16;
	return segmentLimt_19_16_shifted | segmentLimit_15_0;
}

void setSegmentLimit32(GDT32* gdt, size_t index, uint32_t segmentLimit) {
	SegmentDescriptor32* sd = &gdt->segmentsInfo[index];

	uint32_t higher_19_16 = getBits32(segmentLimit, SEGMENT_LIMIT_END_2_32, SEGMENT_LIMIT_START_2_32);

	uint32_t lower_15_0 = getBits32(segmentLimit, SEGMENT_LIMIT_END_1_32, SEGMENT_LIMIT_START_1_32);

	setBitsModify32(&sd->higher, higher_19_16, SEGMENT_LIMIT_END_2_32, SEGMENT_LIMIT_START_2_32);

	setBitsModify32(&sd->lower, lower_15_0, SEGMENT_LIMIT_END_1_32, SEGMENT_LIMIT_START_1_32);

	printBinary32(segmentLimit, "segmentLimit");

	// printBinary(higher_19_16, "higher_19_16");
	// printBinary(lower_15_0, "lower_15_0");
}

// Available for system software
uint8_t getAVL32(GDT32* gdt, size_t index) {
	SegmentDescriptor32 sd = gdt->segmentsInfo[index];

	uint8_t avl = getBits32(sd.higher, AVL_32, AVL_32);

	return avl;
}

void setAVL32(GDT32* gdt, size_t index, uint8_t avl) {
	SegmentDescriptor32* sd = &gdt->segmentsInfo[index];

	setBitsModify32(&sd->higher, avl, AVL_32, AVL_32);

	printBinary32(avl, "avl");
}

uint8_t getLongMode32(GDT32* gdt, size_t index) {
	SegmentDescriptor32 sd = gdt->segmentsInfo[index];

	uint8_t L = getBits32(sd.higher, LONG_MODE_32, LONG_MODE_32);

	return L;
}

void setLongMode32(GDT32* gdt, size_t index, uint8_t L) {
	SegmentDescriptor32* sd = &gdt->segmentsInfo[index];

	setBitsModify32(&sd->higher, L, LONG_MODE_32, LONG_MODE_32);

	printBinary32(L, "L");
}

uint8_t getDefaultOperationSize32(GDT32* gdt, size_t index) {
	SegmentDescriptor32 sd = gdt->segmentsInfo[index];

	// D slash B. D/B. D or B
	uint8_t DsB = getBits32(sd.higher, DEFAULT_OPERATION_SIZE_32, DEFAULT_OPERATION_SIZE_32);

	return DsB;
}

void setDefaultOperationSize32(GDT32* gdt, size_t index, uint8_t DsB) {
	SegmentDescriptor32* sd = &gdt->segmentsInfo[index];

	setBitsModify32(&sd->higher, DsB, DEFAULT_OPERATION_SIZE_32, DEFAULT_OPERATION_SIZE_32);

	printBinary32(DsB, "DsB");
}

uint8_t getGranularity32(GDT32* gdt, size_t index) {
	SegmentDescriptor32 sd = gdt->segmentsInfo[index];

	uint8_t granularity = getBits32(sd.higher, GRANULARITY_32, GRANULARITY_32);

	return granularity;
}

void setGranularity32(GDT32* gdt, size_t index, uint8_t granularity) {
	SegmentDescriptor32* sd = &gdt->segmentsInfo[index];

	setBitsModify32(&sd->higher, granularity, GRANULARITY_32, GRANULARITY_32);

	printBinary32(granularity, "granularity");
}

uint32_t getBaseAddress32(GDT32* gdt, size_t index) {
	/*
	Using magic values cause it somehow cleaner
	*/

	SegmentDescriptor32 sd = gdt->segmentsInfo[index];

	uint32_t base_0_15 = getBits32(sd.lower, 31, 16);

	uint32_t base_16_23 = getBits32(sd.higher, 7, 0);

	uint32_t base_24_31 = getBits32(sd.higher, 31, 24);

	uint32_t base_0_15_shifted = base_0_15 << 0; // get optimised. But i prefer clarity
	uint32_t base_16_23_shifted = base_16_23 << 16;
	uint32_t base_24_31_shifted = base_24_31 << 24;

	uint32_t baseAddress = base_0_15_shifted | base_16_23_shifted | base_24_31_shifted;

	return baseAddress;
}

void setBaseAddress32(GDT32* gdt, size_t index, uint32_t baseAddress) {

	SegmentDescriptor32* sd = &gdt->segmentsInfo[index];

	uint32_t higher_31_24 = getBits32(baseAddress, 31, 24);

	uint32_t higher_7_0 = getBits32(baseAddress, 23, 16);

	uint32_t lower_31_16 = getBits32(baseAddress, 15, 0);

	setBitsModify32(&sd->lower, lower_31_16, 31, 16);

	setBitsModify32(&sd->higher, higher_7_0, 7, 0);

	setBitsModify32(&sd->higher, higher_31_24, 31, 24);

	printBinary32(baseAddress, "baseAddress");

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

void test(void) {
	printf("0xFACEB007 should be transformed to 0xFACDB007\n");
	uint32_t x = 0xFACEB007;
	printf("start = %x\n", x);

	setBitsModify32(&x, 0xD, 19, 16);

	printf("modified = %x\n", x);
	printBinary32(x, "x");
}
