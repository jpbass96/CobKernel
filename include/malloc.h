#ifndef _malloc_h
#define _malloc_h
#include "types.h"
#include "arm.h"

#define kmalloc(bytes) _malloc(&kernel_heap, bytes)
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
void init_kheap(void *start, size_t size, size_t page_size);

#endif

