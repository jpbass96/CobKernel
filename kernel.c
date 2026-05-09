// kernel.c
//

#include "led.h"
#include "time.h"
#include "types.h"
#include "util.h"
#include "reset.h"
#include "fixups.h"
#include "base.h"
#include "printf.h"
#include "uart.h"
#include "console.h"
#include "arm.h"
#include "psci.h"
#include "kernel_info.h"
#include "rp1_pcie.h"

extern u8 console_initialized;

extern void _secondary_start(u64 ctx);
extern void _start(u64 ctx);

void _blink_code(int count, int final_state, int delay) {
  LED_off();
  wait(0x6F0000);
  for (int i = 0; i < count; i++) {
    LED_on();
    wait(delay);
    LED_off();
  }

  wait(0x6F0000);
  if (final_state) {
    LED_on();
  }
  wait(0x6F0000);
}
void blink_code(int err) {
  //success. 3 slow blinks and remain on
  if (err == 0) {
    //wait(0x0F0000);
     _blink_code(2, 1, 0x3F0000);
  }

  //pcie link is down
  if (err == 1) {
    _blink_code(3, 0, 0x0F0000);
  }
}

void display_banner() {
  printf("\n\r********************************\n\r");
  printf(    "*     Welcome to cobKernel     *\n\r");
  printf(    "********************************\n\r");
}

int secondary_main(u64 ctx) {
  while (1) {
      asm volatile("wfe"); 
  }
}

int main (u64 ctx)
{
  //loop forever on this core
  if (ctx != 0) {
    secondary_main(ctx);
  }

  int err;
  
  err = 0;
  
  pcie_fixups();
  uart_fixups();
 
  err = rp1_pcie_link_up();
  if (err) 
    blink_code(1);

  init_printf(NULL, _write_char);
  init_console(_get_char, _write_char, _wait_fifo_empty);
  console_initialized = 1;
  display_banner();

  printf("Passed in core context is 0x%lx\n\r", ctx);
  get_core_context();
  get_psci_version();

  for (int i =0; i < 4; i++) {
    get_core_state(0, i, 0);
  }

  printf("Trying to power other cores\n\r");
  struct cpu_affinity affinity;
  affinity.aff0 = 0;
  affinity.aff2 = 0;
  affinity.aff3 = 0;
  for (u64 i = 1; i < 4; i++) {
    //power on each core
    affinity.aff1 = i;
    psci_cpu_on(affinity, (u64)_secondary_start, i);
  }

  for (int i =0; i < 4; i++) {
    get_core_state(0, i, 0);
  }

  start_console();
   
  return 0;
  
}
