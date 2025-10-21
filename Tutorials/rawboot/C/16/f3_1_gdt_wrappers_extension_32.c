#include "f3_1_gdt_wrappers_extension_32.h"
#include "stdio.h"

// ---------------- 32-bit extended segment limit ----------------
uint32_t getSegmentLimitLow20_E(GDT* gdt, size16_t index) {
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];

	uint16_t low16 = getSegmentLimitLow16(gdt, index);
	uint8_t high4 = getSegmentLimitHigh4(gdt, index);

	uint32_t segmentLimit = ((uint32_t)high4 << 16) | low16;
	return segmentLimit;
}

void setSegmentLimitLow20_E(GDT* gdt, size16_t index, uint32_t segmentLimit) {
	if (segmentLimit > 0xFFFFF) {
		printf("Warning: segment limit exceeds 20 bits, truncating.\n");
		segmentLimit &= 0xFFFFF;
	}

	uint16_t low16 = segmentLimit & 0xFFFF;
	uint8_t high4 = (segmentLimit >> 16) & 0xF;

	setSegmentLimitLow16(gdt, index, low16);
	setSegmentLimitHigh4(gdt, index, high4);
}

// ---------------- 32-bit extended base address ----------------
uint32_t getBaseAddress32_E(GDT* gdt, size16_t index) {
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];

	uint16_t low16 = getBaseAddressLow16(gdt, index);
	uint8_t middle8 = getBaseAddressMiddle8(gdt, index);
	uint8_t high8 = getBaseAddressHigh8(gdt, index);

	uint32_t base = ((uint32_t)high8 << 24) | ((uint32_t)middle8 << 16) | low16;
	return base;
}

void setBaseAddress32_E(GDT* gdt, size16_t index, uint32_t baseAddress) {
	uint16_t low16 = baseAddress & 0xFFFF;
	uint8_t middle8 = (baseAddress >> 16) & 0xFF;
	uint8_t high8 = (baseAddress >> 24) & 0xFF;

	setBaseAddressLow16(gdt, index, low16);
	setBaseAddressMiddle8(gdt, index, middle8);
	setBaseAddressHigh8(gdt, index, high8);
}
