#pragma once
#include "gdt.h"
#include "static_assert.h"
#include <stdbool.h>
#include <stdint.h>

/*
The TSS segment selectors points to an entry in the GDT.
That entry has a base address field (separated in parts (3/4)), which points to the TSS
The TSS has information about the different stack for different priviledge level.
If a priviledge level change is done, then the stack is read from it
*/

#define PACKED __attribute__((packed))

// 64-bit TSS descriptor (long mode) // Takes 2 gdt entry
typedef struct PACKED tss_descriptor_64
{
	uint16_t limit_low;		 // bits 0-15 of limit
	uint16_t base_low;		 // bits 0-15 of base
	uint8_t	 base_middle1;	 // bits 16-23 of base
	uint8_t	 type : 4;		 // 40-43
	uint8_t	 s : 1;			 // 44, 0 = system
	uint8_t	 dpl : 2;		 // 45-46
	uint8_t	 p : 1;			 // 47
	uint8_t	 limit_high : 4; // 48-51
	uint8_t	 avl : 1;		 // 52
	uint8_t	 l : 1;			 // 53
	uint8_t	 db : 1;		 // 54
	uint8_t	 g : 1;			 // 55
	uint8_t	 base_middle2;	 // bits 24-31 of base
	uint32_t base_high;		 // bits 32-63 of base
	uint32_t reserved;		 // must be zero
} tss_descriptor_64_t;

STATIC_ASSERT(sizeof(tss_descriptor_64_t) == 16, "64-bit TSS descriptor must be 16 bytes");

// Normal 32-bit segment descriptor reader
static inline segment_descriptor_t segment_descriptor_read(segment_descriptor_t *gdt, uint16_t index)
{
	return gdt[index];
}

static inline tss_descriptor_64_t *tss_descriptor_64_fast(segment_descriptor_t *gdt, uint16_t index)
{
	// Each TSS takes 2 GDT entries, so index points to the first
	return (tss_descriptor_64_t *)&gdt[index];
}

typedef struct __attribute__((packed)) tss16
{
	uint16_t prev_tss; // Previous TSS selector (Not an address. a 16 bit raw value)

	uint16_t sp0; // Ring 0 stack pointer (These are used for when an interupts change from a different ring level (1,2,3) to ring 0)
	segment_selector_t ss0; // Ring 0 stack segment (Same as above. If I'm always in ring0, it can stay NULL)

	uint16_t		   sp1; // Ring 1 stack pointer
	segment_selector_t ss1; // Ring 1 stack segment

	uint16_t		   sp2; // Ring 2 stack pointer
	segment_selector_t ss2; // Ring 2 stack segment

	uint16_t ip;	// Instruction pointer (task switch)
	uint16_t flags; // Flags (task switch)

	uint16_t ax; // General-purpose registers
	uint16_t cx;
	uint16_t dx;
	uint16_t bx;
	uint16_t sp; // Stack pointer
	uint16_t bp;
	uint16_t si;
	uint16_t di;

	segment_selector_t es; // Segment registers
	segment_selector_t cs;
	segment_selector_t ss;
	segment_selector_t ds;
	// No FS and GS

	// Local Descriptor Table selector
	segment_selector_t ldt_selector;
	// uint16_t trap; // Trap on task switch flag
	// uint16_t io_map_base; // I/O map base address
} tss16_t;

STATIC_ASSERT(sizeof(tss16_t) == 44, "16-bit TSS must be 44 bytes");

typedef struct __attribute__((packed)) tss32
{
	uint16_t prev_tss; // Previous TSS selector
	uint16_t reserved0;

	uint32_t esp0; // Ring 0 stack pointer (These are used for when an interupts change from a different ring level (1,2,3) to ring 0)
	segment_selector_t ss0; // Ring 0 stack segment (Same as above. If I'm always in ring0, it can stay NULL)
	uint16_t		   reserved1;

	uint32_t		   esp1; // Ring 1 stack pointer
	segment_selector_t ss1;	 // Ring 1 stack segment
	uint16_t		   reserved2;

	uint32_t		   esp2; // Ring 2 stack pointer
	segment_selector_t ss2;	 // Ring 2 stack segment
	uint16_t		   reserved3;

	uint32_t cr3;	 // Page directory base (for hardware task switch)
	uint32_t eip;	 // Instruction pointer (task switch)
	uint32_t eflags; // Flags (task switch)
	uint32_t eax;	 // General-purpose registers
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esp; // Stack pointer
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;

	segment_selector_t es; // Segment registers
	uint16_t		   reserved4;
	segment_selector_t cs;
	uint16_t		   reserved5;
	segment_selector_t ss;
	uint16_t		   reserved6;
	segment_selector_t ds;
	uint16_t		   reserved7;
	segment_selector_t fs;
	uint16_t		   reserved8;
	segment_selector_t gs;
	uint16_t		   reserved9;

	segment_selector_t ldt_selector; // Local Descriptor Table selector
	uint16_t		   reserved10;
	uint16_t		   trap;		// Trap on task switch flag
	uint16_t		   io_map_base; // I/O map base address
} tss32_t;

