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


int main (void)
{
  //u32* rp1_addr = 0x1f00000000;
  //0x30000
  //size_t uart_addr = (size_t)0x1f00030000ULL;
  
  
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

  get_core_context();
  get_psci_version();

  for (int i =0; i < 4; i++) {
    get_core_state(0, i, 0);
  }
  
  start_console();
   
  return 0;
  
}
