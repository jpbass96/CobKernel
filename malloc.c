//Basic memory allocator. Defines a memory region with a top, a total size in bytes, and a page size in bytes.
//Memory region is initialized for the beginning to contain the page table. The page table is simply a flat array
//of address pointers representing given page. Minimum page size is 4096 bytes. This allows us to use the lower
//12 bits of the page table entry as a flags attribute

//Each entry will use the LSB to indicate if the page is in use. The page entry
// is encoded as follows:
// y = MSB of largest address system supports. either 64 or 32.
// page_entry[63:12] = addr[y:12] of the first page in this block. Used on free to figure out which
//                    pages are associated with a particular block allocation
// page_entry[11:1] = Reserved flags. Set to 0
// page_entry[0] = Page in use. Set to 1 when allocated. 0 means the page is free to be allocated.
//
// Malloc will simply start searching at the beginning of the table for next open hole matching the size requested
// Free releases all pages associated with the given address.

//TODO: Add support for tracking bytes allocated in page for a given context. This will allow consecutive calls to malloc with small allocations to be merged together.

#include "types.h"
#include "util.h"
#include "printf.h"
#include "malloc.h"
#include "arm.h"

#define PAGE_ENTRY_FLAGS(entry) ((entry) & 0xFFFULL)
#define PAGE_ENTRY_START_ADDR(entry) ((entry) & ~0xFFFULL)
#define PAGE_ENTRY_VALID(entry) ((entry) & 1)
#define IS_FIRST_PAGE_IN_BLOCK(entry)  (((entry) & ~0xFFEULL) == 1)
struct memory_region kernel_heap;

//Initialize a memory region struct with the given start address and size. It is assumed
//The heap will grow up from "top". The first x bytes are used to store the page table,
//so the actual size of the region will be less than the requested size. The page table
//will be aligned to a page size, and the end of the memory allocation range will also
//be aligned to the page size. This prevents having to check for partial pages in allocators.
//Arguments: 
//  top: Address of region we want to allocate from
//  size: size in bytes of total region pointed to by top
//  page_size: minimum number of bytes that can be allocated to a user
//  region: region struct that will get updated with final attributes after initialization
//Return:
//  0 on success, -1 on failure
int init_memory_region(void *top, u64 size, u64 page_size, struct memory_region *region) {
    
    if (!ispow2(page_size) | (page_size < 4096)) {
        LOG_ERROR("page size must be power of 2 and > 4096\n\r");
        return -1;
    }

    //calculate number of pages needed to allocate requested size rounded down to make sure we dont overflow the memory region provided
    region->page_table_entries = size / page_size;

    region->page_table = top;

    //actual top of the memory region will be at the end of the page table aligned to the next nearest page to make
    //sure all allocations return a page aligned address
    region->top = (void*)alignup((size_t)(region->page_table + region->page_table_entries*sizeof(void*)), page_size);

   //error condition checking
    if (size < page_size) {
        LOG_ERROR("Invalid size. must be > page_size\n\r");
        return -1;
    }
    if ((region->top + page_size) > (top + size)) {
        LOG_ERROR("Memory region could not even fit 1 page after page table allocation\n\r");
        return -1;
    }

    //Calculate allocatable size from top of region to first allocatable page. Align size down to page size to prevent
    //partial pages.
    region->size = aligndown(((top + size) - (region->top)), page_size);
    region->page_size = page_size;

     //initialize page table to be all 0's
    u64 *cur = region->page_table;
    for (size_t i = 0; i < region->page_table_entries; i++) {
        *(cur++) = 0x0;
    }

    if (arm64_init_semaphore(&(region->sem)) != 0) {
        LOG_ERROR("Could not initialize memory region semaphore\n\r");
        return -1;
    }

    return 0;
}

//iterate through the page table in region and find the first contiguous
//region of unmapped pages that can fit num_pages.
//Assumes the page table can be accessed in an exclusive context
static void *find_hole(struct memory_region *region, size_t num_pages) {
    u64 *cur;
    void *start, *end;
    size_t num_found;

    cur = region->page_table;
    end = cur + region->page_table_entries;
    num_found = 0;
    start = NULL;

    while ((void*)cur < end) {
        if (!(*cur & 0x1)) {
            num_found++;
            start = start == NULL ? cur : start;
            if (num_found == num_pages) {
                return start;
            }
        }
        else {
            start = NULL;
        }
        cur++;
    } 
    //iterated through whole table and never found a hole
    return NULL;
}

