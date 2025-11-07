#include "add16_wrapper.h"
#include "f3_segment_descriptor_internals.h"
#include <stddef.h>
#include <stdint.h>

uint16_t get_ss_selector() {
	uint16_t ss_value;
	__asm__ volatile(
	    ".intel_syntax noprefix\n" // switch to Intel syntax
	    "mov %0, ss\n"             // Intel-style mov
	    ".att_syntax prefix\n"     // switch back to AT&T (default)
	    : "=r"(ss_value)           // output operand
	);
	return ss_value;
}

uint16_t get_cs_selector() {
	uint16_t cs_value;
	__asm__ volatile(
	    ".intel_syntax noprefix\n" // switch to Intel syntax
	    "mov %0, cs\n"             // Intel-style mov
	    ".att_syntax prefix\n"     // switch back to AT&T (default)
	    : "=r"(cs_value)           // output operand
	);
	return cs_value;
}

uint16_t get_es_selector() {
	uint16_t es_value;
	__asm__ volatile(
	    ".intel_syntax noprefix\n" // switch to Intel syntax
	    "mov %0, es\n"             // Intel-style mov
	    ".att_syntax prefix\n"     // switch back to AT&T (default)
	    : "=r"(es_value)           // output operand
	);
	return es_value;
}

uint16_t get_fs_selector() {
	uint16_t fs_value;
	__asm__ volatile(
	    ".intel_syntax noprefix\n" // switch to Intel syntax
	    "mov %0, fs\n"             // Intel-style mov
	    ".att_syntax prefix\n"     // switch back to AT&T (default)
	    : "=r"(fs_value)           // output operand
	);
	return fs_value;
}

uint16_t get_gs_selector() {
	uint16_t gs_value;
	__asm__ volatile(
	    ".intel_syntax noprefix\n" // switch to Intel syntax
	    "mov %0, gs\n"             // Intel-style mov
	    ".att_syntax prefix\n"     // switch back to AT&T (default)
	    : "=r"(gs_value)           // output operand
	);
	return gs_value;
}

uint16_t get_ds_selector() {
	uint16_t ds_value;
	__asm__ volatile(
	    ".intel_syntax noprefix\n" // switch to Intel syntax
	    "mov %0, ds\n"             // Intel-style mov
	    ".att_syntax prefix\n"     // switch back to AT&T (default)
	    : "=r"(ds_value)           // output operand
	);
	return ds_value;
}

typedef SegmentDescriptor GDT_ENTRY;

typedef struct __attribute__((packed)) {
	uint16_t limit;
	GDT_ENTRY* base;
} GDT_ROOT;

GDT_ROOT get_gdt_root(void) {

	GDT_ROOT gdt_root;
	__asm__ volatile("sgdt %0" : "=m"(gdt_root));

	return gdt_root;
}

extern GDT_ROOT GDT16_DESCRIPTOR;
GDT_ROOT* GDT16_ROOT = &GDT16_DESCRIPTOR;
