#pragma once
#include <stddef.h>
#include <stdint.h>

#define SD_COUNT 8192 // 2^13.
/*in the gdtr, there's 13 bits for table index, 2 for priviledge and 1 for gdt/ldt*/
typedef struct {
	uint32_t lower;
	uint32_t higher;
} SegmentDescriptor;

typedef struct {
	SegmentDescriptor segmentsInfo[SD_COUNT];
} GDT;

// Type field
uint8_t getType(GDT* gdt, size_t index);
void setType(GDT* gdt, size_t index, uint8_t type);

// Descriptor Type (S)
uint8_t getDescriptorTypeS(GDT* gdt, size_t index);
void setDescriptorTypeS(GDT* gdt, size_t index, uint8_t descriptorTypeS);

// Descriptor Privilege Level (DPL)
uint8_t getPriviledgeDPL(GDT* gdt, size_t index);
void setPriviledgeDPL(GDT* gdt, size_t index, uint8_t priviledgeDPL);

// Present bit
uint8_t getPresent(GDT* gdt, size_t index);
void setPresent(GDT* gdt, size_t index, uint8_t presentP);

// Segment Limit
uint32_t getSegmentLimit(GDT* gdt, size_t index);
void setSegmentLimit(GDT* gdt, size_t index, uint32_t segmentLimit);

// Available for system software (AVL)
uint8_t getAVL(GDT* gdt, size_t index);
void setAVL(GDT* gdt, size_t index, uint8_t avl);

// Long Mode (L)
uint8_t getLongMode(GDT* gdt, size_t index);
void setLongMode(GDT* gdt, size_t index, uint8_t L);

// Default Operation Size (D/B)
uint8_t getDefaultOperationSize(GDT* gdt, size_t index);
void setDefaultOperationSize(GDT* gdt, size_t index, uint8_t DsB);

// Granularity
uint8_t getGranularity(GDT* gdt, size_t index);
void setGranularity(GDT* gdt, size_t index, uint8_t granularity);

// Base Address
uint32_t getBaseAddress(GDT* gdt, size_t index);
void setBaseAddress(GDT* gdt, size_t index, uint32_t baseAddress);

// Test function
void test(void);

// Bit positions for GDT segment descriptor fields
static const uint8_t TYPE_START = 8;
static const uint8_t TYPE_END = 11;

static const uint8_t DESCRIPTOR_TYPE_S = 12;

static const uint8_t DPL_START = 13;
static const uint8_t DPL_END = 14;

static const uint8_t PRESENT = 15;

static const uint8_t SEGMENT_LIMIT_START_1 = 0;
static const uint8_t SEGMENT_LIMIT_END_1 = 15;
static const uint8_t SEGMENT_LIMIT_START_2 = 16;
static const uint8_t SEGMENT_LIMIT_END_2 = 19;

static const uint8_t AVL = 20;
static const uint8_t LONG_MODE = 21;
static const uint8_t DEFAULT_OPERATION_SIZE = 22;
static const uint8_t GRANULARITY = 23;

// Base address positions within descriptor (higher and lower parts)
// These are present as documentation, but won't be used, since the indexing makes it weird
static const uint8_t BASE_ADDRESS_LOWER_START = 16;
static const uint8_t BASE_ADDRESS_LOWER_END = 31;

static const uint8_t BASE_ADDRESS_HIGHER_START_1 = 0;
static const uint8_t BASE_ADDRESS_HIGHER_END_1 = 7;
static const uint8_t BASE_ADDRESS_HIGHER_START_2 = 24;
static const uint8_t BASE_ADDRESS_HIGHER_END_2 = 31;

// Base address logical indices (for get/set helpers)
static const uint8_t BASE_ADDRESS_INDEX_START_1 = 0;
static const uint8_t BASE_ADDRESS_INDEX_END_1 = 15;
static const uint8_t BASE_ADDRESS_INDEX_START_2 = 16;
static const uint8_t BASE_ADDRESS_INDEX_END_2 = 23;
static const uint8_t BASE_ADDRESS_INDEX_START_3 = 24;
static const uint8_t BASE_ADDRESS_INDEX_END_3 = 31;
