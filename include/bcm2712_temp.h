
//TODO: Add GPL header here for code lifted from linux kernel
#ifndef _bcm2712_temp_h
#define _bcm2712_temp_h
#include "types.h"
#include "util.h"

//Pulled from linux kernel
#define AVS_RO_TEMP_STATUS		0x200
#define AVS_RO_TEMP_STATUS_VALID_MSK	((1 << 16) | (1<<10))
#define AVS_RO_TEMP_STATUS_DATA_MSK	  bitmask(0, 10) 

u32 avs_read32(size_t offset);
int bcm2712_get_temp(int *temp);
#endif