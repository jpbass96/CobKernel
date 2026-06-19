#ifndef _malloc_h
#define _malloc_h
#include "types.h"
#include "arm.h"

#define kmalloc(bytes) _malloc(&kernel_heap, bytes)
#define kmalloc_aligned(bytes, align) _malloc_aligned(&kernel_heap, bytes, align);
#define kfree(addr) _free(&kernel_heap, addr)

extern struct memory_region kernel_heap;

struct memory_region {
    void *top;
    size_t size;
    void *page_table;
    size_t page_table_entries;
    size_t page_size;
    arm64_sem sem;
};

void _free(struct memory_region *region, void *addr);
void *_malloc(struct memory_region *region, size_t num_bytes);
void *_malloc_aligned(struct memory_region *region, size_t num_bytes, u64 align);
void init_kheap(void *start, size_t size, size_t page_size);

#endif

