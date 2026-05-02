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

#define PCIE_MISC_PCIE_STATUS                           0x4068


//#define UART_BASE 0x1f00030000ULL

void print_test() {
  u32 val = 0xff00ff00;
  u64 val2 = 0xf123456789abcdefULL;

  u32 val3 = 0x0f00ff00;
  u64 val4 = 0x0123456789abcdefULL;
  
  printf("val deciamal Expected: 4278255360 or -16711936. Actual %d\n\r", val);
  printf("val deciamal Expected: 0xff00ff00. Actual %x\n\r", val);

  printf("val2 deciamal Expected: ?? or -1070935975390360081. Actual %ld\n\r", val2);
  printf("val2 deciamal Expected: 0xf123456789abcdef. Actual %lx\n\r", val2);


  printf("val3 deciamal Expected: 251723520. Actual %d\n\r", val3);
  printf("val3 deciamal Expected: 0x0f00ff00. Actual %x\n\r", val3);

  printf("val4 deciamal Expected: 81985529216486895. Actual %ld\n\r", val4);
  printf("val4 deciamal Expected: 0x0123456789abcdef. Actual %lx\n\r", val4);


}

void print_cpsr(u8 elevel) {
  u64 msr, tmp;

  switch(elevel) {
    case 1:
      printf("Reading SPSR_EL1\n\r");
      readmsr(SPSR_EL1, msr);
      break;
    case 2:
      printf("Reading SPSR_EL2\n\r");
      readmsr(SPSR_EL2, msr);
      break;
    case 3:
      printf("Reading SPSR_EL3\n\r");
      readmsr(SPSR_EL3, msr);
      break;
  }

  tmp = get_bits_sz(msr, 0, 4);
  printf("Processor state: ");
  if (get_bits_sz(msr, 4, 1)) {
    printf("  Aarch32  ");
  }
  else {
    printf("  Aarch64 ");
  }
  switch(tmp) {
    case 0b00000:
      printf("  User State\n\r");
      break;

    case 0b00001:
      printf("  FIQ State\n\r");
      break;
    case 0b00010:
      printf("  IRQ State\n\r");
      break;
    case 0b00011:
      printf("  Supervisor State\n\r");
      break;
    case 0b10110:
      printf("  Monitor State\n\r");
      break;
    case 0b00111:
      printf("  Abort State\n\r");
      break;
    case 0b01010:
      printf("  Hypervisor State\n\r");
      break;
    case 0b01011:
      printf("  Undefined State\n\r");
      break;
    case 0b1111:
      printf("  System State\n\r");
      break;
    default:
      printf("  Unknown state!\n\r");
  }
  
  if (!get_bits_sz(msr, 5, 1)) {
    printf("Core operating in ARM state\n\r");
  }
  else {
    printf("Core operating in Thumb or Jazelle state\n\r");
  }

  printf("FIQ Mask Bit: 0x%x\n\r", get_bits_sz(msr, 6, 1));
  printf("IRQ Mask Bit: 0x%x\n\r", get_bits_sz(msr, 7, 1));
  printf("Asynchronous Abort Mask Bit: 0x%x\n\r", get_bits_sz(msr, 8, 1));
  if (get_bits_sz(msr, 9, 1)) {
    printf("Big Endian Operation\n\r");
  }
  else {
    printf("Litle Endian Operation\n\r");
  }

  printf("\n\r");
  
}

void print_scr_el3() {

    u64 msr;

  
    printf("Reading SCR_EL3\n\r");
    readmsr(SCR_EL3, msr);

    if (get_bits_sz(msr, 18, 1)) {
      printf("Secure EL2 enabled\n\r");
    }

    if (get_bits_sz(msr, 0, 1)) {
      printf("Secure EL0/EL1 enabled\n\r");
    }
}

void get_core_affinity() {

  u64 msr;
  readmsr(MPIDR_EL1, msr);
  printf("Aff0: 0x%x\n\r", get_bits_sz(msr, 0, 8));
  printf("Aff1: 0x%x\n\r", get_bits_sz(msr, 8, 8));
  printf("Aff2: 0x%x\n\r", get_bits_sz(msr, 16, 8));
  printf("Aff3: 0x%x\n\r", get_bits_sz(msr, 32, 8));
}

void get_core_context() {
  u64 msr;

  
  readmsr(MIDR_EL1, msr);
  printf("MIDR val is 0x%lx\n\r", msr);

  readmsr(CurrentEL, msr);
  printf("curentEL val is 0x%lx\n\r", msr);

  //print_cpsr(1);
  //print_cpsr(2);

  //print_scr_el3();
  get_core_affinity();
  printf("\n\r");

}


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
 
  err = link_up();
  if (err) 
    blink_code(err);

  init_printf(NULL, _write_char);
  init_console(_get_char, _write_char, _wait_fifo_empty);
  display_banner();

  get_core_context();
  get_psci_version();

  for (int i =0; i < 4; i++) {
    get_core_state(0, i, 0);
  }
  
  start_console();
   
  return 0;
  
}
