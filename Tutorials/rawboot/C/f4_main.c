#include "f2_string32.h"
#include "f3_segment_descriptor_internals32.h"
#include <stdio.h>

int main(void) {

	printf("\n\n\n-----Start of Program------\n\n\n");

	// test();
	// exit(0);
	// test2();

	GDT gdt = {0};
	printf("\n\n====== Start OF SETTING ======\n");

	setType(&gdt, 5, 0b100);
	setDescriptorTypeS(&gdt, 5, 0b1);
	setPriviledgeDPL(&gdt, 5, 0b11); // system services. ring 2
	setPresent(&gdt, 5, 0b0);
	setSegmentLimit(&gdt, 5, 0xba2ed);
	setAVL(&gdt, 5, 0);
	setLongMode(&gdt, 5, 1);
	setDefaultOperationSize(&gdt, 5, 0);
	setGranularity(&gdt, 5, 0);
	setBaseAddress(&gdt, 5, 0xfedc1234);

	printf("====== END OF SETTING ======\n\n\n");

	printBinary32(gdt.segmentsInfo[5].higher, "higher");
	printBinary32(gdt.segmentsInfo[5].lower, "lower");

	printf("\n");

	printf("----- Parsed Fields -----\n");
	uint32_t baseAddress = getBaseAddress(&gdt, 5);
	printBinary32(baseAddress, "Base Address");

	printBinary32(getType(&gdt, 5), "Type (bits 8-11)");
	printf("DescriptorTypeS = %u\n", getDescriptorTypeS(&gdt, 5));
	printBinary32(getPriviledgeDPL(&gdt, 5), "DPL (bits 13-14)");
	printf("Present = %u\n", getPresent(&gdt, 5));
	printf("AVL = %u\n", getAVL(&gdt, 5));
	printBinary32(getSegmentLimit(&gdt, 5), "SegmentLimit (bits 0-19)");
	printf("LongMode = %u\n", getLongMode(&gdt, 5));
	printf("DefaultOperationSize = %u\n", getDefaultOperationSize(&gdt, 5));
	printf("Granularity = %u\n", getGranularity(&gdt, 5));
	printBinary32(getBaseAddress(&gdt, 5), "BaseAddress (bits 0-31)");
}
