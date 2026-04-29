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

#define PCIE_MISC_PCIE_STATUS                           0x4068


//#define UART_BASE 0x1f00030000ULL

int link_up() {

  
  u32 val = read32(PCIE_BASE + PCIE_MISC_PCIE_STATUS);
  if ((val & 0x30) == 0x30) {
    
    return 0;
    
  }

  
  return 1;
  
  
 
}

int periphid_matches() {

  if (read32(UART_BASE + 0xFE0) == 0x11) {
    return 0;
  }

  return 2;
}

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

  //uart ID isnt correct
  if (err == 2) {
    _blink_code(6, 0, 0x0F0000);
  }
  
}

int main (void)
{
  //u32* rp1_addr = 0x1f00000000;
  //0x30000
  //size_t uart_addr = (size_t)0x1f00030000ULL;
  
  
  int err;
  err = 0;
  
  //pcie_fixups();
  uart_fixups();
 
  err = link_up();
  if (err) 
    blink_code(err);
 
  init_printf(NULL, _write_char);
  init_console(_get_char, _write_char, _wait_fifo_empty);
  
  start_console();
   
  return 0;
  
}
