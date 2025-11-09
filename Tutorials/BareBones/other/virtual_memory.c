#include "virtual_memory.h"
#include <stddef.h> // for NULL
#include <stdint.h>

#define PAGE_PRESENT 0x1 // Page exists in physical memory
#define PAGE_RW 0x2      // Read/write

#define PAGE_USER 0x004       // User/supervisor
#define PAGE_PWT 0x008        // Page-level write-through
#define PAGE_PCD 0x010        // Page-level cache disable
#define PAGE_ACCESSED 0x020   // Accessed by CPU
#define PAGE_DIRTY 0x040      // Dirty (for page tables)
#define PAGE_PS 0x080         // Page size (0=4KB, 1=4MB)
#define PAGE_GLOBAL 0x100     // Global page
#define PAGE_AVAILABLE1 0x200 // OS available
#define PAGE_AVAILABLE2 0x400
#define PAGE_AVAILABLE3 0x800

uint32_t page_directory[1024] __attribute__((aligned(4096)));
uint32_t first_page_table[1024] __attribute__((aligned(4096)));

#define PAGE_SIZE 0x1000                        // 4 KB
#define TOTAL_MEMORY (4UL * 1024 * 1024 * 1024) // 4 GB
#define NUM_PAGES (TOTAL_MEMORY / PAGE_SIZE)
#define BITMAP_SIZE (NUM_PAGES / 8)
uint8_t page_bitmap[BITMAP_SIZE];

/* Page bitmap helpers */
static inline uint32_t page_index(uint32_t addr) { return addr / PAGE_SIZE; }
static inline void set_page_present(uint32_t idx) { page_bitmap[idx / 8] |= (1 << (idx % 8)); }
static inline void clear_page_present(uint32_t idx) { page_bitmap[idx / 8] &= ~(1 << (idx % 8)); }
static inline int is_page_present(uint32_t idx) { return page_bitmap[idx / 8] & (1 << (idx % 8)); }

/**
 * Initialize the page bitmap.
 * Marks first 4 MB (first page table) as allocated.
 */
void init_page_bitmap(void) {
	for (uint32_t i = 0; i < BITMAP_SIZE; i++)
		page_bitmap[i] = 0;

	for (uint32_t i = 0; i < 1024; i++) // First 1024 pages = 4 MB
		set_page_present(i);
}

/**
 * Enable paging with first 4 MB identity-mapped.
 */
void init_paging(void) {
	for (int i = 0; i < 1024; i++)
		first_page_table[i] = (i * PAGE_SIZE) | PAGE_PRESENT | PAGE_RW;

	page_directory[0] = ((uint32_t)first_page_table) | PAGE_PRESENT | PAGE_RW;

	asm volatile(
	    "mov %[pd], %%cr3\n"
	    "mov %%cr0, %%eax\n"
	    "or $0x80000000, %%eax\n"
	    "mov %%eax, %%cr0\n"
	    :
	    : [pd] "r"(page_directory)
	    : "eax");
}

/**
 * Allocate a single 4 KB page.
 * Returns the physical address, or 0 if none available.
 */
uint32_t alloc_page(void) {
	for (uint32_t i = 0; i < NUM_PAGES; i++) {
		if (!is_page_present(i)) {
			set_page_present(i);
			return i * PAGE_SIZE;
		}
	}
	return 0;
}

/**
 * Free a single 4 KB page.
 */
void free_page(uint32_t phys_addr) {
	uint32_t idx = page_index(phys_addr);
	clear_page_present(idx);
}

/**
 * Allocate 'n' consecutive pages. Returns physical address of first page.
 * Returns 0 if not enough contiguous pages exist.
 */
uint32_t alloc_n_pages(uint32_t n) {
	if (n == 0 || n > NUM_PAGES)
		return 0;

	uint32_t start = 0, count = 0;
	for (uint32_t i = 0; i < NUM_PAGES; i++) {
		if (!is_page_present(i)) {
			if (count == 0)
				start = i;
			count++;
			if (count == n) {
				for (uint32_t j = 0; j < n; j++)
					set_page_present(start + j);
				return start * PAGE_SIZE;
			}
		} else {
			count = 0;
		}
	}
	return 0;
}

