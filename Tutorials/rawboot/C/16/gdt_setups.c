#include "f1_binary_operation16.h"
#include "f2_string16.h"
#include "f3_gdt_wrappers16.h"
#include "gdt_setups.h"

// Utility for clearing a descriptor
static void clear_gdt_entry(SegmentDescriptor* sd) {
	sd->baseAddressLow16 = 0;
	sd->baseAddressMiddle8 = 0;
	sd->baseAddressHigh8 = 0;

	sd->g_db_l_avl_segmentHigh = 0;
	sd->segmentLow16 = 0;
	sd->p_dpl_s_type = 0;
}

void setGDTElementKernelCode(GDT* gdt, size16_t index, uint16_t baseAddressHigh16, uint16_t baseAddressLow16, uint8_t segmentLimitHigh4, uint16_t segmentLimitLow16) {

	SegmentDescriptor* sd = &gdt->segmentsInfo[index];

	// Code = 1, Conforming=1/NonConforming, Read Permision, Accessed/Not Accesed

	// We want Code Here.
	// For Code:
	// 1, 0, 1, 0
	// It's code.
	// We want non conforming so that you can't escalate priviledge. (Security reasons)
	// We want read permissions, because execute only without read permissions is problematic.
	// ----- mov eax, [rip + something]. or switch case n: func_#n() would break it
	// And Accessed should be set to 0. (Software should never set it to 1. The cpu will set it to 1 when used for the first time)

	// This is just one byte. And value in the struct. So, we could manually set it.
	setType(gdt, index, 0b1010);       //
	setDescriptorTypeS(gdt, index, 1); // 1 for code/data segment. (Since this isn't a system segment (The GDT, IDT, ... are in system segment))
	setPrivilegeDPL(gdt, index, 0b00);
	setPresent(gdt, index, 1);

	setAVL(gdt, index, 0);                  // Not used right now
	setLongMode(gdt, index, 0);             // 32 bit mode
	setDefaultOperationSize(gdt, index, 1); // 32 bit default operation size
	setGranularity(gdt, index, 1);          // 4Kb chunk

	setBaseAddressHigh16(gdt, index, baseAddressHigh16);
	setBaseAddressLow16(gdt, index, baseAddressLow16);

	setSegmentLimitLow16(gdt, index, segmentLimitLow16);
	setSegmentLimitHigh4(gdt, index, segmentLimitHigh4);
}

void setGDTElementKernelData(GDT* gdt, size16_t index, uint16_t baseAddressHigh16, uint16_t baseAddressLow16, uint8_t segmentLimitHigh4, uint16_t segmentLimitLow16) {

	SegmentDescriptor* sd = &gdt->segmentsInfo[index];

	// DATA = 0, Expend Down=1, Write Permission, Accessed/Not Accesed

	// We want Data Here.
	// For Data:
	// 0, 0, 1, 0
	// It's code.
	// We want non conforming so that you can't escalate priviledge. (Security reasons)
	// We want read permissions, because execute only without read permissions is problematic.
	// ----- mov eax, [rip + something]. or switch case n: func_#n() would break it
	// And Accessed should be set to 0. (Software should never set it to 1. The cpu will set it to 1 when used for the first time)

	// This is just one byte. And value in the struct. So, we could manually set it.
	setType(gdt, index, 0b0010);       //
	setDescriptorTypeS(gdt, index, 1); // 1 for code/data segment. (Since this isn't a system segment (The GDT, IDT, ... are in system segment))
	setPrivilegeDPL(gdt, index, 0b00);
	setPresent(gdt, index, 1);

	setAVL(gdt, index, 0);                  // Not used right now
	setLongMode(gdt, index, 0);             // 32 bit mode
	setDefaultOperationSize(gdt, index, 1); // 32 bit default operation size
	setGranularity(gdt, index, 1);          // 4Kb chunk

	setBaseAddressHigh16(gdt, index, baseAddressHigh16);
	setBaseAddressLow16(gdt, index, baseAddressLow16);

	setSegmentLimitLow16(gdt, index, segmentLimitLow16);
	setSegmentLimitHigh4(gdt, index, segmentLimitHigh4);
}

void setGDTElementKernelStack(GDT* gdt, size16_t index, uint16_t baseAddressHigh16, uint16_t baseAddressLow16, uint8_t segmentLimitHigh4, uint16_t segmentLimitLow16) {

	SegmentDescriptor* sd = &gdt->segmentsInfo[index];

	// DATA = 0, Expend Down=1, Write Permission, Accessed/Not Accesed

	// 0, 0, 1, 0
	// It's code.
	// We want non conforming so that you can't escalate priviledge. (Security reasons)
	// We want read permissions, because execute only without read permissions is problematic.
	// ----- mov eax, [rip + something]. or switch case n: func_#n() would break it
	// And Accessed should be set to 0. (Software should never set it to 1. The cpu will set it to 1 when used for the first time)

	// This is just one byte. And value in the struct. So, we could manually set it.
	setType(gdt, index, 0b0110);       //
	setDescriptorTypeS(gdt, index, 1); // 1 for code/data segment. (Since this isn't a system segment (The GDT, IDT, ... are in system segment))
	setPrivilegeDPL(gdt, index, 0b00);
	setPresent(gdt, index, 1);

	setAVL(gdt, index, 0);                  // Not used right now
	setLongMode(gdt, index, 0);             // 32 bit mode
	setDefaultOperationSize(gdt, index, 1); // 32 bit default operation size
	setGranularity(gdt, index, 1);          // 4Kb chunk

	setBaseAddressHigh16(gdt, index, baseAddressHigh16);
	setBaseAddressLow16(gdt, index, baseAddressLow16);

	setSegmentLimitLow16(gdt, index, segmentLimitLow16);
	setSegmentLimitHigh4(gdt, index, segmentLimitHigh4);
}

void setFirstGDTEntries(GDT* gdt) {
	// The first element of the GDT is set with zero so we don't touch it.

	setGDTElementKernelCode(gdt, 1, 0x0000, 0x0000, 0x0, 0x3FF);  // start at 0, end at 4mb -1byte
	setGDTElementKernelData(gdt, 2, 0x0000, 0x0000, 0x0, 0x800);  // start at 0, end at 8mb
	setGDTElementKernelStack(gdt, 3, 0x0000, 0x0000, 0x0, 0x800); // start at 0, end at 8mb
	// Though, I'll put the code at specific address
	// Code will start at 0
	// Data will start at 4mb and end at 8mb
	// %esp will be placed at 8mb

	// MMIO segment, Covers the whole ram
	setGDTElementKernelData(gdt, 4, 0x0000, 0x0000, 0xF, 0xFFFF);
}

void initializeGDT(GDT* gdt) {

	// 0 All Entries
	for (size16_t i = 0; i < SD_COUNT; i++) {
		clear_gdt_entry(&gdt->segmentsInfo[i]);
	}

	setFirstGDTEntries(gdt);
}
