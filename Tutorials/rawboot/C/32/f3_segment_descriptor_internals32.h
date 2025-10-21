#pragma once
#include <stddef.h>
#include <stdint.h>

#define SD_COUNT 8192 // 2^13.
/*in the gdtr, there's 13 bits for table index, 2 for priviledge and 1 for gdt/ldt*/
typedef struct {
	uint32_t lower;
	uint32_t higher;
} SegmentDescriptor32;

typedef struct {
	SegmentDescriptor32 segmentsInfo[SD_COUNT];
} GDT32;

// Type field
uint8_t getType32(GDT32* gdt, size_t index);
void setType32(GDT32* gdt, size_t index, uint8_t type);

// Descriptor Type (S)
uint8_t getDescriptorTypeS32(GDT32* gdt, size_t index);
void setDescriptorTypeS32(GDT32* gdt, size_t index, uint8_t descriptorTypeS);

// Descriptor Privilege Level (DPL)
uint8_t getPriviledgeDPL32(GDT32* gdt, size_t index);
void setPriviledgeDPL32(GDT32* gdt, size_t index, uint8_t priviledgeDPL);

// Present bit
uint8_t getPresent32(GDT32* gdt, size_t index);
void setPresent32(GDT32* gdt, size_t index, uint8_t presentP);

// Segment Limit
uint32_t getSegmentLimit32(GDT32* gdt, size_t index);
void setSegmentLimit32(GDT32* gdt, size_t index, uint32_t segmentLimit);

// Available for system software (AVL)
uint8_t getAVL32(GDT32* gdt, size_t index);
void setAVL32(GDT32* gdt, size_t index, uint8_t avl);

// Long Mode (L)
uint8_t getLongMode32(GDT32* gdt, size_t index);
void setLongMode32(GDT32* gdt, size_t index, uint8_t L);

// Default Operation Size (D/B)
uint8_t getDefaultOperationSize32(GDT32* gdt, size_t index);
void setDefaultOperationSize32(GDT32* gdt, size_t index, uint8_t DsB);

// Granularity
uint8_t getGranularity32(GDT32* gdt, size_t index);
void setGranularity32(GDT32* gdt, size_t index, uint8_t granularity);

// Base Address
uint32_t getBaseAddress32(GDT32* gdt, size_t index);
void setBaseAddress32(GDT32* gdt, size_t index, uint32_t baseAddress);

// Test function
void test(void);

// Bit positions for GDT segment descriptor fields
static const uint8_t TYPE_START_32 = 8;
static const uint8_t TYPE_END_32 = 11;

static const uint8_t DESCRIPTOR_TYPE_S_32 = 12;

static const uint8_t DPL_START_32 = 13;
static const uint8_t DPL_END_32 = 14;

static const uint8_t PRESENT_32 = 15;

static const uint8_t SEGMENT_LIMIT_START_1_32 = 0;
static const uint8_t SEGMENT_LIMIT_END_1_32 = 15;
static const uint8_t SEGMENT_LIMIT_START_2_32 = 16;
static const uint8_t SEGMENT_LIMIT_END_2_32 = 19;

static const uint8_t AVL_32 = 20;
static const uint8_t LONG_MODE_32 = 21;
static const uint8_t DEFAULT_OPERATION_SIZE_32 = 22;
static const uint8_t GRANULARITY_32 = 23;

// Base address positions within descriptor (higher and lower parts)
// These are present as documentation, but won't be used, since the indexing makes it weird
static const uint8_t BASE_ADDRESS_LOWER_START_32 = 16;
static const uint8_t BASE_ADDRESS_LOWER_END_32 = 31;

static const uint8_t BASE_ADDRESS_HIGHER_START_1_32 = 0;
static const uint8_t BASE_ADDRESS_HIGHER_END_1_32 = 7;
static const uint8_t BASE_ADDRESS_HIGHER_START_2_32 = 24;
static const uint8_t BASE_ADDRESS_HIGHER_END_2_32 = 31;

// Base address logical indices (for get/set helpers)
static const uint8_t BASE_ADDRESS_INDEX_START_1_32 = 0;
static const uint8_t BASE_ADDRESS_INDEX_END_1_32 = 15;
static const uint8_t BASE_ADDRESS_INDEX_START_2_32 = 16;
static const uint8_t BASE_ADDRESS_INDEX_END_2_32 = 23;
static const uint8_t BASE_ADDRESS_INDEX_START_3_32 = 24;
static const uint8_t BASE_ADDRESS_INDEX_END_3_32 = 31;
