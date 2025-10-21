#include "f3_gdt_wrappers16.h"

uint32_t getSegmentLimitLow20_E(GDT* gdt, size16_t index);
uint32_t getBaseAddress32_E(GDT* gdt, size16_t index);

void setSegmentLimitLow20_E(GDT* gdt, size16_t index, uint32_t segmentLimit);
void setBaseAddress32_E(GDT* gdt, size16_t index, uint32_t baseAddress);
