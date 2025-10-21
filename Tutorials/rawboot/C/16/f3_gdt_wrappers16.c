#include "stdio.h"

#include "f1_binary_operation16.h"
#include "f2_string16.h"
#include "f3_gdt_wrappers16.h"

GDT mainGDT;

uint8_t getType(GDT* gdt, size16_t index) {
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];

	uint8_t type = getBits8(sd->p_dpl_s_type, TYPE_END, TYPE_START);

	return type;
}

void setType(GDT* gdt, size16_t index, uint8_t type) {
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];

	setBitsModify8(&sd->p_dpl_s_type, type, TYPE_END, TYPE_START);

	printBinary8(type, "type");
}

uint8_t getDescriptorTypeS(GDT* gdt, size16_t index) {
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];

	uint8_t S = getBits8(sd->p_dpl_s_type, DESCRIPTOR_TYPE_S, DESCRIPTOR_TYPE_S);
	return S;
}

void setDescriptorTypeS(GDT* gdt, size16_t index, uint8_t descriptorTypeS) {
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];

	setBitsModify8(&sd->p_dpl_s_type, descriptorTypeS, DESCRIPTOR_TYPE_S, DESCRIPTOR_TYPE_S);
	printBinary8(descriptorTypeS, "descriptorTypeS");
}

// Descriptor Privilege Level
uint8_t getPrivilegeDPL(GDT* gdt, size16_t index) {
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];

	uint8_t DPL = getBits8(sd->p_dpl_s_type, DPL_END, DPL_START);
	return DPL;
}

// Descriptor Privilege Level
void setPrivilegeDPL(GDT* gdt, size16_t index, uint8_t privilegeDPL) {
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];

	setBitsModify8(&sd->p_dpl_s_type, privilegeDPL, DPL_END, DPL_START);

	printBinary8(privilegeDPL, "privilegeDPL");
}

uint8_t getPresent(GDT* gdt, size16_t index) {
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];

	uint8_t P = getBits8(sd->p_dpl_s_type, PRESENT, PRESENT);
	return P;
}

void setPresent(GDT* gdt, size16_t index, uint8_t presentP) {
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];

	setBitsModify8(&sd->p_dpl_s_type, presentP, PRESENT, PRESENT);

	printBinary8(presentP, "presentP");
}

uint16_t getSegmentLimitLow16(GDT* gdt, size16_t index) {
	return gdt->segmentsInfo[index].segmentLow16;
}

void setSegmentLimitLow16(GDT* gdt, size16_t index, uint16_t segmentLow16) {
	printBinary16(segmentLow16, "SegmentLow16");
	gdt->segmentsInfo[index].segmentLow16 = segmentLow16;
}

void setSegmentLimitHigh4(GDT* gdt, size16_t index, uint8_t segmentHigh4) {
	printBinary16(segmentHigh4, "SegmentHigh4");
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];

	if (segmentHigh4 > 0xF) {
		printf("This is bad, it should be smaller then 16\n");
		segmentHigh4 = 0xF & segmentHigh4;
	}

	setBitsModify8(&sd->g_db_l_avl_segmentHigh, segmentHigh4, SEGMENT_LIMIT_HIGH_END, SEGMENT_LIMIT_HIGH_START);
}

uint8_t getSegmentLimitHigh4(GDT* gdt, size16_t index) {
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];

	uint8_t segmentHigh = getBits8(sd->g_db_l_avl_segmentHigh, SEGMENT_LIMIT_HIGH_END, SEGMENT_LIMIT_HIGH_START);
	return segmentHigh;
}

// Available for system software
uint8_t getAVL(GDT* gdt, size16_t index) {
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];

	uint8_t avl = getBits8(sd->g_db_l_avl_segmentHigh, AVL, AVL);

	return avl;
}

void setAVL(GDT* gdt, size16_t index, uint8_t avl) {
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];

	setBitsModify8(&sd->g_db_l_avl_segmentHigh, avl, AVL, AVL);

	printBinary8(avl, "avl");
}

uint8_t getLongMode(GDT* gdt, size16_t index) {
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];

	uint8_t L = getBits8(sd->g_db_l_avl_segmentHigh, LONG_MODE, LONG_MODE);

	return L;
}

void setLongMode(GDT* gdt, size16_t index, uint8_t L) {
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];

	setBitsModify8(&sd->g_db_l_avl_segmentHigh, L, LONG_MODE, LONG_MODE);

	printBinary8(L, "L");
}

uint8_t getDefaultOperationSize(GDT* gdt, size16_t index) {
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];

	// D slash B. D/B. D or B
	uint8_t DsB = getBits8(sd->g_db_l_avl_segmentHigh, DEFAULT_OPERATION_SIZE, DEFAULT_OPERATION_SIZE);

	return DsB;
}

void setDefaultOperationSize(GDT* gdt, size16_t index, uint8_t DsB) {
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];

	setBitsModify8(&sd->g_db_l_avl_segmentHigh, DsB, DEFAULT_OPERATION_SIZE, DEFAULT_OPERATION_SIZE);

	printBinary8(DsB, "DsB");
}

uint8_t getGranularity(GDT* gdt, size16_t index) {
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];

	uint8_t granularity = getBits8(sd->g_db_l_avl_segmentHigh, GRANULARITY, GRANULARITY);

	return granularity;
}

void setGranularity(GDT* gdt, size16_t index, uint8_t granularity) {
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];

	setBitsModify8(&sd->g_db_l_avl_segmentHigh, granularity, GRANULARITY, GRANULARITY);

	printBinary8(granularity, "granularity");
}

void setBaseAddressLow16(GDT* gdt, size16_t index, uint16_t baseAddressLow16) {
	printBinary16(baseAddressLow16, "baseAddressLow16");
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];
	sd->baseAddressLow16 = baseAddressLow16;
}

uint16_t getBaseAddressLow16(GDT* gdt, size16_t index) {
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];
	return sd->baseAddressLow16;
}

void setBaseAddressMiddle8(GDT* gdt, size16_t index, uint8_t baseAddressMiddle8) {
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];
	sd->baseAddressMiddle8 = baseAddressMiddle8;
}

uint8_t getBaseAddressMiddle8(GDT* gdt, size16_t index) {
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];
	return sd->baseAddressMiddle8;
}

void setBaseAddressHigh8(GDT* gdt, size16_t index, uint8_t baseAddressHigh8) {
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];
	sd->baseAddressHigh8 = baseAddressHigh8;
}

uint8_t getBaseAddressHigh8(GDT* gdt, size16_t index) {
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];
	return sd->baseAddressHigh8;
}

void setBaseAddressHigh16(GDT* gdt, size16_t index, uint16_t baseAddressHigh16) {
	printBinary16(baseAddressHigh16, "baseAddressHigh16");
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];
	uint8_t baseAddressMiddle8 = baseAddressHigh16 & 0xFF;
	uint8_t baseAddressHigh8 = baseAddressHigh16 >> 8;

	sd->baseAddressMiddle8 = baseAddressMiddle8;
	sd->baseAddressHigh8 = baseAddressHigh8;
}

uint16_t getBaseAddressHigh16(GDT* gdt, size16_t index) {
	SegmentDescriptor* sd = &gdt->segmentsInfo[index];

	uint8_t baseAddressMiddle8 = sd->baseAddressMiddle8;
	uint8_t baseAddressHigh8 = sd->baseAddressHigh8;
	uint16_t baseAddressHigh16 = baseAddressHigh8 << 8 | baseAddressMiddle8;
	return baseAddressHigh16;
}
