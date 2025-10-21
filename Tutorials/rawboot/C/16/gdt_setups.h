#pragma once
#include "f3_gdt_wrappers16.h"

void setGDTElementKernelCode(GDT* gdt, size16_t index, uint16_t baseAddressHigh16, uint16_t baseAddressLow16, uint8_t segmentLimitHigh4, uint16_t segmentLimitLow16);
void setGDTElementKernelData(GDT* gdt, size16_t index, uint16_t baseAddressHigh16, uint16_t baseAddressLow16, uint8_t segmentLimitHigh4, uint16_t segmentLimitLow16);
void setGDTElementKernelStack(GDT* gdt, size16_t index, uint16_t baseAddressHigh16, uint16_t baseAddressLow16, uint8_t segmentLimitHigh4, uint16_t segmentLimitLow16);

void setFirstGDTEntries(GDT* gdt);

static void clear_gdt_entry(SegmentDescriptor* sd);
void initializeGDT(GDT* gdt);
