#pragma once
#include <stdint.h>

namespace multicore_gdt
{

struct FS_CONTENT
{
	uint8_t core_id;
};

struct GS_CONTENT
{
	uint32_t other_stuff;
};

struct FS_GS
{
	struct FS_CONTENT fs;
	struct GS_CONTENT gs;
};

enum class fs_or_gs
{
	fs = 0,
	gs = 1,
};

void set_fs_or_segment_selector(uint8_t core_id, enum fs_or_gs segment_selector);

void set_fs_or_segment_selector(uint8_t core_id, bool gs_not_fs);
void init_multicore_gdt();

// Get the current core's FS_CONTENT struct
static inline struct FS_CONTENT *get_fs_struct(void)
{
	struct FS_CONTENT *ptr;
	// Move 32-bit value from FS:0 (offset 0 in FS segment)
	__asm__ volatile("movl %%fs:0, %0" : "=r"(ptr));
	return ptr;
}

// Get the current core's GS_CONTENT struct
static inline struct GS_CONTENT *get_gs_struct(void)
{
	struct GS_CONTENT *ptr;
	// Move 32-bit value from GS:0 (offset 0 in GS segment)
	__asm__ volatile("movl %%gs:0, %0" : "=r"(ptr));
	return ptr;
}

} // namespace multicore_gdt