STATIC_ASSERT(sizeof(tss32_t) == 104, "32-bit TSS must be 104 bytes");

typedef struct PACKED tss64
{
	uint32_t reserved0;

	// Stack pointers for privilege levels 0-2
	uint64_t rsp0; // Ring 0 stack pointer
	uint64_t rsp1; // Ring 1 stack pointer
	uint64_t rsp2; // Ring 2 stack pointer

	uint64_t reserved1;

	// Interrupt Stack Table (IST1-IST7)
	uint64_t ist1;
	uint64_t ist2;
	uint64_t ist3;
	uint64_t ist4;
	uint64_t ist5;
	uint64_t ist6;
	uint64_t ist7;

	uint64_t reserved2;
	uint16_t reserved3;

	// I/O map base address
	uint16_t iomap_base;
} tss64_t;

STATIC_ASSERT(sizeof(tss64_t) == 104, "x86-64 TSS must be 104 bytes");

/*
32-bit PM, no error, same cpl (eq. Keyboard iRQ, `int n` from ring 0)
*/
typedef struct PACKED
{
	uint32_t		   eip;
	segment_selector_t cs;
	uint16_t		   _padding; // align EFLAGS
	uint32_t		   eflags;
} interrupt_information_32_t;

STATIC_ASSERT(sizeof(interrupt_information_32_t) == 12, "32-bit no-error struct must be 12 bytes");

/*
32-bit PM, No CPL change, with error code. (eg, page fault from kernel mode)
*/
typedef struct PACKED
{

	uint32_t		   eflags;
	segment_selector_t cs;
	uint16_t		   _padding;
	uint32_t		   eip;

	uint32_t error_code;

} interrupt_information_32_error_t;

STATIC_ASSERT(sizeof(interrupt_information_32_error_t) == 16, "32-bit no CPL change with error code must be 16 bytes");

/*
32-bit PM, CPl Change (ring3 -> 0), without error code. (Typical software interupt from ring 3)
Also legacy mode without error (in emulated 32 bit mode, from Long mode)
*/
typedef struct PACKED
{
	segment_selector_t old_ss;
	uint16_t		   _pad0;
	uint32_t		   old_esp;

	uint32_t		   eflags;
	segment_selector_t cs;
	uint16_t		   _pad1;
	uint32_t		   eip;

} interrupt_information_32_cplchange_t;

STATIC_ASSERT(sizeof(interrupt_information_32_cplchange_t) == 20, "32-bit CPL change without error must be 20 bytes");

/*
32-bit PM, change (ring 3 -> 0), with error code (eg, page fault from user mode)
Also legacy mode with error (in emulated 32 bit mode, from Long mode)
*/
typedef struct PACKED
{

	segment_selector_t old_ss;
	uint16_t		   _pad0;
	uint32_t		   old_esp;

	uint32_t		   eflags;
	segment_selector_t cs;
	uint16_t		   _pad1;
	uint32_t		   eip;

	uint32_t error_code;
} interrupt_information_32_error_cplchange_t;

STATIC_ASSERT(sizeof(interrupt_information_32_error_cplchange_t) == 24, "32-bit CPL change with error code must be 24 bytes");

/*
64-bit LM, Any ring changes, without error code (eg, a software interupt)
*/
typedef struct PACKED
{

	segment_selector_t old_ss;
	uint16_t		   _pad_ss;
	uint32_t		   _pad_ss2;
	uint64_t		   old_rsp;

	uint64_t		   rflags;
	segment_selector_t cs;
	uint16_t		   _pad_cs;
	uint32_t		   _pad_cs2;
	uint64_t		   rip;

} interrupt_information_64_t;

