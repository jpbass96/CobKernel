#ifndef _primes_h
#define _primes_h
#include "types.h"
#include "work_queue.h"

int calculate_primes(u64 start, u64 stop);
DECLARE_TASK_HANDLE(calculate_primes, 2, u64);

#endif