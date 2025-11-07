#pragma once
#include <stddef.h>
#include <stdint.h>

#define SD_COUNT 5 // 2^13.
/*in the gdtr, there's 13 bits for table index, 2 for priviledge and 1 for gdt/ldt*/
// 8912 is 2^13
typedef struct __attribute__((packed)) {
	uint32_t lower;
	uint32_t higher;
} SegmentDescriptor;

typedef struct {
	SegmentDescriptor segmentsInfo[SD_COUNT];
} GDT;

// Type field
uint8_t getType(SegmentDescriptor* sd);
void setType(SegmentDescriptor* sd, uint8_t type);

// Descriptor Type (S)
uint8_t getDescriptorTypeS(SegmentDescriptor* sd);
void setDescriptorTypeS(SegmentDescriptor* sd, uint8_t descriptorTypeS);

// Descriptor Privilege Level (DPL)
uint8_t getPriviledgeDPL(SegmentDescriptor* sd);
void setPriviledgeDPL(SegmentDescriptor* sd, uint8_t priviledgeDPL);

// Present bit
uint8_t getPresent(SegmentDescriptor* sd);
void setPresent(SegmentDescriptor* sd, uint8_t presentP);

// Segment Limit
uint32_t getSegmentLimit(SegmentDescriptor* sd);
void setSegmentLimit(SegmentDescriptor* sd, uint32_t segmentLimit);

// Available for system software (AVL)
uint8_t getAVL(SegmentDescriptor* sd);
void setAVL(SegmentDescriptor* sd, uint8_t avl);

// Long Mode (L)
uint8_t getLongMode(SegmentDescriptor* sd);
void setLongMode(SegmentDescriptor* sd, uint8_t L);

// Default Operation Size (D/B)
uint8_t getDefaultOperationSize(SegmentDescriptor* sd);
void setDefaultOperationSize(SegmentDescriptor* sd, uint8_t DsB);

// Granularity
uint8_t getGranularity(SegmentDescriptor* sd);
void setGranularity(SegmentDescriptor* sd, uint8_t granularity);

// Base Address
uint32_t getBaseAddress(SegmentDescriptor* sd);
void setBaseAddress(SegmentDescriptor* sd, uint32_t baseAddress);

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