STATIC_ASSERT(sizeof(interrupt_information_64_t) == 40, "64-bit LM without error must be 40 bytes");

/*
64-bit LM, Any ring changes, with error code (eg, any error interrupt, page fault from anywhere)
*/
typedef struct PACKED
{

	segment_selector_t old_ss;
	uint16_t		   _pad_ss;
	uint32_t		   _pad_ss2;
	uint64_t		   old_rsp;

	uint64_t		   rflags;
	segment_selector_t cs;
	uint16_t		   _pad_cs;
	uint32_t		   _pad_cs2;
	uint64_t		   rip;

	uint32_t error_code;
	uint32_t _pad_error;
} interrupt_information_64_error_t;

STATIC_ASSERT(sizeof(interrupt_information_64_error_t) == 48, "64-bit LM with error must be 48 bytes");

/* =============32-bit PM,  tss process=============== */

// #define C_TEST

#ifdef C_TEST
// Doing in software what the hardware do

void push(uint32_t *esp, uint32_t stack_base, interrupt_information_32_t information)
{
	// This is assuming we are manually doing the manual address calculation.
	// So, this code that tries to simulate what happen when an interupt is called would not compile

	*esp -= sizeof(information);
	uint32_t linear_adddress = stack_base + *esp;

	interrupt_information_32_t *esp_write = (interrupt_information_32_t *)linear_adddress;
	*esp_write							  = information;
}

void push_cpl_change(uint32_t *esp, uint32_t stack_base, interrupt_information_32_cplchange_t information)
{

	*esp -= sizeof(information);
	uint32_t linear_adddress = stack_base + *esp;

	interrupt_information_32_cplchange_t *esp_write = (interrupt_information_32_cplchange_t *)linear_adddress;
	*esp_write										= information;
}

void do_tss_stuff(segment_descriptor_t *gdt, segment_selector_t ts)
{

	const bool ring_change = false;
	if (ring_change)
	{

		__asm__ volatile("ltr %0" : : "r"(ts));

		segment_descriptor_t tss_entry = gdt[ts.index];
		uint32_t			 tss_base  = get_segment_descriptor_base(&tss_entry);

		volatile tss32_t  *tss_base_address = (tss32_t *)tss_base;
		uint32_t		   esp0				= tss_base_address->esp0;
		segment_selector_t ss0				= tss_base_address->ss0;

		segment_descriptor_t	  stack0_entry			 = gdt[ss0.index];
		uint32_t				  stack0_base			 = get_segment_descriptor_base(&stack0_entry);
		[[maybe_unused]] uint32_t stack0_linear_adddress = stack0_base + esp0;
		// We don't do this case. it would be another type of information

		interrupt_information_32_cplchange_t keyboard_press_info;
		keyboard_press_info.cs	   = (segment_selector_t){.index = 2, .rpl = 0, .ti = 0};
		keyboard_press_info.eflags = 0xdeadface;
		keyboard_press_info.eip	   = 0xc0de;

		uint32_t		   current_esp3;
		segment_selector_t current_ss3;
		__asm__ volatile("mov %%esp, %0" : "=r"(current_esp3));
		__asm__ volatile("mov %%ss, %0" : "=r"(current_ss3));
		keyboard_press_info.old_esp = current_esp3;
		keyboard_press_info.old_ss	= current_ss3;

		push_cpl_change(&esp0, stack0_base, keyboard_press_info);
	}
	else
	{

		// obtain current esp and ss
		uint32_t		   esp0;
		segment_selector_t ss0;
		__asm__ volatile("mov %%esp, %0" : "=r"(esp0));
		__asm__ volatile("mov %%ss, %0" : "=r"(ss0));

		segment_descriptor_t stack0_entry = gdt[ss0.index];
		uint32_t			 stack0_base  = get_segment_descriptor_base(&stack0_entry);

		interrupt_information_32_t keyboard_press_info;
		keyboard_press_info.cs	   = (segment_selector_t){.index = 2, .rpl = 0, .ti = 0};
		keyboard_press_info.eflags = 0xdeadface;
		keyboard_press_info.eip	   = 0xc0de;

		push(&esp0, stack0_base, keyboard_press_info);
	}
}
#endif
