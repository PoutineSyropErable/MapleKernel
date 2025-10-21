#pragma once
#include <stdint.h>

#define size16_t uint16_t

#define SD_COUNT 8192 // 2^13.
/*in the gdtr, there's 13 bits for table index, 2 for privilege and 1 for gdt/ldt*/
typedef struct __attribute__((packed)) {
	// low 32
	uint16_t segmentLow16;
	uint16_t baseAddressLow16;

	// high 32
	uint8_t baseAddressMiddle8;
	uint8_t p_dpl_s_type; // access information
	uint8_t g_db_l_avl_segmentHigh;
	uint8_t baseAddressHigh8;
} SegmentDescriptor;
// Low is for 16 bit. Lower, Middle and Higher is for 8 bit

typedef struct {
	SegmentDescriptor segmentsInfo[SD_COUNT];
} GDT32;

// Type field
uint8_t getType32(GDT32* gdt, size16_t index);
void setType32(GDT32* gdt, size16_t index, uint8_t type);

// Descriptor Type (S)
uint8_t getDescriptorTypeS32(GDT32* gdt, size16_t index);
void setDescriptorTypeS32(GDT32* gdt, size16_t index, uint8_t descriptorTypeS);

// Descriptor Privilege Level (DPL)
uint8_t getPrivilegeDPL(GDT32* gdt, size16_t index);
void setPrivilegeDPL(GDT32* gdt, size16_t index, uint8_t privilegeDPL);

// Present bit
uint8_t getPresent32(GDT32* gdt, size16_t index);
void setPresent32(GDT32* gdt, size16_t index, uint8_t presentP);

// Available for system software (AVL)
uint8_t getAVL32(GDT32* gdt, size16_t index);
void setAVL32(GDT32* gdt, size16_t index, uint8_t avl);

// Long Mode (L)
uint8_t getLongMode32(GDT32* gdt, size16_t index);
void setLongMode32(GDT32* gdt, size16_t index, uint8_t L);

// Default Operation Size (D/B)
uint8_t getDefaultOperationSize32(GDT32* gdt, size16_t index);
void setDefaultOperationSize32(GDT32* gdt, size16_t index, uint8_t DsB);

// Granularity
uint8_t getGranularity32(GDT32* gdt, size16_t index);
void setGranularity32(GDT32* gdt, size16_t index, uint8_t granularity);

// Base Address
uint16_t getBaseAddressLow16(GDT32* gdt, size16_t index);
void setBaseAddressLow16(GDT32* gdt, size16_t index, uint16_t baseAddressLow16);

uint8_t getBaseAddressMiddle8(GDT32* gdt, size16_t index);
void setBaseAddressMiddle8(GDT32* gdt, size16_t index, uint8_t baseAddressMiddle8);

uint8_t getBaseAddressHigh8(GDT32* gdt, size16_t index);
void setBaseAddressHigh8(GDT32* gdt, size16_t index, uint8_t baseAddressHigh8);

uint16_t getBaseAddressHigh16(GDT32* gdt, size16_t index);
void setBaseAddressHigh16(GDT32* gdt, size16_t index, uint16_t baseAddressHigh16);

uint16_t getSegmentLimitLow16(GDT32* gdt, size16_t index);
void setSegmentLimitLow16(GDT32* gdt, size16_t index, uint16_t segmentLow16);
void setSegmentLimitHigh4(GDT32* gdt, size16_t index, uint8_t segmentHigh);
uint8_t getSegmentLimitHigh4(GDT32* gdt, size16_t index);

// Test function
void test(void);

// Bit positions for GDT segment descriptor fields
static const uint8_t TYPE_START = 0;
static const uint8_t TYPE_END = 3;

static const uint8_t DESCRIPTOR_TYPE_S = 4;

static const uint8_t DPL_START = 5;
static const uint8_t DPL_END = 6;

static const uint8_t PRESENT = 7;

static const uint8_t SEGMENT_LIMIT_LOW_START_1 = 0;
static const uint8_t SEGMENT_LIMIT_LOW_END = 15;
static const uint8_t SEGMENT_LIMIT_HIGH_START = 0;
static const uint8_t SEGMENT_LIMIT_HIGH_END = 3;

static const uint8_t AVL = 4;
static const uint8_t LONG_MODE = 5;
static const uint8_t DEFAULT_OPERATION_SIZE = 6;
static const uint8_t GRANULARITY = 7;