/**
 * Free 'n' consecutive pages starting at physical address.
 */
void free_n_pages(uint32_t phys_addr, uint32_t n) {
	uint32_t start_idx = page_index(phys_addr);
	for (uint32_t i = 0; i < n; i++)
		clear_page_present(start_idx + i);
}

/* --- kmalloc / kfree --- */

// ***Translate a kernel virtual address to a physical address.*Returns 0 if the virtual address is unmapped.*/
uint32_t virt_to_phys(uint32_t virt_addr) {
	uint32_t pd_index = (virt_addr >> 22) & 0x3FF; // top 10 bits
	uint32_t pt_index = (virt_addr >> 12) & 0x3FF; // next 10 bits
	uint32_t offset = virt_addr & 0xFFF;           // bottom 12 bits

	uint32_t pd_entry = page_directory[pd_index];
	if (!(pd_entry & PAGE_PRESENT)) {
		// Page directory entry not present
		return 0;
	}

	uint32_t* pt = (uint32_t*)(pd_entry & 0xFFFFF000); // mask flags
	uint32_t pt_entry = pt[pt_index];
	if (!(pt_entry & PAGE_PRESENT)) {
		// Page table entry not present
		return 0;
	}

	uint32_t phys_addr = (pt_entry & 0xFFFFF000) + offset;
	return phys_addr;
}

/**
 * Translate a physical address to a kernel virtual address.
 * Scans the page tables for the first mapping.
 * Returns NULL if the physical page is not mapped.
 *
 * Optimizable: a reverse mapping table could make this O(1).
 */
void* phys_to_virt(uint32_t phys_addr) {
	uint32_t pd_index, pt_index, offset;
	offset = phys_addr & 0xFFF; // offset within page
	uint32_t phys_page = phys_addr & 0xFFFFF000;

	// Scan page directory
	for (pd_index = 0; pd_index < 1024; pd_index++) {
		uint32_t pd_entry = page_directory[pd_index];
		if (!(pd_entry & PAGE_PRESENT))
			continue;

		uint32_t* pt = (uint32_t*)(pd_entry & 0xFFFFF000);
		for (pt_index = 0; pt_index < 1024; pt_index++) {
			uint32_t pt_entry = pt[pt_index];
			if (!(pt_entry & PAGE_PRESENT))
				continue;

			if ((pt_entry & 0xFFFFF000) == phys_page) {
				// Found mapping
				uint32_t virt_addr = (pd_index << 22) | (pt_index << 12) | offset;
				return (void*)virt_addr;
			}
		}
	}

	// No mapping found
	return NULL;
}

struct kmalloc_header {
	uint32_t pages; // Number of pages allocated
};

/**
 * Allocate 'size' bytes from the kernel heap.
 * Returns a virtual address or NULL.
 */
void* kmalloc(uint32_t size) {
	if (!size)
		return NULL;

	uint32_t pages_needed = (size + PAGE_SIZE - 1) / PAGE_SIZE;
	uint32_t phys_addr = alloc_n_pages(pages_needed);
	if (!phys_addr)
		return NULL;

	uint32_t* virt_addr = (uint32_t*)phys_to_virt(phys_addr);

	struct kmalloc_header* header = (struct kmalloc_header*)virt_addr;
	header->pages = pages_needed;

	return (void*)(header + 1);
}

/**
 * Free memory previously allocated by kmalloc.
 */
void kfree(void* ptr) {
	if (!ptr)
		return;

	struct kmalloc_header* header = ((struct kmalloc_header*)ptr) - 1;
	uint32_t phys_addr = virt_to_phys((uint32_t)header);

	free_n_pages(phys_addr, header->pages);
}
