#include "16/f2_string16.h"
#include "16/f3_1_gdt_wrappers_extension_32.h"
#include "16/f3_gdt_wrappers16.h"
#include "32/f1_binary_operation32.h"
#include "32/f2_string32.h"
#include <stdio.h>

void printGDTEntry(GDT* gdt, size16_t index) {
	printf("=== GDT Entry %u ===\n", index);

	printf("Type: %u\n", getType(gdt, index));
	printf("DescriptorTypeS: %u\n", getDescriptorTypeS(gdt, index));
	printf("DPL: %u\n", getPrivilegeDPL(gdt, index));
	printf("Present: %u\n", getPresent(gdt, index));

	printf("AVL: %u\n", getAVL(gdt, index));
	printf("LongMode: %u\n", getLongMode(gdt, index));
	printf("DefaultOperationSize: %u\n", getDefaultOperationSize(gdt, index));
	printf("Granularity: %u\n", getGranularity(gdt, index));

	printf("SegmentLimit: 0x%05X\n", getSegmentLimitLow20_E(gdt, index));

	printf("BaseAddress: 0x%08X\n", getBaseAddress32_E(gdt, index));

	printf("\n");
}

int main(void) {

	printf("\n\n\n-----Start of Program------\n\n\n");

	// test();
	// exit(0);
	// test2();

	GDT gdt = {0};
	printf("\n\n====== Start OF SETTING ======\n");

	setType(&gdt, 5, 0b100);
	setDescriptorTypeS(&gdt, 5, 0b1);
	setPrivilegeDPL(&gdt, 5, 0b11); // system services. ring 2
	setPresent(&gdt, 5, 0b0);
	setSegmentLimitLow16(&gdt, 5, 0xa2ed);
	setSegmentLimitHigh4(&gdt, 5, 0xb);
	setAVL(&gdt, 5, 0);
	setLongMode(&gdt, 5, 1);
	setDefaultOperationSize(&gdt, 5, 0);
	setGranularity(&gdt, 5, 0);
	setBaseAddressLow16(&gdt, 5, 0x1234);
	setBaseAddressHigh16(&gdt, 5, 0xfedc);

	printf("====== END OF SETTING ======\n\n\n");

	// printBinary32(gdt.segmentsInfo[5].higher, "higher");
	// printBinary32(gdt.segmentsInfo[5].lower, "lower");

	printf("\n");
	// Print the 5th GDT entry
	printGDTEntry(&gdt, 5);
}
