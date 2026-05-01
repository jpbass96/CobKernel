#include "types.h"


u32 read32(size_t addr);
void write32(size_t addr, u32 data);
#define min(a, b)  ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))