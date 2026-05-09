#include "kernel_info.h"
#include "types.h"
#include "arm.h"
#include "printf.h"
#include "util.h"

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

u64 get_core_affinity() {

  u64 msr;
  readmsr(MPIDR_EL1, msr);
  printf("Aff0: 0x%x\n\r", get_bits_sz(msr, 0, 8));
  printf("Aff1: 0x%x\n\r", get_bits_sz(msr, 8, 8));
  printf("Aff2: 0x%x\n\r", get_bits_sz(msr, 16, 8));
  printf("Aff3: 0x%x\n\r", get_bits_sz(msr, 32, 8));
  return msr;
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

  /*
check the C bit (bit 2) of SCTLR_EL2. controls cacheability
check the M bit (bit 0) of SCTLR_EL2. enables MMU
check TCR_EL2 bits 13:8 for shareability and cacheability
*/
readmsr(SCTLR_EL2, msr);
printf("Bits 2:0 of SCTLR_EL2: 0x%lx\n\r", get_bits_sz(msr, 0, 3));

readmsr(TCR_EL2, msr);
printf("Bits 13:8 of TCR_EL2: 0x%lx\n\r", get_bits_sz(msr, 8, 6));

}
