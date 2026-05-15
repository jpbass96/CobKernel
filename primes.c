#include "types.h"
#include "printf.h"
#include "work_queue.h"
#include "primes.h"
#include "util.h"
#include "kernel_info.h"

int calculate_primes(u64 start, u64 stop) {
    boolean is_prime;
    u64 count = 0;

    for (u64 i=start; i < stop; i++) {
        
        if (!(i & 1)) {
            is_prime = FALSE;
            continue;
        }
        is_prime = TRUE;
        //we already know it is not divisible by 2, so start at 3.
        //go up to i/2 since mathematically 2 is the lowest possible divisor
        for (u64 j=3; j < (i/2); j++) {
            if (i%j == 0) {
                is_prime = FALSE;
                break;
            }
        }
        if (is_prime) {
            //need to make this snprintf really
            count++;
        }
    }
    LOG_INFO("Core %ld: Found %ld primes in range %ld to %ld\n\r", get_core_affinity()>>8, count, start, stop);
    return 0;
}

INIT_TASK_HANDLE(calculate_primes, 2, u64)