//
// time.c
//

#include "types.h"
#include "arm.h"
#include "util.h"
#include "time.h"

//MSRs
//  CNTFRQ_EL0 = frequency of system clock
//  CNTPCT_EL0 = current count of clock   


struct clock kernel_clock;

u64 aarch64_read_sysclk() {
    u64 msr;
    readmsr(CNTPCT_EL0, msr);
    return msr;
}

int aarch64_init_sysclk(struct clock *clk) {
    readmsr(CNTFRQ_EL0, clk->frequency);
    if (clk->frequency == 0) {
        return -1;
    }
    
    clk->read_count = aarch64_read_sysclk;
    return 0;
}

void _wait_us(struct clock *clk, u64 us) {
    u64 start, end;
    start = clk->read_count();
    end = start + US_TO_CYCLES(us, clk->frequency);

    while (clk->read_count() < end) {
        //eventually yield here
    }
}

void init_kernel_clk() {
    aarch64_init_sysclk(&kernel_clock);
}
