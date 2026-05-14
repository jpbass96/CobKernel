#ifndef _util_h
#define _util_h
#include "types.h"
#include "printf.h"

#define DEBUG

#ifdef DEBUG
#define LOG_DEBUG(...) printf("[DEBUG] %s: ", __FILE__); \
                       printf(__VA_ARGS__)
#else
#define LOG_DEBUG(...) 
#endif

#define LOG_ERROR(...) printf("[ERROR] %s: ", __FILE__); \
                       printf(__VA_ARGS__)

u32 read32(size_t addr);
void write32(size_t addr, u32 data);

#define get_bits_sz(data, start, size) ((data >> start) & ((1 << size) - 1))
#define get_bits_stop(data, start, stop) ((data >> start) & ((1 << (stop - start + 1)) - 1))

#define _set_bits_mask(start, size) ((((1 << (size)) - 1)) << (start))
#define _clear_bits(data, start, size)  ((data) & ~_set_bits_mask(start, size))
#define set_bits(data, val, start, size) _clear_bits(data, start, size) | ((val) << (start))

#define min(a, b)  ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define ceildiv(a, b) (((a) + (b)-1)/(b))
#define alignup(a, align) (((a) + ((align) - 1)) & ~((align) - 1))
#define aligndown(a, align) ((a) & ~((align)-1))

static inline int ispow2(u64 val) {
    u8 count = 0;
    while (val) {
        //count lsb if it 1 and shift bit out. Return if vector is 0
        count += val & 1;
        val>>=1;
    }
    return count < 2;
}

#endif