//mark all entries in range start_entry + num_pages used. Set
//start entry of all entries except start_entry itself
//Assumes the page table can be accessed in an exclusive context
static void mark_pages_used(u64 *start_entry, size_t num_pages) {
    u64 *cur = start_entry;
    LOG_DEBUG("Marking Starting page of block: 0x%lx\n\r", cur);
    *cur++ = 1;
    for (size_t i = 1; i < num_pages; i++) {
        *cur++ = 1 | (size_t)start_entry;
    }
    LOG_DEBUG("Marked final page used 0x%lx. Starting block address is 0x%lx\n\r", --cur, start_entry);
}

void *_malloc(struct memory_region *region, size_t num_bytes) {
    size_t num_pages;
    void *page_table_start;
    void *addr;
    //get number of pages required to fit this allocation.
    num_pages = ceildiv(num_bytes, region->page_size);

     //take semaphore after checking all constant fields
    arm64_take_semaphore_exclusive(&(region->sem)) ;

    LOG_DEBUG("Allocating %ld pages\n\r", num_pages);
    page_table_start = find_hole(region, num_pages);

    LOG_DEBUG("Found hole at 0x%lx\n\r", page_table_start);
    if (page_table_start == NULL) {
        addr = NULL;
        goto _semfree;
    }

    mark_pages_used(page_table_start, num_pages);

    arm64_put_semaphore_exclusive(&(region->sem));

    //calculate actual start address of allocated region
    addr = (((page_table_start - region->page_table) / sizeof(void*)) * region->page_size) + region->top;

    _semfree: arm64_put_semaphore_exclusive(&(region->sem)) ;
     return addr;
}

void _free(struct memory_region *region, void *addr) {
    size_t page_entry_idx;
    u64 *start_entry, *cur;

    if (addr == NULL) {
        goto _ret;
    }
    //Basic checks that we got a valid address and region
    if (region == NULL) {
        LOG_ERROR("NULL region passed to free\n\r");
        goto _ret;
    }

    //Check that address is within the provided memory region
    if ((addr < region->top) | (addr > (region->top + region->size))) {
        LOG_ERROR("Invalid Free, Address provided was not within memory region\n\r");
        goto _ret;
    }

    page_entry_idx = (addr - region->top) / region->page_size;
    start_entry = region->page_table + page_entry_idx*sizeof(void*);

    //take semaphore after checking all constant fields
    arm64_take_semaphore_exclusive(&(region->sem)) ;

    LOG_DEBUG("Calculated start_entry in free 0x%lx. Value of entry 0x%lx\n\r", start_entry, *start_entry);

    //check if address passed in matches a previous malloc call
    if ((addr != (region->top + page_entry_idx*region->page_size)) | !IS_FIRST_PAGE_IN_BLOCK(*start_entry)) {
        LOG_ERROR("Address passed to free was not an address previously returned by malloc\n\r");
        LOG_ERROR("Address was 0x%lx, expected address was 0x%lx\n\r", addr, (size_t)(region->top + page_entry_idx*region->page_size));
        LOG_ERROR("First page in block calculation: %d\n\r", IS_FIRST_PAGE_IN_BLOCK(*start_entry));
        goto _semfree;
    }

    cur = start_entry;

    *cur++ = 0;
    while ((void*)PAGE_ENTRY_START_ADDR(*cur) == start_entry) {
        *cur++ = 0;
    }

    _semfree: arm64_put_semaphore_exclusive(&(region->sem));
    _ret: return;
}

void init_kheap(void *start, size_t size, size_t page_size) {
    init_memory_region(start, size, page_size, &kernel_heap);
    LOG_DEBUG("Kernel Heap Initialized: \n\r");
    LOG_DEBUG("  Page Table Starts at: 0x%lx\n\r", kernel_heap.page_table);
    LOG_DEBUG("  Page Table Entries: %ld\n\r", kernel_heap.page_table_entries);
    LOG_DEBUG("  Memory Region Starts at: 0x%lx\n\r", kernel_heap.top);
    LOG_DEBUG("  Allocatable Heap Size: 0x%lx\n\r", kernel_heap.size);
}
