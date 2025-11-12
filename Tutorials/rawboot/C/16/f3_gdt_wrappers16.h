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
	uint8_t p_dpl_s_type;           // access
	uint8_t g_db_l_avl_segmentHigh; // flags + segmentHigh
	uint8_t baseAddressHigh8;
} SegmentDescriptor;
// Low is for 16 bit. Lower, Middle and Higher is for 8 bit

typedef struct {
	SegmentDescriptor segmentsInfo[SD_COUNT];
} GDT;

// Type field
uint8_t getType(GDT* gdt, size16_t index);
void setType(GDT* gdt, size16_t index, uint8_t type);

// Descriptor Type (S)
uint8_t getDescriptorTypeS(GDT* gdt, size16_t index);
void setDescriptorTypeS(GDT* gdt, size16_t index, uint8_t descriptorTypeS);

// Descriptor Privilege Level (DPL)
uint8_t getPrivilegeDPL(GDT* gdt, size16_t index);
void setPrivilegeDPL(GDT* gdt, size16_t index, uint8_t privilegeDPL);

// Present bit
uint8_t getPresent(GDT* gdt, size16_t index);
void setPresent(GDT* gdt, size16_t index, uint8_t presentP);

// Available for system software (AVL)
uint8_t getAVL(GDT* gdt, size16_t index);
void setAVL(GDT* gdt, size16_t index, uint8_t avl);

// Long Mode (L)
uint8_t getLongMode(GDT* gdt, size16_t index);
void setLongMode(GDT* gdt, size16_t index, uint8_t L);

// Default Operation Size (D/B)
uint8_t getDefaultOperationSize(GDT* gdt, size16_t index);
void setDefaultOperationSize(GDT* gdt, size16_t index, uint8_t DsB);

// Granularity
uint8_t getGranularity(GDT* gdt, size16_t index);
void setGranularity(GDT* gdt, size16_t index, uint8_t granularity);

// Base Address
uint16_t getBaseAddressLow16(GDT* gdt, size16_t index);
void setBaseAddressLow16(GDT* gdt, size16_t index, uint16_t baseAddressLow16);

uint8_t getBaseAddressMiddle8(GDT* gdt, size16_t index);
void setBaseAddressMiddle8(GDT* gdt, size16_t index, uint8_t baseAddressMiddle8);

uint8_t getBaseAddressHigh8(GDT* gdt, size16_t index);
void setBaseAddressHigh8(GDT* gdt, size16_t index, uint8_t baseAddressHigh8);

uint16_t getBaseAddressHigh16(GDT* gdt, size16_t index);
void setBaseAddressHigh16(GDT* gdt, size16_t index, uint16_t baseAddressHigh16);

uint16_t getSegmentLimitLow16(GDT* gdt, size16_t index);
void setSegmentLimitLow16(GDT* gdt, size16_t index, uint16_t segmentLow16);
void setSegmentLimitHigh4(GDT* gdt, size16_t index, uint8_t segmentHigh4);
uint8_t getSegmentLimitHigh4(GDT* gdt, size16_t index);

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
