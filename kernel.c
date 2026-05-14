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
#include "malloc.h"

extern u8 console_initialized;

extern void _secondary_start(u64 ctx);

void _blink_code(int count, int final_state, u64 delay_ms) {
  LED_off();
  wait_s(2);
  for (int i = 0; i < count; i++) {
    LED_on();
    wait_ms(delay_ms);
    LED_off();
  }

  wait_s(2);
  if (final_state) {
    LED_on();
  }
  wait_s(2);
}
void blink_code(int err) {
  //success. 3 slow blinks and remain on
  if (err == 0) {
    //wait(0x0F0000);
     _blink_code(2, 1, 250);
  }

  //pcie link is down
  if (err == 1) {
    _blink_code(3, 0, 750);
  }
}

void display_banner() {
  printf("\n\r********************************\n\r");
  printf(    "*     Welcome to cobKernel     *\n\r");
  printf(    "********************************\n\r");
}

int secondary_main() {
  printf("hello from Core 0x%lx\n\r", get_core_affinity());
  while (1) {
      asm volatile("wfe"); 
  }
}

int main (void *heap_start, void* heap_end)
{
  //loop forever on this core
  //TODO: get core context from CPU affinity rather than a kernel argument
  if (get_core_affinity() != 0x0) {
    secondary_main();
  }

  int err;
  err = 0;
  
  //init kernel clock first so we can accurately wait on certain events. Needed
  //for early kernel error codes
  init_kernel_clk();

  //remove these once proper uart/pcie drivers are implemented.
  //These are know nregister values written based on a readout
  //of raspbi when operating at OS. Makes sure baud rate is set correctly,
  //and that fifos are enabled.
  pcie_fixups();
  uart_fixups();
 
  err = rp1_pcie_link_up();
  if (err) 
    blink_code(1);

  init_printf(NULL, _write_char);

  init_console(_get_char, _write_char, _wait_fifo_empty);
  console_initialized = 1;
  display_banner();

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

  printf("Initializing memory allocator...\n\r");
  init_kheap(heap_start, (size_t)(heap_end - heap_start), 65536);

  start_console();
   
  return 0;
  
}
