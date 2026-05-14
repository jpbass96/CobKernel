// time.h

#ifndef _ms_time_h
#define _ms_time_h

#include "types.h"

#define US_TO_CYCLES(_us, _freq) (((_us) * (_freq)) / 1000000)

struct clock {
    u64 frequency;
    u64 (*read_count)(void);
};

#define wait_us(_us) _wait_us(&kernel_clock, (_us))
#define wait_ms(_ms) wait_us((_ms)*1000)
#define wait_s(_s) wait_ms((_s)*1000)
extern struct clock kernel_clock;

void init_kernel_clk();
void _wait_us(struct clock *clk, u64 us);

#endif
