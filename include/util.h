#ifndef _util_h
#define _util_h
#include "types.h"


u32 read32(size_t addr);
void write32(size_t addr, u32 data);

#define get_bits_sz(data, start, size) ((data >> start) & ((1 << size) - 1))
#define get_bits_stop(data, start, stop) ((data >> start) & ((1 << (stop - start + 1)) - 1))

#define _set_bits_mask(start, size) ((((1 << (size)) - 1)) << (start))
#define _clear_bits(data, start, size)  ((data) & ~_set_bits_mask(start, size))
#define set_bits(data, val, start, size) _clear_bits(data, start, size) | ((val) << (start))

#define min(a, b)  ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#endif