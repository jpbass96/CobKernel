// kernel.c
// Initial code taken from tutorial on www.satyria.de
// Has since been heavily modified

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
#include "work_queue.h"

extern u8 console_initialized;
struct work_queue_entry *queues[3];

extern void _secondary_start(u64 q);


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

int secondary_main(struct work_queue_entry *q) {

  printf("hello from Core 0x%lx. Starting work queue\n\r", get_core_affinity());
  while (1) {
      execute_work(q);
      //asm volatile("wfe"); 
  }
}

int main (void *heap_start, void* heap_end)
{
  //reinterpret heap_start as the work queue arguments and jump to secondary main
  //if we are a secndary core
  if (get_core_affinity() != 0x0) {
    secondary_main((struct work_queue_entry *) heap_start);
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

  kinit_printf(NULL, _write_char);

  init_console(_get_char, _write_char, _wait_fifo_empty);
  console_initialized = 1;
  display_banner();

  get_core_context();
  get_psci_version();

  for (int i =0; i < 4; i++) {
    get_core_state(0, i, 0);
  }

  printf("Initializing memory allocator...\n\r");
  init_kheap(heap_start, (size_t)(heap_end - heap_start), 65536);

  printf("Trying to power other cores\n\r");
  
  struct cpu_affinity affinity;
  affinity.aff0 = 0;
  affinity.aff2 = 0;
  affinity.aff3 = 0;
  for (u64 i = 1; i < 4; i++) {
    //power on each core
    affinity.aff1 = i;
    //create work queue with return buffer sized such that only one page is malloc'd
    queues[i-1] = create_work_queue_entry();
    if (queues[i-1] == NULL) {
      LOG_ERROR("irrecoverable Major error. could not allocate work queue for core %d\n\r", i);
      reboot();

    }
    psci_cpu_on(affinity, (u64)_secondary_start, (u64)queues[i-1]);
  }

  for (int i =0; i < 4; i++) {
    get_core_state(0, i, 0);
  }


  start_console();
   
  return 0;
  
}
