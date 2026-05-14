#include "exceptions.h"
#include "types.h"
#include "arm.h"
#include "util.h"
#include "kernel_info.h"
#include "printf.h"
u8 console_initialized;

extern void _secondary_start(u64 ctx);
extern void _start();

void handle_generic_exception(u64 exception) {
  if (console_initialized) {
    u64 affinity;
    printf("Got exception 0x%x\n\r", exception);
    affinity = get_core_affinity();
    affinity = get_bits_sz(affinity, 16, 8);
    if (affinity) {
      _secondary_start(affinity);

    }
    _start(affinity);
  }

}

void handle_synchronous_exception(u64 exception) {
    u64 affinity;

    if (console_initialized) {
        u64 msr;
        printf("Got Synchronous exception\n\r");

        readmsr(ESR_EL2, msr);

        printf("EC is 0x%lx\n\r", get_bits_sz(msr, 2, 6));

        if (get_bits_sz(msr, 25, 1)) {
            printf("Valid ISS: 0x%lx\n\r", get_bits_sz(msr, 0, 25));
        }
    } 

    affinity = get_core_affinity();
    affinity = get_bits_sz(affinity, 16, 8);
    if (affinity) {
      _secondary_start(affinity);
    }
  _start(affinity);

}