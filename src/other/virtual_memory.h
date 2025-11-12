#pragma once
#include <stddef.h> // for NULL
#include <stdint.h>

void init_paging();
void init_page_bitmap();

uint32_t alloc_page(void);
void free_page(uint32_t phys_addr);
uint32_t alloc_n_pages(uint32_t n);
void free_n_pages(uint32_t phys_addr, uint32_t n);

uint32_t virt_to_phys(uint32_t virt_addr);
void* phys_to_virt(uint32_t phys_addr);

void* kmalloc(uint32_t size);
void kfree(void* ptr);
