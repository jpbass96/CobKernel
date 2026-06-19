#ifndef _util_h
#define _util_h
#include "types.h"
#include "printf.h"
#include "time.h"

#define LOG_LEVEL(level, fmt, ...) printf("[" #level "] " __FILE__ ": " fmt, ##__VA_ARGS__)  

#ifdef VERBOSITY_DEBUG
#define VERBOSITY_INFO
#define LOG_DEBUG(fmt, ...) LOG_LEVEL(DEBUG, fmt, ##__VA_ARGS__)
//printf("[DEBUG] " __FILE__ ": " fmt, __VA_ARGS__);        
#else
#define LOG_DEBUG(fmt, ...) 
#endif

#ifdef VERBOSITY_INFO
#define VERBOSITY_ERROR
#define LOG_INFO(fmt, ...) LOG_LEVEL(INFO, fmt, ##__VA_ARGS__)
#else
#define LOG_INFO(fmt, ...)
#endif

#ifdef VERBOSITY_ERROR
#define LOG_ERROR(fmt, ...) LOG_LEVEL(ERROR, fmt, ##__VA_ARGS__)
#else
#define LOG_ERROR(fmt, ...)
#endif

u32 read32(uintptr addr);
void write32(uintptr addr, u32 data);

#define get_bits_sz(data, start, size) ((data >> start) & ((1 << size) - 1))
#define get_bits_stop(data, start, stop) ((data >> start) & ((1 << (stop - start + 1)) - 1))
#define get_bits_mask(data, mask, lsb) (((data) & (mask)) >> (lsb))

#define bitmask(start, size) ((((1 << (size)) - 1)) << (start))
#define _clear_bits(data, start, size)  ((data) & ~bitmask(start, size))
#define set_bits(data, val, start, size) _clear_bits(data, start, size) | ((val) << (start))


#define min(a, b)  ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define ceildiv(a, b) (((a) + (b)-1)/(b))
#define alignup(a, align) (((a) + ((align) - 1)) & ~((align) - 1))
#define aligndown(a, align) ((a) & ~((align)-1))


#define time_after_us()
/**
 * readl_poll_timeout - Periodically poll an address until a condition is met or a timeout occurs
 * @addr: Address to poll
 * @val: Variable to read the value into
 * @cond: Break condition (usually involving @val)
 * @sleep_us: Maximum time to sleep between reads in uS (0 tight-loops)
 * @timeout_us: Timeout in uS, 0 means never timeout
 *
 * Returns 0 on success and -ETIMEDOUT upon a timeout. In either
 * case, the last read value at @addr is stored in @val. Must not
 * be called from atomic context if sleep_us or timeout_us are used.
 * 
 * Stole perfectly good poll implementation from the linux kernel
 */
#define read32_poll_timeout(val, addr, cond, sleep_us, timeout_us) \
({ \
	unsigned long timeout = get_kernel_time_us() + timeout_us; \
	for (;;) { \
		(val) = read32(addr); \
		if ((cond) || (timeout_us && (get_kernel_time_us() > timeout))) \
			break; \
		if (sleep_us) \
			wait_us(sleep_us); \
	} \
	(cond) ? 0 : -1; \
})

//no timeout yet. need to add
//#define read32_poll(val, addr,  condition, timeout_us, sleep_us) val = read32((addr)); while (!(condition)) { wait_us(sleep_us); val = read32(addr);}
                                        
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