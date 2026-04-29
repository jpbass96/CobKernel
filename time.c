//
// time.c
//

#include "types.h"

void wait(u32 cycles) 
{
    volatile u32 i;
    for (i = 0; i < cycles; i++) 
    {
        // Empty loop for delay
    }
}
