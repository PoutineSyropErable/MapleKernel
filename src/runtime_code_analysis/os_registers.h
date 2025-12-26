#include "f3_segment_descriptor_internals.h"
#include <stddef.h>
#include <stdint.h>

typedef SegmentDescriptor GDT_ENTRY2;

typedef struct __attribute__((packed, aligned(4)))
{
	uint32_t lower;
	uint32_t higher;
} SegmentDescriptor1;

typedef struct __attribute__((packed, aligned(4)))
{
	uint16_t	limit;
	GDT_ENTRY2 *base;
} GDT_ROOT;
