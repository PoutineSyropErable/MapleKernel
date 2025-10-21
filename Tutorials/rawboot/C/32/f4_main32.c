#include "f2_string32.h"
#include "f3_segment_descriptor_internals32.h"
#include <stdio.h>

int main(void) {

	printf("\n\n\n-----Start of Program------\n\n\n");

	// test();
	// exit(0);
	// test2();

	GDT32 gdt = {0};
	printf("\n\n====== Start OF SETTING ======\n");

	setType32(&gdt, 5, 0b100);
	setDescriptorTypeS32(&gdt, 5, 0b1);
	setPriviledgeDPL32(&gdt, 5, 0b11); // system services. ring 2
	setPresent32(&gdt, 5, 0b0);
	setSegmentLimit32(&gdt, 5, 0xba2ed);
	setAVL32(&gdt, 5, 0);
	setLongMode32(&gdt, 5, 1);
	setDefaultOperationSize32(&gdt, 5, 0);
	setGranularity32(&gdt, 5, 0);
	setBaseAddress32(&gdt, 5, 0xfedc1234);

	printf("====== END OF SETTING ======\n\n\n");

	printBinary32(gdt.segmentsInfo[5].higher, "higher");
	printBinary32(gdt.segmentsInfo[5].lower, "lower");

	printf("\n");

	printf("----- Parsed Fields -----\n");
	uint32_t baseAddress = getBaseAddress32(&gdt, 5);
	printBinary32(baseAddress, "Base Address");

	printBinary32(getType32(&gdt, 5), "Type (bits 8-11)");
	printf("DescriptorTypeS = %u\n", getDescriptorTypeS32(&gdt, 5));
	printBinary32(getPriviledgeDPL32(&gdt, 5), "DPL (bits 13-14)");
	printf("Present = %u\n", getPresent32(&gdt, 5));
	printf("AVL = %u\n", getAVL32(&gdt, 5));
	printBinary32(getSegmentLimit32(&gdt, 5), "SegmentLimit (bits 0-19)");
	printf("LongMode = %u\n", getLongMode32(&gdt, 5));
	printf("DefaultOperationSize = %u\n", getDefaultOperationSize32(&gdt, 5));
	printf("Granularity = %u\n", getGranularity32(&gdt, 5));
	printBinary32(getBaseAddress32(&gdt, 5), "BaseAddress (bits 0-31)");
}
