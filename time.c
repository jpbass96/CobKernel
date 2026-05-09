//
// time.c
//

#include "types.h"

void wait(u64 cycles) 
{
    volatile u64 i;
    for (i = 0; i < cycles; i++) 
    {
        // Empty loop for delay
    }
}
