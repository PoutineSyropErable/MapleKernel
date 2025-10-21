#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "f1_binary_operation32.h"
#include "f2_string32.h"
#include "f3_segment_descriptor_internals32.h"

uint8_t getType(GDT* gdt, size_t index) {
	SegmentDescriptor sd = gdt->segmentsInfo[index];

	uint8_t type = getBits32(sd.higher, TYPE_END, TYPE_START);

	return type;
}

void setType(GDT* gdt, size_t index, uint8_t type) {
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];

	setBitsModify32(&sd->higher, type, TYPE_END, TYPE_START);

	printBinary32(type, "type");
}

uint8_t getDescriptorTypeS(GDT* gdt, size_t index) {
	SegmentDescriptor sd = gdt->segmentsInfo[index];

	uint8_t S = getBits32(sd.higher, DESCRIPTOR_TYPE_S, DESCRIPTOR_TYPE_S);
	return S;
}

void setDescriptorTypeS(GDT* gdt, size_t index, uint8_t descriptorTypeS) {
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];

	setBitsModify32(&sd->higher, descriptorTypeS, DESCRIPTOR_TYPE_S, DESCRIPTOR_TYPE_S);
	printBinary32(descriptorTypeS, "descriptorTypeS");
}

// Descriptor Priviledge Level
uint8_t getPriviledgeDPL(GDT* gdt, size_t index) {
	SegmentDescriptor sd = gdt->segmentsInfo[index];

	uint8_t DPL = getBits32(sd.higher, DPL_END, DPL_START);
	return DPL;
}

// Descriptor Priviledge Level
void setPriviledgeDPL(GDT* gdt, size_t index, uint8_t priviledgeDPL) {
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];

	setBitsModify32(&sd->higher, priviledgeDPL, DPL_END, DPL_START);

	printBinary32(priviledgeDPL, "priviledgeDPL");
}

uint8_t getPresent(GDT* gdt, size_t index) {
	SegmentDescriptor sd = gdt->segmentsInfo[index];

	uint8_t P = getBits32(sd.higher, PRESENT, PRESENT);
	return P;
}

void setPresent(GDT* gdt, size_t index, uint8_t presentP) {
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];

	setBitsModify32(&sd->higher, presentP, PRESENT, PRESENT);

	printBinary32(presentP, "presentP");
}

uint32_t getSegmentLimit(GDT* gdt, size_t index) {
	SegmentDescriptor sd = gdt->segmentsInfo[index];
	uint32_t segmentLimit_19_16 = getBits32(sd.higher, SEGMENT_LIMIT_END_2, SEGMENT_LIMIT_START_2);

	uint32_t segmentLimit_15_0 = getBits32(sd.lower, SEGMENT_LIMIT_END_1, SEGMENT_LIMIT_START_1);

	uint32_t segmentLimt_19_16_shifted = segmentLimit_19_16 << 16;
	return segmentLimt_19_16_shifted | segmentLimit_15_0;
}

void setSegmentLimit(GDT* gdt, size_t index, uint32_t segmentLimit) {
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];

	uint32_t higher_19_16 = getBits32(segmentLimit, SEGMENT_LIMIT_END_2, SEGMENT_LIMIT_START_2);

	uint32_t lower_15_0 = getBits32(segmentLimit, SEGMENT_LIMIT_END_1, SEGMENT_LIMIT_START_1);

	setBitsModify32(&sd->higher, higher_19_16, SEGMENT_LIMIT_END_2, SEGMENT_LIMIT_START_2);

	setBitsModify32(&sd->lower, lower_15_0, SEGMENT_LIMIT_END_1, SEGMENT_LIMIT_START_1);

	printBinary32(segmentLimit, "segmentLimit");

	// printBinary(higher_19_16, "higher_19_16");
	// printBinary(lower_15_0, "lower_15_0");
}

// Available for system software
uint8_t getAVL(GDT* gdt, size_t index) {
	SegmentDescriptor sd = gdt->segmentsInfo[index];

	uint8_t avl = getBits32(sd.higher, AVL, AVL);

	return avl;
}

void setAVL(GDT* gdt, size_t index, uint8_t avl) {
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];

	setBitsModify32(&sd->higher, avl, AVL, AVL);

	printBinary32(avl, "avl");
}

uint8_t getLongMode(GDT* gdt, size_t index) {
	SegmentDescriptor sd = gdt->segmentsInfo[index];

	uint8_t L = getBits32(sd.higher, LONG_MODE, LONG_MODE);

	return L;
}

void setLongMode(GDT* gdt, size_t index, uint8_t L) {
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];

	setBitsModify32(&sd->higher, L, LONG_MODE, LONG_MODE);

	printBinary32(L, "L");
}

uint8_t getDefaultOperationSize(GDT* gdt, size_t index) {
	SegmentDescriptor sd = gdt->segmentsInfo[index];

	// D slash B. D/B. D or B
	uint8_t DsB = getBits32(sd.higher, DEFAULT_OPERATION_SIZE, DEFAULT_OPERATION_SIZE);

	return DsB;
}

void setDefaultOperationSize(GDT* gdt, size_t index, uint8_t DsB) {
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];

	setBitsModify32(&sd->higher, DsB, DEFAULT_OPERATION_SIZE, DEFAULT_OPERATION_SIZE);

	printBinary32(DsB, "DsB");
}

uint8_t getGranularity(GDT* gdt, size_t index) {
	SegmentDescriptor sd = gdt->segmentsInfo[index];

	uint8_t granularity = getBits32(sd.higher, GRANULARITY, GRANULARITY);

	return granularity;
}

void setGranularity(GDT* gdt, size_t index, uint8_t granularity) {
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];

	setBitsModify32(&sd->higher, granularity, GRANULARITY, GRANULARITY);

	printBinary32(granularity, "granularity");
}

uint32_t getBaseAddress(GDT* gdt, size_t index) {
	/*
	Using magic values cause it somehow cleaner
	*/

	SegmentDescriptor sd = gdt->segmentsInfo[index];

	uint32_t base_0_15 = getBits32(sd.lower, 31, 16);

	uint32_t base_16_23 = getBits32(sd.higher, 7, 0);

	uint32_t base_24_31 = getBits32(sd.higher, 31, 24);

	uint32_t base_0_15_shifted = base_0_15 << 0; // get optimised. But i prefer clarity
	uint32_t base_16_23_shifted = base_16_23 << 16;
	uint32_t base_24_31_shifted = base_24_31 << 24;

	uint32_t baseAddress = base_0_15_shifted | base_16_23_shifted | base_24_31_shifted;

	return baseAddress;
}

void setBaseAddress(GDT* gdt, size_t index, uint32_t baseAddress) {

	SegmentDescriptor* sd = &gdt->segmentsInfo[index];

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
