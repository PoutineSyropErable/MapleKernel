#include "virtual_memory.h"
#include <stdint.h>

#define PAGE_PRESENT 0x1 // The page exist in physical memory.
#define PAGE_RW 0x2      // This page is read write or read only

#define PAGE_USER 0x004       // Bit 2: User/supervisor (1 = user)
#define PAGE_PWT 0x008        // Bit 3: Page-level write-through
#define PAGE_PCD 0x010        // Bit 4: Page-level cache disable
#define PAGE_ACCESSED 0x020   // Bit 5: Accessed by CPU
#define PAGE_DIRTY 0x040      // Bit 6: Dirty (for page tables)
#define PAGE_PS 0x080         // Bit 7: Page size (0=4KB, 1=4MB)
#define PAGE_GLOBAL 0x100     // Bit 8: Global page (ignore TLB flush)
#define PAGE_AVAILABLE1 0x200 // Bit 9: Available for OS
#define PAGE_AVAILABLE2 0x400 // Bit 10: Available for OS
#define PAGE_AVAILABLE3 0x800 // Bit 11: Available for OS

uint32_t page_directory[1024] __attribute__((aligned(4096)));
uint32_t first_page_table[1024] __attribute__((aligned(4096)));

#define PAGE_SIZE 0x1000                        // 4 KB
#define TOTAL_MEMORY (4UL * 1024 * 1024 * 1024) // 4 GB emulated memory
#define NUM_PAGES (TOTAL_MEMORY / PAGE_SIZE)    // total number of pages
#define BITMAP_SIZE (NUM_PAGES / 8)             // 1 bit per pag
uint8_t page_bitmap[BITMAP_SIZE];

#define PAGE_INDEX(addr) ((addr) / PAGE_SIZE)
#define SET_PAGE_PRESENT(idx) (page_bitmap[(idx) / 8] |= (1 << ((idx) % 8)))
#define CLEAR_PAGE_PRESENT(idx) (page_bitmap[(idx) / 8] &= ~(1 << ((idx) % 8)))
#define IS_PAGE_PRESENT(idx) (page_bitmap[(idx) / 8] & (1 << ((idx) % 8)))

void init_page_bitmap() {
	uint32_t page_index; // Current page number
	uint32_t byte_index; // Which byte in the bitmap
	uint8_t bit_mask;    // Mask for the bit inside the byte

	// Clear the entire bitmap initially
	for (uint32_t i = 0; i < NUM_PAGES / 8; i++) {
		page_bitmap[i] = 0;
	}

	// Set bits for the pages that are mapped in the first page table (first 4 MB)
	for (page_index = 0; page_index < 1024; page_index++) {
		byte_index = page_index / 8;      // Which byte contains this page's bit
		bit_mask = 1 << (page_index % 8); // Which bit inside the byte

		page_bitmap[byte_index] |= bit_mask; // Mark page as present
	}
}

void init_paging() {
	// 0x1000 = 4096 = 1024 * sizeof(int32/float32)
	for (int i = 0; i < 1024; i++)
		first_page_table[i] = (i * 0x1000) | PAGE_PRESENT | PAGE_RW;

	page_directory[0] = ((uint32_t)first_page_table) | PAGE_PRESENT | PAGE_RW;
	// Only map the first page directory. So only 4mb of memory are present. They are identity mapped.

	// Inline assembly to load CR3 and enable paging
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
 *
 * Scans the page bitmap for the first free page, marks it as present,
 * and returns its physical address. Returns 0 if no free pages remain.
 *
 * Optimizable: Could use a free-list or bit-scan instructions for faster O(1) search.
 */
int32_t alloc_page() {
	uint32_t page_index;
	uint32_t byte_index;
	uint8_t bit_mask;

	for (page_index = 0; page_index < NUM_PAGES; page_index++) {
		byte_index = page_index / 8;
		bit_mask = 1 << (page_index % 8);

		// Check if the page is free
		if (!(page_bitmap[byte_index] & bit_mask)) {
			// Mark it as present
			page_bitmap[byte_index] |= bit_mask;

			// Return physical address
			return page_index * PAGE_SIZE;
		}
	}

	// No free pages available
	return 0;
}

/**
 * Free a previously allocated page.
 *
 * Clears the present bit in the page bitmap so that it can be reallocated.
 *
 * page_addr: Physical address of the page to free. Must be 4 KB aligned.
 */
void free_page(uint32_t page_addr) {
	uint32_t page_index = page_addr / PAGE_SIZE;
	uint32_t byte_index = page_index / 8;
	uint8_t bit_mask = 1 << (page_index % 8);

	// Mark page as free
	page_bitmap[byte_index] &= ~bit_mask;
}

/**
 * Allocate 'n' consecutive 4 KB pages.
 *
 * Scans the page bitmap to find a hole of 'n' free pages. Marks them as present
 * and returns the physical address of the first page.
 *
 * Returns 0 if no suitable hole is found.
 *
 * Optimizable: Could maintain a free-list of contiguous page ranges or use
 * hierarchical bitmaps for faster search.
 */
uint32_t alloc_n_pages(uint32_t n) {
	if (n == 0 || n > NUM_PAGES) {
		return 0; // invalid request
	}

	uint32_t start_index = 0; // candidate start of free block
	uint32_t free_count = 0;  // number of consecutive free pages found
	uint32_t page_index, byte_index;
	uint8_t bit_mask;

	// Scan through all pages
	for (page_index = 0; page_index < NUM_PAGES; page_index++) {
		byte_index = page_index / 8;
		bit_mask = 1 << (page_index % 8);

		if (!(page_bitmap[byte_index] & bit_mask)) {
			// Page is free
			if (free_count == 0) {
				start_index = page_index; // potential start of hole
			}
			free_count++;

			if (free_count == n) {
				// Found enough consecutive pages, mark them as allocated
				for (uint32_t i = 0; i < n; i++) {
					uint32_t idx = start_index + i;
					page_bitmap[idx / 8] |= 1 << (idx % 8);
				}

				// Return physical address of the first page
				return start_index * PAGE_SIZE;
			}
		} else {
			// Page is taken, reset counter
			free_count = 0;
		}
	}

	// No contiguous block of 'n' pages found
	return 0;
}

struct kmalloc_header {
	uint32_t pages;
};

// Simple kmalloc: allocate contiguous pages
void* kmalloc(uint32_t size) {
	if (size == 0)
		return NULL;

	// Calculate number of pages required
	uint32_t pages_needed = (size + PAGE_SIZE - 1) / PAGE_SIZE;

	// Allocate physical pages
	uint32_t phys_addr = alloc_n_pages(pages_needed);
	if (phys_addr == 0)
		return NULL; // allocation failed

	// Convert to kernel virtual address
	uint32_t* virt_addr = (uint32_t*)phys_to_virt(phys_addr);

	// Store header before returned pointer
	struct kmalloc_header* header = (struct kmalloc_header*)virt_addr;
	header->pages = pages_needed;

	// Return pointer after header
	return (void*)(header + 1);
}

void kfree(void* ptr) {
	if (!ptr)
		return;

	// Retrieve header
	struct kmalloc_header* header = ((struct kmalloc_header*)ptr) - 1;
	uint32_t pages_to_free = header->pages;

	// Compute physical address of allocation
	uint32_t phys_addr = virt_to_phys((uint32_t)header);

	// Free physical pages
	free_n_pages(phys_addr, pages_to_free);
}
